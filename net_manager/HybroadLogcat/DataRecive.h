#ifndef _DataRecive_H_
#define _DataRecive_H_

#include <stdint.h>
#include <cutils/logger.h>
#include <cutils/logd.h>
#include <cutils/sockets.h>
#include <cutils/logprint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include "DataSource.h"

#ifdef __cplusplus

struct queued_entry_t {
    union {
        unsigned char buf[LOGGER_ENTRY_MAX_LEN + 1] __attribute__((aligned(4)));
        struct logger_entry entry __attribute__((aligned(4)));
    };
    queued_entry_t* next;

    queued_entry_t() {
        next = NULL;
    }
};

static int cmp(queued_entry_t* a, queued_entry_t* b) {
    int n = a->entry.sec - b->entry.sec;
    if (n != 0) {
        return n;
    }
    return a->entry.nsec - b->entry.nsec;
}

struct log_device_t {
    char* device;
    bool binary;
    int fd;
    bool printed;
    char label;

    queued_entry_t* queue;
    log_device_t* next;

    log_device_t(char* d, bool b, char l) {
        device = d;
        binary = b;
        label = l;
        queue = NULL;
        next = NULL;
        printed = false;
    }

    void enqueue(queued_entry_t* entry) {
        if (this->queue == NULL) {
            this->queue = entry;
        } else {
            queued_entry_t** e = &this->queue;
            while (*e && cmp(entry, *e) >= 0) {
                e = &((*e)->next);
            }
            entry->next = *e;
            *e = entry;
        }
    }
};

namespace android {

class DataRecive : public DataSource {
public:
    DataRecive();
    ~DataRecive();
    
    int convertDeviceEntryToAndroidLogEntry(struct logger_entry *buf, AndroidLogEntry *entry);
    int logPrefix(char* buffer, int length, android_LogPriority priority, const char* tag, const char* message);
    void sendLogToRingBuffer(android_LogPriority priority, const char* tag, const char* message);
    void readLogLines(log_device_t* devices);
    void readLogLoop();
private:
	log_device_t* m_devices;
};

} /* namespace android */

#endif /* __cplusplus */

#endif /* _DataRecive_H_ */

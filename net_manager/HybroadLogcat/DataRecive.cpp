#include "DataRecive.h"
#include "RingBuffer.h"
#include "DataSource.h"
#include "DataSink.h"

namespace android {

#define MAX_BLOCK_SIZE	512
static int g_devCount = 0;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

int
DataRecive::convertDeviceEntryToAndroidLogEntry(struct logger_entry *buf,
                                 AndroidLogEntry *entry){
    entry->tv_sec = buf->sec;
    entry->tv_nsec = buf->nsec;
    entry->pid = buf->pid;
    entry->tid = buf->tid;

    if (buf->len < 3) {
        fprintf(stderr, "+++ LOG: entry too small\n");
        return -1;
    }

    int msgStart = -1;
    int msgEnd = -1;

    int i;
    for (i = 1; i < buf->len; i++) {
        if (buf->msg[i] == '\0') {
            if (msgStart == -1) {
                msgStart = i + 1;
            } else {
                msgEnd = i;
                break;
            }
        }
    }

    if (msgStart == -1) {
        fprintf(stderr, "+++ LOG: malformed log message\n");
        return -1;
    }
    if (msgEnd == -1) {
        msgEnd = buf->len - 1;
        buf->msg[msgEnd] = '\0';
    }

    entry->priority = (android_LogPriority)buf->msg[0];
    entry->tag = buf->msg + 1;
    entry->message = buf->msg + msgStart;
    entry->messageLen = msgEnd - msgStart;

    return 0;
}

int
DataRecive::logPrefix(char* buffer, int length, android_LogPriority priority, const char* tag, const char* message)
{
	return snprintf(buffer, length, "%s\n", message);
}

void
DataRecive::sendLogToRingBuffer(android_LogPriority priority, const char* tag, const char* message)
{
    uint8_t* bufPointer;
    uint32_t bufLength;
    int dataSize = 8, blockSize, writeLen;

    pthread_mutex_lock(&g_mutex);

    m_ringBuffer->getWriteHead(&bufPointer, &bufLength);  /** getWriteHead cannot use LogModule */
    if (bufLength == 0) {
        pthread_mutex_unlock(&g_mutex);
        return;
    }

    writeLen = logPrefix((char*)bufPointer + dataSize, bufLength - dataSize, priority, tag, message);
    if ((writeLen < 0) || (writeLen >= ((int)bufLength - dataSize)))
        goto Exit;
    dataSize += writeLen;

    *((int *)(bufPointer + 4)) = dataSize - 8;
    blockSize = (dataSize + 8 + 3) & 0xfffffffc;
    if ((bufLength - blockSize) > MAX_BLOCK_SIZE) {
        *((int *)(bufPointer + 0)) = blockSize;
        m_ringBuffer->submitWrite(bufPointer, blockSize);

    }
    else {
        *((int *)(bufPointer + 0)) = bufLength;
        m_ringBuffer->submitWrite(bufPointer, bufLength);
    }
    if (m_dataSink)
        m_dataSink->receiveData();
Exit:
    pthread_mutex_unlock(&g_mutex);
}
void
DataRecive::readLogLines(log_device_t* devices)
{
    log_device_t* dev;
    int max = 0;
    int ret;
    int queued_lines = 0;
    bool sleep = true;

    int result;
    fd_set readset;

    for (dev=devices; dev; dev = dev->next) {
        if (dev->fd > max) {
            max = dev->fd;
        }
    }

    while (1) {
        do {
            timeval timeout = { 0, 5000 /* 5ms */ };
            FD_ZERO(&readset);
            for (dev=devices; dev; dev = dev->next) {
                FD_SET(dev->fd, &readset);
            }
            result = select(max + 1, &readset, NULL, NULL, sleep ? NULL : &timeout);
        } while (result == -1 && errno == EINTR);

        if (result >= 0) {
            for (dev=devices; dev; dev = dev->next) {
                if (FD_ISSET(dev->fd, &readset)) {
                    //queued_entry_t* entry = new queued_entry_t();
			queued_entry_t entry;
                    ret = read(dev->fd, entry.buf, LOGGER_ENTRY_MAX_LEN);

                    if (ret < 0) {
                        if (errno == EINTR) {
                            //delete entry;
                            goto next;
                        }
                        if (errno == EAGAIN) {
                            //delete entry;
                            break;
                        }
                        perror("logcat read");
                        exit(EXIT_FAILURE);
                    }
                    else if (!ret) {
                        fprintf(stderr, "read: Unexpected EOF!\n");
                        exit(EXIT_FAILURE);
                    }
                    else if (entry.entry.len != ret - sizeof(struct logger_entry)) {
                        fprintf(stderr, "read: unexpected length. Expected %d, got %d\n",
                                entry.entry.len, ret - sizeof(struct logger_entry));
                        exit(EXIT_FAILURE);
                    }
					
                    entry.entry.msg[entry.entry.len] = '\0';
                    AndroidLogEntry mEntry;

                    memset((void*)&mEntry, 0, sizeof(AndroidLogEntry));
                    convertDeviceEntryToAndroidLogEntry(&entry.entry, &mEntry);
					
                    //char buf[5*1024] = {'\0'};

                   // sprintf (buf, "<%d>%s:%s", (int)mEntry.priority, mEntry.tag, mEntry.message);
                  //  android_log_postUdp("110.1.1.29", 514, buf, strlen(buf));
                    sendLogToRingBuffer(mEntry.priority, mEntry.tag, mEntry.message);
                    //printf ("fyx:%s\n", buf);
                    dev->enqueue(&entry);
                    ++queued_lines;

                }
				
            }
        }
next:
        ;
    }
}

DataRecive::DataRecive():m_devices(NULL)
{
		int err;
    log_device_t* dev;


    if (!m_devices) {
        m_devices = new log_device_t(strdup("/dev/"LOGGER_LOG_MAIN), false, 'm');
        android::g_devCount = 1;

        if (0 == access("/dev/"LOGGER_LOG_SYSTEM, R_OK)) {
            m_devices->next = new log_device_t(strdup("/dev/"LOGGER_LOG_SYSTEM), false, 's');
            android::g_devCount++;
        }
    }

    dev = m_devices;
    while (dev) {
        dev->fd = open(dev->device, O_RDONLY);
        if (dev->fd < 0) {
            fprintf(stderr, "Unable to open log device '%s': %s\n",
                dev->device, strerror(errno));
            exit(EXIT_FAILURE);
        }

        dev = dev->next;
    }
}

DataRecive::~DataRecive()
{
	  m_devices = NULL;
}

void
DataRecive::readLogLoop()
{
    readLogLines(m_devices);
}

} /* namespace android */

#ifndef _UpgradeReceiver_H_
#define _UpgradeReceiver_H_

#include "DataSink.h"
#include "MessageHandler.h"

#ifdef __cplusplus

namespace Hippo {

typedef struct {
    char product_id[12];
    unsigned int checksum;
    unsigned int pcb_version;
    unsigned int chip_version;
    int major_version;
    int minor_version;
    int svn_version;
    int force;
    int compressmode;
    unsigned int upgradesize;
    unsigned int datasize;
    char reserve[8];
    unsigned int filesystem;
    char target[16];
    unsigned char MD5[16];
    unsigned char copyright[160];
} UP_HEADER_INFO_b200;


class UpgradeData;
class UpgradeSource;
class UpgradeManager;

class UpgradeReceiver : public DataSink, MessageHandler {
public:
    UpgradeReceiver(UpgradeManager*, UpgradeSource*);
    ~UpgradeReceiver();

    virtual bool start();
    bool stop();

    enum State {
    	URS_OK,
    	URS_ERROR
    };
    virtual State state() { return m_state; }

    UpgradeData* getUpgradeData();
    UpgradeSource* getUpgradeSource() { return m_source; }
    int getUpgradeVersionFromHead(char *path, long offset, UP_HEADER_INFO_b200 *headInfo, const char *targetName);
    void getSettingVersion(const char *path); 

    /* Override. May be called by other thread. */
    virtual bool onDataArrive();
    virtual bool onError();
    virtual bool onEnd();
    bool onTimer();
    int getReceiveLen() {return mReceiveLen; }
protected:
    enum MessageCode {
    	MC_DataArrive,
    	MC_End,
    	MC_Error,
    	MC_TIMER
    };
    /**
     * Subclasses must implement this to receive messages.
     */
    virtual void handleMessage(Message* msg);

    virtual void receiveData();
    virtual void receiveError();
    virtual void receiveEnd();

private:
    UpgradeManager* m_manager;
    UpgradeSource* m_source;
    UpgradeData* m_data;
    State m_state;
    unsigned int mReceiveLen;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UpgradeReceiver_H_

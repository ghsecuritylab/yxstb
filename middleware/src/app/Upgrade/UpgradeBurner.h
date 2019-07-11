#ifndef _UpgradeBurner_H_
#define _UpgradeBurner_H_

#include "MessageHandler.h"

#include <pthread.h>

#ifdef __cplusplus

namespace Hippo {

class UpgradeData;
class UpgradeManager;

class UpgradeBurner : public MessageHandler {
public:
    UpgradeBurner(UpgradeManager*, UpgradeData*);
    ~UpgradeBurner();

    virtual bool start();
    virtual bool stop();

    enum State {
    	UBS_OK,
    	UBS_ERROR
    };

    enum MessageCode {
        MC_Progress,
        MC_Timer			
    };
	
    State state() { return m_state;}
    void setState(State pState) { m_state = pState; }	
    int errorCode() {return m_errorCode; }
    void setErrorCode(int pErrorCode) { m_errorCode = pErrorCode; }
    UpgradeData* upgradeData() { return m_data; }
    UpgradeManager* upgradeManager() { return m_manager; }

    void updateProgress(int size);
    void updateEnd(void);	
    int upgradeSettingFile(const char*);
	
protected:
    /**
     * Subclasses must implement this to receive messages.
     */
    virtual void handleMessage(Message* msg);

protected:
    UpgradeManager* m_manager;
    UpgradeData* m_data;
    int m_dataSize;
    State m_state;	
    int m_errorCode;
    pthread_t m_thread;
};

} /* namespace Hippo */

#endif /* __cplusplus */

#endif /* _UpgradeBurner_H_ */

#ifndef _UpgradeChecker_H_
#define _UpgradeChecker_H_

#include "MessageHandler.h"

#ifdef __cplusplus

namespace Hippo {

class UpgradeSource;
class UpgradeManager;

class UpgradeChecker : public MessageHandler {
public:
    UpgradeChecker(UpgradeManager*);
    ~UpgradeChecker();

    enum State {
        UCS_IDLE,
        UCS_WORKING,
        UCS_OK,
        UCS_ERROR,
        UCS_SELECTED,
        UCS_FINISH
    };
    State state() { return m_state; }
    int type() { return m_type; }
    void setState(State pState) { m_state = pState;}
    int errorCode() { return m_errorCode; }    
    void setErrorCode(int pErrorCode) { m_errorCode = pErrorCode; }
	
    virtual bool start();
    virtual bool stop();
    virtual bool reset();

    virtual UpgradeSource* upgradeSource() { return m_source; }

protected:
    UpgradeManager* m_manager;
    UpgradeSource* m_source;
    State m_state;
    int m_type;
    int m_errorCode;	
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UpgradeChecker_H_

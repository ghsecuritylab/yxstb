#ifndef _BrowserAgentTakin_H_
#define _BrowserAgentTakin_H_

#include "BrowserAgent.h"

#ifdef __cplusplus

#define MAX_URL_LEN 2048 //看见多个url length的宏不知道该相信谁，见到最大2048

namespace Hippo {

class BrowserAgentTakin : public BrowserAgent {
public:
    BrowserAgentTakin();
    ~BrowserAgentTakin();

    virtual int openUrl(const char *url);

    virtual void handleMessage(Message *msg);

    virtual void closeAllConnection();

    virtual void setView(BrowserView *view);

    int getHandle();
    void setCurrentUrl(char *pUrl);
    char *getCurrentUrl();
    int getJvmStatus();
    void closeBrowser();
    void getTakinVersion(int *svn,  char **pTime, char **builder);
    virtual void setTakinSettings(int type, char *buffer, unsigned int bufferLen);
    void getTakinSettings(int type, char *buffer, unsigned int bufferLen);
    void cleanTakinCache();
    void setTakinEDSFlag(int flag);	

    int testRestore(char* url); //For Test

private:
    void enteringJvm();
    void exitingJvm();
    int  ReadJvmVersion(void);

    int *mBrowserHandle;
    char mCurrentUrl[MAX_URL_LEN];
    char *mOpeningUrl;
    int mJvmStatus;

};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserAgentTakin_H_


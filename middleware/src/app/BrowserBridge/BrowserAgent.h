#ifndef _BrowserAgent_H_
#define _BrowserAgent_H_

#include "PromptWidget.h"
#include "MessageHandler.h"
#include "PicSetWidget.h"

#ifdef __cplusplus

namespace Hippo {

class BrowserView;

class BrowserAgent : public MessageHandler {
public:
    BrowserAgent();
    ~BrowserAgent();

    typedef enum {
        PromptDebug,  // 0
        PromptWifiDisconnect,
        PromptWifiDisconnectAlert,
        PromptWifiNoSignal,
        PromptDisconnect,
        PromptDisconnectAlert,
        PromptPress,
        PromptDuplicate,
        PromptNtpFailed,  // 5
        PromptWifiLowQuality, // 6 PromptWifiLowQuality and PromptWifiNoSignal displayed alternately
        PromptErrorCode10030,
        PromptErrorCode10031,
        PromptDhcpFailed,
        PromptClearNetwork,
        PromptClearAll,
        PromptVisualizationDebug,
        PromptUsbInfo,
    } PromptStatus;


    virtual void handleMessage(Message *msg);

    virtual int openUrl(const char *url) = 0;
    virtual void closeAllConnection() = 0;

    virtual void setTakinSettings(int type, char *buffer, unsigned int bufferLen) = 0;
    virtual void setTakinEDSFlag(int flag) = 0;

    virtual void setView(BrowserView *view) { mView = view; }

    BrowserView *mView;
    static PromptWidget* mPrompt;
    static PicSetWidget* mWaitClock;
    static int NtpFailedShowed;
};

BrowserAgent &epgBrowserAgent();

} // namespace Hippo

#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void epgBrowserAgentCreate();
int epgBrowserAgentOpenUrl(const char* url);
int epgBrowserAgentGetHandle();
void epgBrowserAgentSetCurrentUrl(char *pUrl);
char *epgBrowserAgentGetCurrentUrl();
int epgBrowserAgentGetJvmStatus();
void epgBrowserAgentCloseBrowser();
void sendMessageToEPGBrowser(int what, int arg1, int arg2, unsigned int pDelayMillis);
void epgBrowserAgentGetTakinVersion(int *svn, char **pTime, char **builer);
void epgBrowserAgentSetTakinSettings(int type, char *buffer, int bufferLen);
void epgBrowserAgentCleanTakinCache();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _BrowserAgent_H_

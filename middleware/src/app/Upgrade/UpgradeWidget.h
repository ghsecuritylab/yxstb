#ifndef _UpgradeWidget_H_
#define _UpgradeWidget_H_

#include "Widget.h"
#include "Message.h"
#include "MessageHandler.h"
#include "Icon.h"
#ifdef __cplusplus

namespace Hippo {

class UpgradeWidget : public Widget, public MessageHandler {
public:
    enum {
        DownLoading = 0,
        Upgrading
    };
    
    enum {
        MC_UZIP_PROGRESS = 0x100,
        MC_REBOOT,
        MC_SHOW_ERROR
    };
    
    UpgradeWidget();
    ~UpgradeWidget();

    int setProgress(int percent);
    int getProgress();
    int setUpgradeState(int state);
    int getUpgradeState();

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);
    virtual void handleMessage(Message* msg);
    void showErrorMessage(int status);
private:  
    int mProgress;
    int mState;
    bool mReboot;	
    Icon* mErrorInfo;
    Icon* mErrorBackground;	
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgressBarWidget_H_

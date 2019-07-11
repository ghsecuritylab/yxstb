#ifndef _UpgradeDialog_H_
#define _UpgradeDialog_H_

#include "Dialog.h"
#include "Icon.h"

#ifdef __cplusplus

namespace Hippo {

class UpgradeManager;

class UpgradeDialog : public Dialog {
public:
    UpgradeDialog();
    ~UpgradeDialog();
    
    virtual bool handleMessage(Message *);
    virtual void draw();

protected:
    bool m_dialogFocus;

private:
    Icon UpgradeOK;
    Icon UpgradeOK1;
    Icon UpgradeCancel;
    Icon UpgradeCancel1;
    Icon UpgradeBackground;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UpgradeDialog_H_

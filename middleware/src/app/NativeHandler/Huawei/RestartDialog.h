#ifndef _RestartDialog_H_
#define _RestartDialog_H_

#include "Dialog.h"
#include "Icon.h"

#ifdef __cplusplus

namespace Hippo {

class RestartDialog : public Dialog {
public:
    RestartDialog();
    ~RestartDialog();
    
    virtual bool handleMessage(Message *);
    virtual void draw();

protected:
    bool m_dialogFocus;

private:
    Icon RestartOK;
    Icon RestartOK1;
    Icon RestartCancel;
    Icon RestartCancel1;
    Icon RestartBackground;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _RestartDialog_H_

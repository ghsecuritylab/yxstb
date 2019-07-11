#ifndef _PowerOffDialog_H_
#define _PowerOffDialog_H_

#include "Dialog.h"
#include "Icon.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerPublic;

class PowerOffDialog : public Dialog {
public:
    PowerOffDialog();
    ~PowerOffDialog();

    void setHandler(NativeHandlerPublic *handler);

    virtual bool handleMessage(Message *);
    virtual void draw();

protected:
    bool m_dialogFocus;
    NativeHandlerPublic* m_handle;

private:
    Icon StandbyOk;
    Icon StandbyOk1;
    Icon StandbyCancel;
    Icon StandbyCancel1;
    Icon StandbyBackground;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _PowerOffDialog_H_

#ifndef _DlnaDialog_H_
#define _DlnaDialog_H_

#include "Dialog.h"
#include "Icon.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerPublic;

class DlnaDialog : public Dialog {
public:
    DlnaDialog();
    ~DlnaDialog();

    void setHandler(NativeHandlerPublic *handler);

    virtual bool handleMessage(Message *);
    virtual void draw();

protected:
    bool m_dialogFocus;
    NativeHandlerPublic* m_handle;

private:
    Icon DlnaOk;
    Icon DlnaOk1;
    Icon DlnaCancel;
    Icon DlnaCancel1;
    Icon DlnaBackground;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _DlnaDialog_H_

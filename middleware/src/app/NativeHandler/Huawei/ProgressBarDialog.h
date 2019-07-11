#ifndef _ProgressBarDialog_H_
#define _ProgressBarDialog_H_

#include "Dialog.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerPublic;
class ProgressBarWidget;

class ProgressBarDialog : public Dialog {
public:
    ProgressBarDialog();
    ~ProgressBarDialog();

    void setHandler(NativeHandlerPublic *handler);

    virtual bool handleMessage(Message *);
    virtual void draw();

    virtual bool onClose();
protected:
    int     m_pos;
    NativeHandlerPublic* m_handle;

};

} // namespace Hippo

#endif // __cplusplus

#endif // _PowerOffDialog_H_

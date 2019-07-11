#ifndef _BrowserView_H_
#define _BrowserView_H_

#include "Widget.h"
#include "StandardScreen.h"

#ifdef __cplusplus

namespace Hippo {

class BrowserLayer : public StandardScreen {
public:
    BrowserLayer(Standard p_Standard, Canvas *canvas) : StandardScreen(p_Standard, canvas) {}
    ~BrowserLayer() {}

protected:
    virtual void onDraw(Canvas *) {}
};

class BrowserView : public Widget {
public:
    BrowserView(int platformLayer);
    ~BrowserView();

    void setContentSize(int, int);

    int mPlatformLayer;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserView_H_

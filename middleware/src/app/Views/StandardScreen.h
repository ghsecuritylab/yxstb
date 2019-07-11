#ifndef _StandardScreen_H_
#define _StandardScreen_H_

#include "Window.h"
#include "LayerMixerDevice.h"

#ifdef __cplusplus

namespace Hippo {

class Canvas;
class Widget;

class StandardScreen : public Window, public LayerMixerDevice::Layer {
public:
    StandardScreen(Standard pStandard, Canvas*);
    ~StandardScreen();

    virtual Standard standard() { return mCurrentStandard; }
    virtual void setStandard(Standard);
    virtual void setCanvasMaxSize(int pWidth, int pHeight);
    virtual void setCanvasValidSize(int pWidth, int pHeight, bool ForcedToRefresh);
    virtual void calculateCanvasOffset();
    virtual bool repaint(Canvas* canvas = 0, bool copy = 0);
    virtual bool dirty();

    virtual Widget* attachChildToFront(Widget* child);
    virtual Widget* attachChildToBack(Widget* child);
    virtual bool hasVisibleChild();

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);
    /** Override this to be notified when the view's size changes. Be sure to call the inherited version too */
    virtual void onSizeChange();

    virtual void onHandleInval(const Rect&);

private:
	Standard mCurrentStandard;
	Canvas* mCanvas;

    typedef Window INHERITED;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _StandardScreen_H_

#ifndef _StandardViewGroup_H_
#define _StandardViewGroup_H_

#include "View.h"
#include "LayerMixerDevice.h"

#ifdef __cplusplus

namespace Hippo {

class Canvas;
class Widget;
class StandardScreen;

class StandardViewGroup : public View, public LayerMixerDevice::Layer {
public:
    StandardViewGroup(Standard pStandard, StandardScreen *parent);
    ~StandardViewGroup();

    virtual Standard standard() { return mCurrentStandard; }
    virtual void setStandard(Standard);
    virtual void setCanvasMaxSize(int pWidth, int pHeight);
    virtual void setCanvasValidSize(int pWidth, int pHeight, bool ForcedToRefresh);
    virtual void calculateCanvasOffset();
    virtual bool repaint(Canvas* canvas = 0, bool copy = 0);
    virtual bool dirty();

    virtual Widget* attachChildToFront(Widget* child);
    virtual Widget* attachChildToBack(Widget* child);

protected:
    /** Override this to be notified when the view's size changes. Be sure to call the inherited version too */
    virtual void onSizeChange();

private:
	Standard mCurrentStandard;
	StandardScreen *mParent;

    typedef View INHERITED;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _StandardViewGroup_H_

#ifndef _BrowserLayerZebra_H_
#define _BrowserLayerZebra_H_

#include "StandardScreen.h"

#include "cairo/cairo.h"

#ifdef __cplusplus

namespace Hippo {

class BrowserLayerZebra : public StandardScreen {
public:
    BrowserLayerZebra(Standard initStandard);
    ~BrowserLayerZebra();

    virtual bool repaint(Canvas* canvas = 0, bool copy = 0);

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);
    /** Override this to be notified when the view's size changes. Be sure to call the inherited version too */
    virtual void onSizeChange();

private:
    int mBackingStoreLayer;
    cairo_surface_t *mCairoSurface;
    cairo_t *mCairo;
    Canvas* mBackingStore;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserLayerZebra_H_

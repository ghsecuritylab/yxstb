
#include "BrowserLayerZebra.h"

#include "BrowserView.h"
#include "BrowserAgent.h"
#include "Canvas.h"

#include "Message.h"
#include "MessageTypes.h"
#include "NativeHandler.h"

#include "libzebra.h"


namespace Hippo {

BrowserLayerZebra::BrowserLayerZebra(Standard initStandard)
	: StandardScreen(initStandard, 0)
	, mBackingStoreLayer(0)
	, mCairoSurface(0)
	, mCairo(0)
	, mBackingStore(0)
{
    unsigned char *buffer;
    int pitch;

    if (ygp_layer_createGraphics(SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, 0, &mBackingStoreLayer))
        return;
    ygp_layer_getMemory(mBackingStoreLayer, (int *)&buffer, &pitch);

    mCairoSurface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, pitch);
    mCairo = cairo_create(mCairoSurface);

    mBackingStore = new Canvas(mCairo);;
}

BrowserLayerZebra::~BrowserLayerZebra()
{
    if (mBackingStore) {
        delete mBackingStore;

        cairo_destroy(mCairo);
        cairo_surface_destroy(mCairoSurface);

        ygp_layer_destroyGraphics(mBackingStoreLayer);
    }
}

bool 
BrowserLayerZebra::repaint(Canvas* canvas, bool copy)
{
    bool ret = update(NULL, mBackingStore);
    //bool ret = update(NULL, canvas);
    //return true;

#if !defined(hi3560e)
    if (copy) { 
        BrowserView *browserView = epgBrowserAgent().mView; 
        ygp_layer_blitLayer(mPlatformLayer, 0, 0, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, mBackingStoreLayer, browserView->locX(), browserView->locY(), browserView->width(), browserView->height(), 
            YX_BLIT_COLOR_COPY_SOURCE, YX_BLIT_ALPHA_COPY_SOURCE, 0); 
    } 
    else { 
        BrowserView *browserView = epgBrowserAgent().mView; 
        ygp_layer_blitLayer(mPlatformLayer, 0, 0, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, mBackingStoreLayer, browserView->locX(), browserView->locY(), browserView->width(), browserView->height(), 
            YX_BLIT_COLOR_USE_SOURCE_ALPHA, YX_BLIT_ALPHA_COPY_COMBINE, 0); 
    } 
#endif
    //usleep(50 * 1000);

    return true;
}

void 
BrowserLayerZebra::onDraw(Canvas* canvas)
{
}

void 
BrowserLayerZebra::onSizeChange()
{
    StandardScreen::onSizeChange();

    mBackingStore->save();
    cairo_t* cr = mBackingStore->mCairo;
    cairo_rectangle(cr, 0, 0, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_fill(cr);
    mBackingStore->restore();
}

} // namespace Hippo

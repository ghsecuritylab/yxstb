
#include "LayerMixerDeviceZebra.h"

#include "Canvas.h"

#include "libzebra.h"


namespace Hippo {

LayerMixerDeviceZebra::LayerMixerDeviceZebra()
{
}

LayerMixerDeviceZebra::~LayerMixerDeviceZebra()
{
}

void
LayerMixerDeviceZebra::refresh(bool force)
{
    bool repainted = force;

    if (mTopLayer->dirty() || mMiddleLayer->dirty() || mBottomLayer->dirty()) {
        if (mTopLayer->hasVisibleChild() || mBottomLayer->hasVisibleChild()) {
            //printf("hasVisibleChild() t ...................\n");
            /* Clear the canvas. */
            Rect rect;
            rect.set(0, 0, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
            mCanvas->save();
            mCanvas->clipRect(rect, Region::kUnion_Op);
            cairo_t* cr = mCanvas->mCairo;
            cairo_rectangle(cr, 0, 0, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);
            cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
            cairo_fill(cr);
            mCanvas->restore();

            repainted |= bottomLayer()->repaint(mCanvas);
            repainted |= middleLayer()->repaint(mCanvas);
            repainted |= topLayer()->repaint(mCanvas);
        }
        else {
            //printf("hasVisibleChild() f ...................\n");
            repainted |= bottomLayer()->repaint(mCanvas, 1);
            repainted |= middleLayer()->repaint(mCanvas, 1);
            repainted |= topLayer()->repaint(mCanvas, 1);
        }
    }

    if (repainted) {
        ygp_layer_updateScreen();
    }
}

} // namespace Hippo

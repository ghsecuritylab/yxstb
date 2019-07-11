
#include "OSDLayer.h"

#include "BrowserView.h"
#include "Canvas.h"

#include "Message.h"
#include "MessageTypes.h"
#include "NativeHandler.h"

#include "libzebra.h"


namespace Hippo {

OSDLayer::OSDLayer(Standard initStandard)
	: StandardScreen(initStandard, 0)
{
}

OSDLayer::~OSDLayer()
{
}

bool 
OSDLayer::repaint(Canvas* canvas, bool copy)
{
    if (/*hasVisibleChild()*/ !copy) {
        Rect bounds;
        getLocalBounds(&bounds);
        fDirtyRgn.op(bounds, Region::kUnion_Op);

        return update(NULL, canvas);
    }
    else {
        fDirtyRgn.setEmpty();
        return false;
    }
}

void 
OSDLayer::onDraw(Canvas* canvas)
{
}

} // namespace Hippo

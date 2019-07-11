
#include "BrowserViewTakin.h"

#include "Canvas.h"
#include "LayerMixerDevice.h"

#include "libzebra.h"

extern "C" void TAKIN_browser_paint(int *handle, cairo_t* cr);
extern "C" int epgBrowserAgentGetHandle();

namespace Hippo {

BrowserViewTakin::BrowserViewTakin(int platformLayer)
	: BrowserView(platformLayer)
{
}

BrowserViewTakin::~BrowserViewTakin()
{
}

void
BrowserViewTakin::setContentSize(int pWidth, int pHeight)
{
    BrowserView::setContentSize(pWidth, pHeight);

    setSize(pWidth, pHeight);
}

void
BrowserViewTakin::onDraw(Canvas* canvas)
{
#ifdef NONE_BROWSER
#else

#if (defined(GRAPHIC_SINGLE_LAYER) || defined(BROWSER_INDEPENDENCE))
    TAKIN_browser_paint((int*)epgBrowserAgentGetHandle(), canvas->mCairo);
#endif
#endif
}

} // namespace Hippo

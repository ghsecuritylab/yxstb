
#include "BrowserView.h"

#include "LayerMixerDevice.h"


namespace Hippo {

static WidgetSource volatileSource = {StandardScreen::S576, 0, 0, 0, 0, 0, 0, 0};

BrowserView::BrowserView(int platformLayer)
	: Widget(&volatileSource)
{
    mPlatformLayer = platformLayer;
}

BrowserView::~BrowserView()
{
}

void 
BrowserView::setContentSize(int pWidth, int pHeight)
{
    volatileSource.standard = LayerMixerDevice::Layer::closeStandard(pWidth, pHeight);

    LayerMixerDevice::Layer::StandardSize &size = LayerMixerDevice::Layer::standardSize(volatileSource.standard);
    volatileSource.x = (size.width - pWidth) >> 1;
    volatileSource.y = (size.height - pHeight) >> 1;
    volatileSource.w = pWidth;
    volatileSource.h = pHeight;
}

} // namespace Hippo

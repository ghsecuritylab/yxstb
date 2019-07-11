
#include "ChannelLogoWidget.h"

#include "Canvas.h"


namespace Hippo {

ChannelLogoWidget::ChannelLogoWidget(WidgetSource *source)
	: Widget(source)
{
}

ChannelLogoWidget::~ChannelLogoWidget()
{
}

void 
ChannelLogoWidget::onDraw(Canvas* canvas)
{
    Rect bounds;
    
    getLocalBounds(&bounds);

    if(mSource->image){
        drawImage(canvas, &bounds, mSource->image, mSource->imageLength);
    }

    return ;
}

} // namespace Hippo


#include "ChannelNOWidget.h"

#include "Canvas.h"


namespace Hippo {

ChannelNOWidget::ChannelNOWidget(WidgetSource *source)
	: Widget(source), mChannelNO(-1)
{
}

ChannelNOWidget::~ChannelNOWidget()
{
}

int 
ChannelNOWidget::setChannelNO(int channelNO)
{
    int oldChannelNO = mChannelNO;
    mChannelNO = channelNO;
    inval(NULL);
    return oldChannelNO;
}

int 
ChannelNOWidget::getChannelNO()
{
    return mChannelNO;
}

void 
ChannelNOWidget::onDraw(Canvas* canvas)
{
}

} // namespace Hippo

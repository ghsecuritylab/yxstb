
#include "Canvas.h"

#include "DolbyWidget.h"

#include "SystemManager.h"
#include "Icon.h"

namespace Hippo {

DolbyWidget::DolbyWidget(WidgetSource *source)
	: Widget(source)
{
}

DolbyWidget::~DolbyWidget()
{
}

void 
DolbyWidget::onDraw(Canvas* canvas)
{
    Rect bounds;
    
    getLocalBounds(&bounds);
	 if(mSource->image){
		VIEW_LOG("drawImage [%s] start\n", (unsigned char*)mSource->image);
		Image img((unsigned char*)mSource->image);		
		drawImage(canvas, &bounds, &img); //draw	
    }
    if(mSource->text){
        drawText(canvas, &bounds, mSource->text);
    }
    return ;
}

} // namespace Hippo

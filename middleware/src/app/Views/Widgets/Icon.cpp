
#include "Icon.h"

#include "Canvas.h"


namespace Hippo {

Icon::Icon(WidgetSource *source)
	: Widget(source)
{
}

Icon::~Icon()
{
}

void 
Icon::onDraw(Canvas* canvas)
{
    Rect bounds;
	int length = 0;
	
    getLocalBounds(&bounds);
    if (mSource->image){
		VIEW_LOG("drawImage [%s] start\n", (unsigned char*)mSource->image);
		Image img((unsigned char*)mSource->image);		
		drawImage(canvas, &bounds, &img); //draw	
    }
    if (mSource->text)
        drawText(canvas, &bounds, mSource->text);
}

} // namespace Hippo

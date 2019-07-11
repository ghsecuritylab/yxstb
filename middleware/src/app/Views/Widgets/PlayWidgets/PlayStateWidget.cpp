
#include "PlayStateWidget.h"

#include "Canvas.h"


namespace Hippo {

PlayStateWidget::PlayStateWidget(WidgetSource *source)
	: Widget(source)
	, mState(StateMax)
{
    for (int i = 0;i < StateMax; i++) {
        mImage[i] = NULL;
        mImageLength[i] = 0;
    }
}

PlayStateWidget::~PlayStateWidget()
{
}

PlayStateWidget::State 
PlayStateWidget::setState(State state)
{
    State oldState = mState;
    mState = state;
    inval(NULL);
    return oldState;
}

PlayStateWidget::State 
PlayStateWidget::getState()
{
    return mState;
}

void 
PlayStateWidget::setStateImage(State state, void *image, int imageLength)
{
    if(state >= StateLive && state < StateMax && image){
        mImage[state] = image;
        mImageLength[state] = imageLength;
    }
    return ;
}

void 
PlayStateWidget::onDraw(Canvas* canvas)
{
    Rect bounds;
    
    getLocalBounds(&bounds);
	 if(mImage[mState]){
        mSource->image = mImage[mState];
    }
    if(mSource->image){
        if(mState == StateTimeShift) {
            bounds.fRight = bounds.fLeft + 42;
        }
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

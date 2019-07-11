
#include "Canvas.h"

#include "AudioMuteWidget.h"

#include "SystemManager.h"
#include "Icon.h"

namespace Hippo {

AudioMuteWidget::AudioMuteWidget(WidgetSource *source)
	: Widget(source)
	, mState(StateMax)
{
    for(int i = 0; i < StateMax; i ++){
        mImage[i] = NULL;
        mImageLength[i] = 0; 
    }
}

AudioMuteWidget::~AudioMuteWidget()
{
}

AudioMuteWidget::State 
AudioMuteWidget::setState(State state)
{
    State oldState = mState;
    mState = state;
    inval(NULL);
    return oldState;
}

AudioMuteWidget::State 
AudioMuteWidget::getState()
{
    return mState;
}

void 
AudioMuteWidget::setStateImage(State state, void *image, int imageLength)
{
    if(state >= StateUnmute && state < StateMax && image){
        mImage[state] = image;
        mImageLength[state] = imageLength;
    }
    return ;
}

void 
AudioMuteWidget::onDraw(Canvas* canvas)
{
    Rect bounds;
    
    getLocalBounds(&bounds);
	if(mImage[mState]){
        mSource->image = mImage[mState];
    }
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

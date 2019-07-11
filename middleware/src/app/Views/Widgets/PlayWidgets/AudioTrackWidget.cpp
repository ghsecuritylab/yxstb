
#include "AudioTrackWidget.h"

#include "Canvas.h"


namespace Hippo {

AudioTrackWidget::AudioTrackWidget(WidgetSource *source)
	: Widget(source)
	, mState(StateMax)
	, mTrackIndex(0)
{
    for(int i = 0; i < StateMax; i ++){
        mImage[i] = NULL;
        mImageLength[i] = 0; 
    }
}

AudioTrackWidget::~AudioTrackWidget()
{
}

AudioTrackWidget::State
AudioTrackWidget::setState(State state, int pIndex)
{
    State oldState = mState;

    mState = state;
    mTrackIndex = pIndex;
    inval(NULL);
    return oldState;
}

AudioTrackWidget::State 
AudioTrackWidget::getState()
{
    return mState;
}

void 
AudioTrackWidget::setStateImage(State state, void *image, int imageLength)
{
    if(state >= StateLR && state < StateMax && image){
        mImage[state] = image;
        mImageLength[state] = imageLength;
    }
    return ;
}

void 
AudioTrackWidget::onDraw(Canvas* canvas)
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

#ifndef _AudioMuteWidget_H_
#define _AudioMuteWidget_H_

#include "Widget.h"

#ifdef __cplusplus

namespace Hippo {

class AudioMuteWidget : public Widget {
public:
    enum State {
        StateUnmute = 0,
        StateMute,
    	StateMax,
    };

    AudioMuteWidget(WidgetSource *source);
    ~AudioMuteWidget();

    State setState(State state);
    State getState();

    void setStateImage(State state, void *image, int imageLength);

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);

private:
    State mState;
    void *mImage[StateMax];
    int mImageLength[StateMax];
};

} // namespace Hippo

#endif // __cplusplus

#endif // _AudioMuteWidget_H_

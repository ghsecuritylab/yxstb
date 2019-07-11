#ifndef _AudioTrackWidget_H_
#define _AudioTrackWidget_H_

#include "Widget.h"

#ifdef __cplusplus

namespace Hippo {

class AudioTrackWidget : public Widget {
public:
    enum State {
        StateLR = 0,
        StateLL,
        StateRR,
        StateRL,
    	StateMax,
    };

    AudioTrackWidget(WidgetSource *source);
    ~AudioTrackWidget();

    State setState(State, int);
    State getState();

    void setStateImage(State state, void *image, int imageLength);

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);

private:
    State mState;
    int mTrackIndex;
    void *mImage[StateMax];
    int mImageLength[StateMax];
};

} // namespace Hippo

#endif // __cplusplus

#endif // _AudioTrackWidget_H_

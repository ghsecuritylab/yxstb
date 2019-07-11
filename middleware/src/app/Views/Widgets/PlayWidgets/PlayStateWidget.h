#ifndef _PlayStateWidget_H_
#define _PlayStateWidget_H_

#include "Widget.h"

#ifdef __cplusplus

namespace Hippo {

class PlayStateWidget : public Widget {
public:
    enum State {
        StateLive = 0,
        StateTimeShift,
        StatePlay,
        StatePause,
        StateFW2,
        StateFW4,
        StateFW8,
        StateFW16,
        StateFW32,
        StateNext,
        StateRW2,
        StateRW4,
        StateRW8,
        StateRW16,
        StateRW32,
        StatePrevious,
        StateStop,
    	StateMax
    };

    PlayStateWidget(WidgetSource *source);
    ~PlayStateWidget();

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

#endif // _PlayStateWidget_H_

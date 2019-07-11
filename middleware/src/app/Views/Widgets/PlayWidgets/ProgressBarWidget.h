#ifndef _ProgressBarWidget_H_
#define _ProgressBarWidget_H_

#include "Widget.h"

#ifdef __cplusplus

namespace Hippo {

class ProgressBarWidget : public Widget {
public:
    enum State {
        StateSeek = 0,
        StateSeekError,
        StateProgress,
        StateMax,
    };

    enum FocusPos {
        PosSeeker = 0,
        PosHourL,
        PosHourR,
        PosMinL,
        PosMinR,
        PosSecL,
        PosSecR,
        PosMax,
    };

    ProgressBarWidget(WidgetSource *source);
    ~ProgressBarWidget();

    int setState(State s);
    State getState();

    int setFocusPos(FocusPos pos);
    FocusPos getFocusPos();

    void setStartTime(unsigned int _t) { mTimeL = _t; }
    void setEndTime(unsigned int _t) { mTimeR = _t; }
    void setCurrentTime(unsigned int _t);
    unsigned int getStartTime() { return mTimeL; }
    unsigned int getEndTime() { return mTimeR; }
    unsigned int getCurrentTime() { return mTimeC; }

    bool InputKey(int key);

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);

private:
    int setProgress(int percent);
    int getProgress();

    int             mProgress;
    State           mState;
    FocusPos        mFocusPos;
    FocusPos        mLastPos;
    unsigned int    mTimeL;
    unsigned int    mTimeR;
    unsigned int    mTimeC;
    char            mMaskH[10];
    char            mMaskM[10];
    char            mMaskS[10];
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgressBarWidget_H_

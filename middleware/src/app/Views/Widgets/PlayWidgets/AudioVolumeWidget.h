#ifndef _AudioVolumeWidget_H_
#define _AudioVolumeWidget_H_

#include "Widget.h"

#ifdef __cplusplus

namespace Hippo {

class AudioVolumeWidget : public Widget {
public:
    AudioVolumeWidget(WidgetSource *source);
    ~AudioVolumeWidget();

    int setMaxValue(int value) { mMaxValue = value; return 0; }

    int setValue(int value);
    int getValue();

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);

private:
    int mMaxValue;
    int mCurrentValue;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _AudioVolumeWidget_H_

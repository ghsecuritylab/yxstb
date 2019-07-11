#ifndef _DolbyDownmixWidget_H_
#define _DolbyDownmixWidget_H_

#include "Widget.h"

#ifdef __cplusplus

namespace Hippo {

class DolbyDownmixWidget : public Widget {
public:

    DolbyDownmixWidget(WidgetSource *source);
    ~DolbyDownmixWidget();

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);

};

} // namespace Hippo

#endif // __cplusplus

#endif // _AudioMuteWidget_H_

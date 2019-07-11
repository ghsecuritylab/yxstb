#ifndef _DolbyWidget_H_
#define _DolbyWidget_H_

#include "Widget.h"

#ifdef __cplusplus

namespace Hippo {

class DolbyWidget : public Widget {
public:

    DolbyWidget(WidgetSource *source);
    ~DolbyWidget();

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);

};

} // namespace Hippo

#endif // __cplusplus

#endif // _AudioMuteWidget_H_

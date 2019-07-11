#ifndef _ChannelLogoWidget_H_
#define _ChannelLogoWidget_H_

#include "Widget.h"

#ifdef __cplusplus

namespace Hippo {

class ChannelLogoWidget : public Widget {
public:
    ChannelLogoWidget(WidgetSource *source);
    ~ChannelLogoWidget();

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);

};

} // namespace Hippo

#endif // __cplusplus

#endif // _ChannelLogoWidget_H_

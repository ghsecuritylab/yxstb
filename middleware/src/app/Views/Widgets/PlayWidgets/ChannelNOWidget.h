#ifndef _ChannelNOWidget_H_
#define _ChannelNOWidget_H_

#include "Widget.h"

#ifdef __cplusplus

namespace Hippo {

class ChannelNOWidget : public Widget {
public:
    ChannelNOWidget(WidgetSource *source);
    ~ChannelNOWidget();

    int setChannelNO(int channelNO);
    int getChannelNO();

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);

private:
    int mChannelNO;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ChannelNOWidget_H_

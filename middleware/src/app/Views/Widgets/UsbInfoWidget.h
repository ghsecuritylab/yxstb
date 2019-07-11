#ifndef UsbInfoWidget_H_
#define UsbInfoWidget_H_


#include "Widget.h"
#include "Canvas.h"

#ifdef __cplusplus
#include <string>

namespace Hippo {

class UsbInfoWidget : public Widget {
public:
    UsbInfoWidget(WidgetSource *source);
    ~UsbInfoWidget();
    void setUsbInfo(std::string msg) { mMsg = msg;}

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    void onDraw(Canvas*);

private:
    std::string mMsg;
};

} // namespace Hippo

#endif // __cplusplus

#endif // UsbInfoWidget_H_

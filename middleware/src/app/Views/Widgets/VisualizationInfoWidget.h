#ifndef VisualizationInfoWidget_H_
#define VisualizationInfoWidget_H_

#include "Widget.h"
#include "Canvas.h"


#ifdef __cplusplus

namespace Hippo {

class VisualizationInfoWidget : public Widget {
public:
    VisualizationInfoWidget(WidgetSource *source);
    ~VisualizationInfoWidget();

    void setNextShowPage(int pageTYpe) { mNextShowPage = pageTYpe; }
protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    void onDraw(Canvas*);
    virtual int drawText(Canvas* canvas, const Rect *rect, const char *text, int fontSize);
    void dealStreamInfo(Canvas*, char*);

private:
    int mNextShowPage;
    char mVisualizationInfo[2048];
};

} // namespace Hippo

#endif // __cplusplus

#endif // VisualizationInfoWidget_H_

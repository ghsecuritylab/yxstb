#ifndef MaintenancePageWidget_H_
#define MaintenancePageWidget_H_

#include "Widget.h"
#include "Canvas.h"
#include "Image.h"


#ifdef __cplusplus

namespace Hippo {

class MaintenancePageWidget : public Widget {
public:
    MaintenancePageWidget(WidgetSource *source);
    ~MaintenancePageWidget();

    enum PIC_TYPE {
        UNKNOWN,
        BG_TYPE,
        FOCUSFRAME_TYPE,
    };

    void setCurrentFocusPos(int type)    { mFocusPos = type; }
    void setNextShowPage(int type)       { mNextShowPage = type; }
protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    void onDraw(Canvas*);
    void drawCommonPage(Canvas*);
    void drawDebugPage(Canvas*);
    void drawAutoDebutPage(Canvas*);
    void drawOtherPage(Canvas*);
    virtual int drawText(Canvas* canvas,const Rect *rect, const char *text, int fontSize);
private:
    char mPageMessage[1024];
    unsigned char *buffer;
    unsigned char *bufferTwo;
    int  mFocusPos;
    int  mNextShowPage;
	Image* mImageBG;
	Image* mImageFocusFrame;
};

} // namespace Hippo

#endif // __cplusplus

#endif // MaintenancePageWidget_H_

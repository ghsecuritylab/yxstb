#ifndef _LogoWidget_H_
#define _LogoWidget_H_

#include "Widget.h"

#ifdef __cplusplus

namespace Hippo {

class LogoWidget : public Widget {
public:
    LogoWidget(WidgetSource *source);
    ~LogoWidget();

    void setLogo(unsigned char *pImage, int pImageLen);
    unsigned char *getLogo(void);
        
protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);
private:
    unsigned char *mImage;
    int mImageLength;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _LogoWidget_H_

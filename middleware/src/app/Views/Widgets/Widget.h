#ifndef _Widget_H_
#define _Widget_H_

#include "View.h"
#include "StandardScreen.h"
#include "ViewAssertions.h"
#include <math.h>
#include "config/pathConfig.h"

#include "Image.h"
#include "Canvas.h"


#ifdef __cplusplus

namespace Hippo {

struct WidgetSource {
    StandardScreen::Standard standard;
    int x, y, w, h;
    char *text;
	void *image;   // the member image of the Struct WidgetSource is the path of images except logo or channel_logo show function;
	int imageLength; 
};

// range: 0 ~ 255
struct WidgetColor {
    WidgetColor():r(0),g(0),b(0),a(0){}
    WidgetColor(int _r, int _g, int _b, int _a) { r = _r; g = _g; b = _b; a = _a; }
    WidgetColor(const WidgetColor& o){ r = o.r; g = o.g; b = o.b; a = o.a; }
    void set(int _r, int _g, int _b, int _a) { r = _r; g = _g; b = _b; a = _a; }
    int r, g, b, a;
};

class Widget : public View {
public:
    Widget(WidgetSource *source);
    ~Widget();

    WidgetSource *getSource() { return mSource; }

protected:
    int int_img_info(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight);

    int drawText(Canvas* canvas, const Rect *rect, const char *text);
    int drawImage(Canvas* canvas,  Rect *rect, const void *image, int length);
	int drawImage(Canvas* canvas, Rect *rect, Image* image);
    int Rectangle(Canvas* canvas, const Rect& rect, const WidgetColor& color, bool bFill = true);
    int Arc(Canvas* canvas, const Point& pt, int radius, const WidgetColor& color, bool bFill = true, double angle1 = 0, double angle2 = M_PI * 2.0);

    WidgetSource *mSource;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _Widget_H_

#include <string.h>
#include "UsbInfoWidget.h"
#include "Assertions.h"


using namespace std;

#if (SUPPORTE_HD)
#define PNG_USBINFO_BG_HD_PATH    SYS_IMG_PATH_ROOT"/debug/usbWaringBG_HD.png"
#else
#define PNG_USBINFO_BG_SD_PATH    SYS_IMG_PATH_ROOT"/debug/usbWaringBG_SD.png"
#endif


namespace Hippo {

#if (SUPPORTE_HD)
static WidgetSource UsbInfoSource = {StandardScreen::S720, 0, 0, 430, 88, 0, (void *)PNG_USBINFO_BG_HD_PATH, 0};
#else
static WidgetSource UsbInfoSource = {StandardScreen::S576, 0, 0, 372, 70, 0, (void *)PNG_USBINFO_BG_SD_PATH, 0};
#endif

UsbInfoWidget::UsbInfoWidget(WidgetSource *source)
    : Widget(source)
{
}

UsbInfoWidget::~UsbInfoWidget()
{
}

void
UsbInfoWidget::onDraw(Canvas* canvas)
{
    Rect rect;
    std::string msg;
    std::string::size_type pos;
	
    cairo_t* cr = canvas->mCairo;
    cairo_save(cr);
    cairo_scale(cr, (double)width() / (double)(mSource->w), (double)height() / (double)(mSource->h)); //Ëõ·Å

    if (mMsg.empty())
        ERR_OUT("ERR, msg is NULL\n");
	VIEW_LOG("drawImage [%s] start\n", (unsigned char*)UsbInfoSource.image);
	rect.set(0, 0, UsbInfoSource.w, UsbInfoSource.h); //set canvas district
	{ 
		Image img((unsigned char*)UsbInfoSource.image);
		drawImage(canvas, &rect, &img); //draw
	} 

    pos = mMsg.find("sd");
    if (pos != mMsg.npos) {
        msg = mMsg.substr(pos, 3);

        if (mMsg.find("HDD_PARTITION_MOUNTED") != mMsg.npos) {
            msg += " \u5df2\u6210\u529f\u63a5\u5165";
        } else if (mMsg.find("HDD_REMOVED") != mMsg.npos) {
            msg += " \u5df2\u62d4\u51fa";
        } else
            ERR_OUT("ERR, can't find usb status\n");
    } else
        msg = mMsg;

#if (SUPPORTE_HD)
    rect.set(0, 35, mSource->w, 53);
#else
    rect.set(0, 28, mSource->w, 42);
#endif

    cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
    drawText(canvas, &rect, msg.c_str());

    cairo_restore(cr);
    return;

Err:
    cairo_restore(cr);
    return;
}
} // namespace Hippo

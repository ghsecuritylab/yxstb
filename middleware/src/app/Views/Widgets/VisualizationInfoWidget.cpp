#include "VisualizationDialog.h"
#include "VisualizationInfoWidget.h"
#include "ViewAssertions.h"

#include "mid_sys.h"

namespace Hippo {

VisualizationInfoWidget::VisualizationInfoWidget(WidgetSource *source)
	: Widget(source)
	, mNextShowPage(Hippo::VisualizationDialog::StreamInfoPage)
{
}

VisualizationInfoWidget::~VisualizationInfoWidget()
{
}

int
VisualizationInfoWidget::drawText(Canvas* canvas, const Rect *rect, const char *text, int fontSize)
{
    cairo_text_extents_t extents;
    cairo_t* cr = canvas->mCairo;
    double x, y;


    cairo_save(cr);

    cairo_set_font_size(cr, fontSize);

    x = (double)(rect->fLeft);
    y = (double)(rect->fTop);

    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text);

    cairo_restore(cr);
    return 0;
}
void
VisualizationInfoWidget::dealStreamInfo(Canvas* canvas, char *title)
{
    Rect rect;
    char *temp = NULL, *tempTwo = NULL;
    int len = 0;

#if (SUPPORTE_HD)
    int titleFont = 20, textFont = 18, lineGap = 29, firstXPOS = 160, twoXPOS = 690, YPOS = 109, maxLen = 42;
#else
    int titleFont = 18, textFont = 15, lineGap = 25, firstXPOS = 79, twoXPOS = 385, YPOS = 85, maxLen = 32;
#endif

    cairo_t* cr = canvas->mCairo;
    cairo_save(cr);

    cairo_set_source_rgba (cr, 0, 0, 0, 0.9);
    cairo_rectangle(cr, (double)(mSource->x), (double)(mSource->y), (double)(mSource->w), (double)(mSource->h));
    cairo_fill(cr);

#if (SUPPORTE_HD)
    rect.set(firstXPOS, 70, 20, 70);
#else
    rect.set(firstXPOS, 40, 27, 27);
#endif
    cairo_set_source_rgba(cr, 0.48, 0.73, 0.91, 0x01);
    drawText(canvas, &rect, title, titleFont);

    memset(mVisualizationInfo, 0, sizeof(mVisualizationInfo));
    mid_sys_getVisualizationInfo(mVisualizationInfo, sizeof(mVisualizationInfo));

    temp = mVisualizationInfo;
    tempTwo = mVisualizationInfo;
    int i = 0;
    while(NULL != (tempTwo = strstr(temp, "\n"))) {
        len = tempTwo - temp;
        if (len > maxLen)
            len = maxLen;
        temp[len] = '\0';
        if (18 >= i) {
            rect.set(firstXPOS, YPOS + i * lineGap, 25, 70);
            cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
            drawText(canvas, &rect, temp, textFont);
        } else if (18 < i && i <= 37){
            rect.set(twoXPOS, YPOS + (i - 19) * lineGap, 25, 70);
            cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
            drawText(canvas, &rect, temp, textFont);
        } else {
            VIEW_LOG_WARNING("do thing\n");
        }
        temp = tempTwo + 1;
        i++;
    }
    cairo_restore(cr);
}

void
VisualizationInfoWidget::onDraw(Canvas* canvas)
{
    char title[128] = {0};

    cairo_t* cr = canvas->mCairo;

    cairo_save(cr);
    cairo_scale(cr, (double)width() / (double)(mSource->w), (double)height() / (double)(mSource->h)); //Ëõ·Å

    switch(mNextShowPage) {
    case Hippo::VisualizationDialog::StreamInfoPage:
        strncpy(title, "\u663e\u793a\u64ad\u653e\u6d41\u5a92\u4f53\u4fe1\u606f", sizeof(title));
        dealStreamInfo(canvas, title);
        break;
    case Hippo::VisualizationDialog::OTTInfoPage:
        strncpy(title, "\u663e\u793aOTT\u8c03\u8bd5\u4fe1\u606f", sizeof(title));
        dealStreamInfo(canvas, title);
        break;
    case Hippo::VisualizationDialog::NULLBGPage:
        cairo_set_source_rgba (cr, 0, 0, 0, 0.7);
        cairo_rectangle(cr, (double)(mSource->x), (double)(mSource->y), (double)(mSource->w), (double)(mSource->h));
        cairo_fill(cr);
        break;
    default:
        break;
    }

    cairo_restore(cr);

    return;
}
} // namespace Hippo

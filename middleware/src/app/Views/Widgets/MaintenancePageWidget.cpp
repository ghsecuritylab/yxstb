#include "MaintenancePageWidget.h"
#include "SysSetting.h"
#include "ViewAssertions.h"
#include "VisualizationDialog.h"

#include "mid_sys.h"

#if (SUPPORTE_HD)
#define VISUALIZATION_POS_X (((1280-878)/2)&0xfffe)
#define VISUALIZATION_POS_Y (((720-563)/2)&0xfffe)
#else
#define VISUALIZATION_POS_X (((720-641)/2)&0xfffe)
#define VISUALIZATION_POS_Y (((576-449)/2)&0xfffe)
#endif

extern "C" {

int ind_img_draw(int depth, unsigned char* osd_buf, int osd_width, int osd_height, int osd_x, int osd_y, char *img_buf, int img_len);
}

#if (SUPPORTE_HD)
#define PNG_VISUALIZATION_BG_HD_PATH                    SYS_IMG_PATH_ROOT"/debug/visualizationBG_HD.png"
#define PNG_VISUALIZATION_SELECT_NO_HD_PATH             SYS_IMG_PATH_ROOT"/debug/visualization_selectNo_HD.png"
#define PNG_VISUALIZATION_SELECT_YES_HD_PATH            SYS_IMG_PATH_ROOT"/debug/visualization_selectYes_HD.png"
#define PNG_VISUALIZATION_GOBACK_SELECT_YES_HD_PATH     SYS_IMG_PATH_ROOT"/debug/visualizationGoback_selectYes_HD.png"
#define PNG_VISUALIZATION_ICON_HD_PATH                  SYS_IMG_PATH_ROOT"/debug/visualizationIcon_HD.png"
#define PNG_VISUALIZATION_CUTOFF_LINE_HD_PATH           SYS_IMG_PATH_ROOT"/debug/visualizationCutoffLine_HD.png"
#define PNG_VISUALIZATION_FOCUS_FRAME_HD_PATH           SYS_IMG_PATH_ROOT"/debug/visualizationFocusFrame_HD.png"
#else
#define PNG_VISUALIZATION_BG_SD_PATH                   SYS_IMG_PATH_ROOT"/debug/visualizationBG_SD.png"
#define PNG_VISUALIZATION_SELECT_NO_SD_PATH            SYS_IMG_PATH_ROOT"/debug/visualization_selectNo_SD.png"
#define PNG_VISUALIZATION_SELECT_YES_SD_PATH           SYS_IMG_PATH_ROOT"/debug/visualization_selectYes_SD.png"
#define PNG_VISUALIZATION_GOBACK_SELECT_YES_SD_PATH    SYS_IMG_PATH_ROOT"/debug/visualizationGoback_selectYes_SD.png"
#define PNG_VISUALIZATION_ICON_SD_PATH                 SYS_IMG_PATH_ROOT"/debug/visualizationIcon_SD.png"
#define PNG_VISUALIZATION_CUTOFF_LINE_SD_PATH          SYS_IMG_PATH_ROOT"/debug/visualizationCutoffLine_SD.png"
#define PNG_VISUALIZATION_FOCUS_FRAME_SD_PATH          SYS_IMG_PATH_ROOT"/debug/visualizationFocusFrame_SD.png"
#endif

namespace Hippo {

#if (SUPPORTE_HD)
static WidgetSource VisualizationBG = {Hippo::StandardScreen::S720, 0, 0, 878, 563, 0, (void *)PNG_VISUALIZATION_BG_HD_PATH, 0};
static WidgetSource selectNo = {Hippo::StandardScreen::S720, 75, 100, 64, 64, 0, (void *)PNG_VISUALIZATION_SELECT_NO_HD_PATH, 0};
static WidgetSource selectYes = {Hippo::StandardScreen::S720, 75, 100, 64, 64, 0, (void *)PNG_VISUALIZATION_SELECT_YES_HD_PATH, 0};
//static WidgetSource GoBackSelectNo = {Hippo::StandardScreen::S720, 341, 300, 230, 94, 0, app_gif_visualization_Goback_selectNO, strlen((const char *)app_gif_visualization_Goback_selectNO)};
static WidgetSource GoBackSelectYes = {Hippo::StandardScreen::S720, 324, 310, 230, 94, 0, (void *)PNG_VISUALIZATION_GOBACK_SELECT_YES_HD_PATH, 0};
static WidgetSource visualizationIcon = {Hippo::StandardScreen::S720, 39, 20, 32, 32, 0, (void *)PNG_VISUALIZATION_ICON_HD_PATH, 0};
static WidgetSource CutoffLine = {Hippo::StandardScreen::S720, 79, 167, 720, 1, 0, (void *)PNG_VISUALIZATION_CUTOFF_LINE_HD_PATH, 0};
static WidgetSource FocusFrame = {Hippo::StandardScreen::S720, 33, 70, 813, 118, 0, (void *)PNG_VISUALIZATION_FOCUS_FRAME_HD_PATH, 0};
#else
static WidgetSource VisualizationBG = {Hippo::StandardScreen::S576, 0, 0, 641, 449, 0, (void *)PNG_VISUALIZATION_BG_SD_PATH, 0};
static WidgetSource selectNo = {Hippo::StandardScreen::S576, 55,  88, 36, 36, 0, (void *)PNG_VISUALIZATION_SELECT_NO_SD_PATH, 0};
static WidgetSource selectYes = {Hippo::StandardScreen::S576,  55,  88, 36, 36, 0, (void *)PNG_VISUALIZATION_SELECT_YES_SD_PATH, 0};
static WidgetSource GoBackSelectYes = {Hippo::StandardScreen::S576, 232, 260, 174, 80, 0, (void *)PNG_VISUALIZATION_GOBACK_SELECT_YES_SD_PATH, 0};
static WidgetSource visualizationIcon = {Hippo::StandardScreen::S576, 34, 13, 24, 24, 0, (void *)PNG_VISUALIZATION_ICON_SD_PATH, 0};
static WidgetSource CutoffLine = {Hippo::StandardScreen::S576, 67, 135, 506, 1, 0, (void *)PNG_VISUALIZATION_CUTOFF_LINE_SD_PATH, 0};
static WidgetSource FocusFrame = {Hippo::StandardScreen::S576, 17, 53, 606, 104, 0, (void *)PNG_VISUALIZATION_FOCUS_FRAME_SD_PATH, 0};
#endif


MaintenancePageWidget::MaintenancePageWidget(WidgetSource *source)
	: Widget(source)
	, mFocusPos(0)
	, mNextShowPage(Hippo::VisualizationDialog::MainPage)
{
	mImageBG = new Image((unsigned char*)VisualizationBG.image);
	mImageFocusFrame = new Image((unsigned char*)FocusFrame.image);
}

MaintenancePageWidget::~MaintenancePageWidget()
{
	VIEW_LOG("~MaintenancePageWidget\n");
	cairo_surface_destroy(mImageBG->imgSurface);
    cairo_surface_destroy(mImageFocusFrame->imgSurface);
	delete mImageBG;
	delete mImageFocusFrame;
}

int
MaintenancePageWidget::drawText(Canvas* canvas, const Rect *rect, const char *text, int fontSize)
{
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
MaintenancePageWidget::drawCommonPage(Canvas* canvas)
{
    int lang = 1, i, len;
    char buffer[512] = {0};
    Rect rect;
#if (SUPPORTE_HD)
    int x, y, lineGap = 80, infoXPOS = 85, infoYPOS = 132, infoFontSize =26, infoFontSize2 = 22, titleFont = 20;
#else
    int x, y, lineGap = 61, infoXPOS = 55, infoYPOS = 110, infoFontSize = 20, infoFontSize2 = 18, titleFont = 16;
#endif

    cairo_t* cr = canvas->mCairo;
    cairo_save(cr);

    rect.set(visualizationIcon.x, visualizationIcon.y, visualizationIcon.x + visualizationIcon.w, visualizationIcon.y + visualizationIcon.h);
    if (visualizationIcon.image){
		VIEW_LOG("visualizationIcon drawImage [%s] start\n", (unsigned char*)visualizationIcon.image);
		Image img((unsigned char*)visualizationIcon.image);
		drawImage(canvas, &rect, &img); //draw

	}
    x = CutoffLine.x;
    y = CutoffLine.y;
    for (i = 1; i <= 3; i++) {
        rect.set(x, y, x + CutoffLine.w, y + CutoffLine.h);
		VIEW_LOG("CutoffLine drawImage [%s] start\n", (unsigned char*)CutoffLine.image);
		{
			Image img((unsigned char*)CutoffLine.image);
			drawImage(canvas, &rect, &img); //draw
		}
        y += lineGap;
    }

    x = FocusFrame.x;
    y = FocusFrame.y;
    if (FocusFrame.image) {
        switch(mFocusPos) {
        case Hippo::VisualizationDialog::MainPage_CollectSTBDebugInfo_Line:
            break;
        case Hippo::VisualizationDialog::MainPage_AutoCollectSTBDebugInfo_Line:
            y += lineGap;
            break;
        case Hippo::VisualizationDialog::MainPage_ShowStreamMediaInfo_Line:
            y += lineGap * 2;
            break;
        case Hippo::VisualizationDialog::MainPage_ShowOTTDebugInfo_Line:
            y += lineGap * 3;
            break;
        default:
            break;
        }
        rect.set(x, y, x + FocusFrame.w, y + FocusFrame.h);
		VIEW_LOG("FocusFrame ---drawImage--- [%s] start\n", (unsigned char*)FocusFrame.image);
		drawImage(canvas, &rect, mImageFocusFrame);
    }

    sysSettingGetInt("lang", &lang, 0);
    lang = 1;
    if (lang) {
#if (SUPPORTE_HD)
        rect.set(85, 42, 25, 25);
#else
        rect.set(65, 31, 27, 27);
#endif
        strncpy(buffer, "\u534e\u4e3aIPTV \u673a\u9876\u76d2\u8fd0\u7ef4\u9875\u9762\n", sizeof(buffer));
        cairo_set_source_rgba(cr, 0.48, 0.73, 0.91, 0x01);
        drawText(canvas, &rect, buffer, titleFont); //title

        rect.set(infoXPOS, infoYPOS, 0, 0);
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, "\u6536\u96c6\u673a\u9876\u76d2\u8c03\u8bd5\u4fe1\u606f", sizeof(buffer));
        cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
        drawText(canvas, &rect, buffer, infoFontSize); //Debug info
#if (SUPPORTE_HD)
        rect.set(700, infoYPOS, 0, 0);
#else
        rect.set(500, infoYPOS, 0, 0);
#endif
        memset(buffer, 0, sizeof(buffer));
        if (mid_sys_getStartCollectDebugInfo()) //Now is open, show open
            strncpy(buffer, "\u542f\u52a8", sizeof(buffer)); //open
         else //Now is close, show open
             strncpy(buffer, "\u505c\u6b62", sizeof(buffer)); //close
        cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
        drawText(canvas, &rect, buffer, infoFontSize2); //debug info

        rect.set(infoXPOS, infoYPOS + lineGap, 0, 0);
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, "\u673a\u9876\u76d2\u5f00\u673a\u65f6\u81ea\u52a8\u6536\u96c6\u8c03\u8bd5\u4fe1\u606f", sizeof(buffer));
        cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
        drawText(canvas, &rect, buffer, infoFontSize); //Auto debug info

        rect.set(infoXPOS, infoYPOS + lineGap * 2, 0, 0);
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, "\u663e\u793a\u64ad\u653e\u6d41\u5a92\u4f53\u4fe1\u606f", sizeof(buffer));
        cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
        drawText(canvas, &rect, buffer, infoFontSize); //show stream info

        rect.set(infoXPOS, infoYPOS + lineGap * 3, 0, 0);
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, "\u663e\u793aOTT\u8c03\u8bd5\u4fe1\u606f", sizeof(buffer));
        cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
        drawText(canvas, &rect, buffer, infoFontSize); //Show OTT info
    } else {
    ;
    }

    cairo_restore(cr);

}

void
MaintenancePageWidget::drawDebugPage(Canvas* canvas)
{
    int lang = 1;
    char buffer[128] = {0};
#if (SUPPORTE_HD)
    int lineGap = 100, infoXPOS = 162, infoYPOS = 140, textFont = 26, titleFont = 20;
#else
    int lineGap = 61, infoXPOS = 107, infoYPOS = 110, textFont = 20, titleFont = 16;
#endif

    Rect rect;

    cairo_t* cr = canvas->mCairo;
    cairo_save(cr);
    if (mid_sys_getStartCollectDebugInfo()) {
        rect.set(selectYes.x, selectYes.y, selectYes.x + selectYes.w, selectYes.y + selectYes.h);
        if (selectYes.image){
			VIEW_LOG("selectYes info:x[%d],y[%d],w[%d],h[%d]", selectYes.x, selectYes.y + lineGap, selectYes.x + selectYes.w, selectYes.y + selectYes.h + lineGap);
			VIEW_LOG("drawImage [%s] start\n", (unsigned char*)selectYes.image);
			Image img((unsigned char*)selectYes.image);
			drawImage(canvas, &rect, &img); //draw
		}

        rect.set(selectNo.x, selectNo.y + lineGap, selectNo.x + selectNo.w, selectNo.y + selectNo.h + lineGap);
        if (selectNo.image){
			VIEW_LOG("selectNo info:x[%d],y[%d],w[%d],h[%d]", selectNo.x, selectNo.y, selectNo.x + selectNo.w, selectNo.y + selectNo.h);
			VIEW_LOG("drawImage [%s] start\n", (unsigned char*)selectNo.image);
			{
				Image img((unsigned char*)selectNo.image);
				drawImage(canvas, &rect, &img); //draw
			}
		}
    } else {
        rect.set(selectNo.x, selectNo.y, selectNo.x + selectNo.w, selectNo.y + selectNo.h);
        if (selectNo.image){
			VIEW_LOG("selectNo info:x[%d],y[%d],w[%d],h[%d]", selectNo.x, selectNo.y, selectNo.x + selectNo.w, selectNo.y + selectNo.h);
			VIEW_LOG("drawImage [%s] start\n", (unsigned char*)selectNo.image);
			{
				Image img((unsigned char*)selectNo.image);
				drawImage(canvas, &rect, &img); //draw
			}
		}

        rect.set(selectYes.x, selectYes.y + lineGap, selectYes.x + selectYes.w, selectYes.y + selectYes.h + lineGap);
        if (selectYes.image){
			VIEW_LOG("selectYes info:x[%d],y[%d],w[%d],h[%d]", selectYes.x, selectYes.y + lineGap, selectYes.x + selectYes.w, selectYes.y + selectYes.h + lineGap);
			VIEW_LOG("drawImage [%s] start\n", (unsigned char*)selectYes.image);
			Image img((unsigned char*)selectYes.image);
			drawImage(canvas, &rect, &img); //draw
		}
    }

    if (Hippo::VisualizationDialog::DebugPage_Start_line == mFocusPos) {
        rect.set(FocusFrame.x, FocusFrame.y, FocusFrame.x + FocusFrame.w, FocusFrame.y + FocusFrame.h);
        if (FocusFrame.image){
			VIEW_LOG("FocusFrame info:x[%d],y[%d],w[%d],h[%d]", FocusFrame.x, FocusFrame.y, FocusFrame.x + FocusFrame.w, FocusFrame.y + FocusFrame.h);
			VIEW_LOG("FocusFrame drawImage [%s] start\n", (unsigned char*)FocusFrame.image);
			drawImage(canvas, &rect, mImageFocusFrame);
		}
    } else  if (Hippo::VisualizationDialog::DebugPage_Stop_line == mFocusPos){
        rect.set(FocusFrame.x, FocusFrame.y + lineGap, FocusFrame.x + FocusFrame.w, FocusFrame.y + FocusFrame.h+ lineGap);
        if (FocusFrame.image){
			VIEW_LOG("FocusFrame info:x[%d],y[%d],w[%d],h[%d]", FocusFrame.x, FocusFrame.y, FocusFrame.x + FocusFrame.w, FocusFrame.y + FocusFrame.h);
			VIEW_LOG("FocusFrame drawImage [%s] start\n", (unsigned char*)FocusFrame.image);
			drawImage(canvas, &rect, mImageFocusFrame);
		}
    }


#if (SUPPORTE_HD)
    rect.set(39, 45, 25, 47);
#else
    rect.set(36, 31, 27, 27);
#endif
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, "\u6536\u96c6\u673a\u9876\u76d2\u8c03\u8bd5\u4fe1\u606f", sizeof(buffer));
    cairo_set_source_rgba(cr, 0.48, 0.73, 0.91, 0x01);
    drawText(canvas, &rect, buffer, titleFont); //Title

    rect.set(infoXPOS, infoYPOS, 0, 0);
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, "\u542f\u52a8", sizeof(buffer));
    cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
    drawText(canvas, &rect, buffer, textFont); //Start

    rect.set(infoXPOS, infoYPOS + lineGap, 0, 0);
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, "\u505c\u6b62", sizeof(buffer));
    cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
    drawText(canvas, &rect, buffer, textFont); //Stop

    cairo_restore(cr);
}

void
MaintenancePageWidget::drawAutoDebutPage(Canvas* canvas)
{
    char buffer[128] = {0};
#if (SUPPORTE_HD)
    int infoXPOS = 200, infoYPOS = 260, textFont = 30, textFontTwo = 26, titleFont = 20;
#else
    int infoXPOS = 133, infoYPOS = 200, textFont = 22, textFontTwo = 20, titleFont = 16;
#endif
    Rect rect;

    cairo_t* cr = canvas->mCairo;
    cairo_save(cr);

#if (SUPPORTE_HD)
    rect.set(39, 45, 25, 47);
#else
    rect.set(36, 31, 27, 27);
#endif
    strncpy(buffer, "\u673a\u9876\u76d2\u5f00\u673a\u65f6\u81ea\u52a8\u6536\u96c6\u8c03\u8bd5\u4fe1\u606f", sizeof(buffer));
    cairo_set_source_rgba(cr, 0.48, 0.73, 0.91, 0x01);
    drawText(canvas, &rect, buffer, titleFont); //Title

    rect.set(infoXPOS, infoYPOS, 0, 0);
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, "\u8bf7\u91cd\u65b0\u542f\u52a8\u673a\u9876\u76d2\u4ee5\u4fbf\u4f7f\u8be5\u529f\u80fd\u751f\u6548", sizeof(buffer));
    cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
    drawText(canvas, &rect, buffer, textFont); //Info

    rect.set(GoBackSelectYes.x, GoBackSelectYes.y, GoBackSelectYes.x + GoBackSelectYes.w, GoBackSelectYes.y + GoBackSelectYes.h);
    if (GoBackSelectYes.image){
		VIEW_LOG("drawImage [%s] start\n", (unsigned char*)GoBackSelectYes.image);
		Image img((unsigned char*)GoBackSelectYes.image);
		drawImage(canvas, &rect, &img); //draw
	}

#if (SUPPORTE_HD)
    rect.set(410, 365, 25, 47);
#else
    rect.set(297, 310, 25, 47);
#endif
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, "\u786e\u5b9a", sizeof(buffer));
    cairo_set_source_rgba(cr, 0xff, 0xff, 0xff, 0x01);
    drawText(canvas, &rect, buffer, textFontTwo); //Goback button

    cairo_restore(cr);
}


void
MaintenancePageWidget::onDraw(Canvas* canvas)
{
    Rect rect;

    cairo_t* cr = canvas->mCairo;
    cairo_save(cr);
    cairo_scale(cr, (double)width() / (double)(mSource->w), (double)height() / (double)(mSource->h)); //Ëõ·Å

#if (SUPPORTE_HD)
    rect.set(0, 0, VisualizationBG.w, VisualizationBG.h);
	VIEW_LOG("VisualizationBG drawImage [%s] start.\n", (unsigned char*)VisualizationBG.image);
	drawImage(canvas, &rect, mImageBG); //draw

#else
    cairo_set_source_rgba (cr, 0x0, 0x0, 0x0, 0.8);
    cairo_rectangle(cr, 20, 0, (double)(mSource->w) - 40, (double)(mSource->h));
    cairo_fill(cr);
#endif

    switch(mNextShowPage) {
    case Hippo::VisualizationDialog::DebugPage:
        drawDebugPage(canvas);
        break;
    case Hippo::VisualizationDialog::AutoDebugPage:
        drawAutoDebutPage(canvas);
        break;
    case Hippo::VisualizationDialog::MainPage:
    default:
        drawCommonPage(canvas);
        break;

    }

    cairo_restore(cr);
    return;
}
} // namespace Hippo

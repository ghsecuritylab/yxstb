
#include <string.h>
#include "Widget.h" 

extern "C" {
int ind_img_draw(int depth, unsigned char* osd_buf, int osd_width, int osd_height, int osd_x, int osd_y, char *img_buf, int img_len);
int img_info_bmp(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight);
int img_info_gif(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight);
int img_info_jpg(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight);
int img_info_png(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight);
}

namespace Hippo {

Widget::Widget(WidgetSource *source)
	: mSource(source)
{
	VIEW_LOG("====widget construct mSource->image[%s], mSource->w[%d], mSource->h[%d].\n", mSource->image, mSource->w, mSource->h);
	/** 若未设置宽高，使用图片实际宽高 */
	if ((mSource->w == -1) || (mSource->h == -1)) {
		int imageWidth, imageHeight;
		if (strstr((const char*)mSource->image, "/") != NULL) { // mSourceImage is a path
			Image img((unsigned char*)mSource->image);  
			if (!img.getSize(&imageWidth, &imageHeight)) {
				mSource->w = imageWidth;
				mSource->h = imageHeight;
			}
		} else { // mSourceImage is image buffer
			if (!int_img_info((unsigned char *)mSource->image, mSource->imageLength, &imageWidth, &imageHeight)) {
				mSource->w = imageWidth;
				mSource->h = imageHeight;
			}
		}
    }
}

Widget::~Widget()
{
}

 int 
 Widget::int_img_info(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight)
{
	if (imgbuf[0] == 'S' && imgbuf[1] == 'T' && imgbuf[2] == 'B' && imgbuf[3] == 0) {
		imglen = (int)(((uint32_t)imgbuf[4] << 24) | ((uint32_t)imgbuf[5] << 16) | ((uint32_t)imgbuf[6] << 8) | ((uint32_t)imgbuf[7] << 0));
		imgbuf += 16;
	}

	if (imgbuf[0] == 'B' && imgbuf[1] == 'M') {
		if (img_info_bmp(imgbuf, imglen, pwidth, pheight))
			return -1;
	} else if (imgbuf[0] == 'G' && imgbuf[1] == 'I' && imgbuf[2] == 'F' && imgbuf[3] == '8') {
		if (img_info_gif (imgbuf, imglen, pwidth, pheight))
			return -1;
	} else if (imgbuf[0] == 0xFF && imgbuf[1] == 0xD8 && imgbuf[2] == 0xFF && (imgbuf[3] == 0xE0 || imgbuf[3] == 0xE1)) {
		if (img_info_jpg(imgbuf, imglen, pwidth, pheight))
			return -1;
//#ifdef ENABLE_PNG
	} else if (imgbuf[0] == 0x89 && imgbuf[1] == 0x50 && imgbuf[2] == 0x4E && imgbuf[3] == 0x47) {
		if (img_info_png(imgbuf, imglen, pwidth, pheight))
			return -1;
//#endif
	} else {
		return -1;
	}

	return 0;
}

int 
Widget::drawText(Canvas* canvas, const Rect *rect, const char *text)
{
    cairo_text_extents_t extents;
    cairo_t* cr = canvas->mCairo;
    double x, y;

    cairo_save(cr);
    cairo_translate(cr, rect->fLeft, rect->fTop);

    cairo_set_font_size(cr, rect->height());
    cairo_text_extents(cr, text, &extents);
    x = (rect->width() >> 1) - (extents.width/2 + extents.x_bearing);
    y = (rect->height() >> 1) - (extents.height/2 + extents.y_bearing);

    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text);

    cairo_restore(cr);
    return 0;
}

int 
Widget::drawImage(Canvas* canvas, Rect *rect, const void *image, int length)
{
    int imageWidth, imageHeight;

    if (!int_img_info((unsigned char *)image, length, &imageWidth, &imageHeight)) {
        //cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imageWidth, imageHeight);
        unsigned char *buffer = (unsigned char *)malloc(imageWidth * 4 * imageHeight);
        memset(buffer, 0, imageWidth * 4 * imageHeight);
        cairo_surface_t *surface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32, imageWidth, imageHeight, imageWidth * 4);
        ind_img_draw(32, cairo_image_surface_get_data(surface), imageWidth, imageHeight, 0 ,0, (char *)image, length);
        cairo_t* cr = canvas->mCairo;
        cairo_save(cr);

        // Draw the image.
        cairo_translate(cr, rect->fLeft, rect->fTop);
        cairo_scale(cr, ((double)rect->width())/imageWidth, ((double)rect->height())/imageHeight);
        cairo_set_source_surface(cr, surface, 0, 0);
        cairo_paint(cr);

        cairo_restore(cr);

        cairo_surface_destroy(surface);
        free(buffer);
    }
    return 0;
}

int 
Widget::drawImage(Canvas* canvas, Rect *rect, Image* image)
{
	if (NULL == image->imgSurface){
		VIEW_LOG("cairo_surface_t imgSurface is nil.\n");
		return -1;
	}
	VIEW_LOG("imgSurface[%p]\n", image->imgSurface);
	int imageWidth, imageHeight;
	image->getSize(&imageWidth, &imageHeight);
	
	cairo_t* cr = canvas->mCairo;
	cairo_save(cr);

	// Draw the image.
	cairo_translate(cr, rect->fLeft, rect->fTop);
	cairo_scale(cr, ((double)rect->width())/imageWidth, ((double)rect->height())/imageHeight);
	cairo_set_source_surface(cr, image->imgSurface, 0, 0);
	cairo_paint(cr);

	cairo_restore(cr);
	VIEW_LOG("cairo_paint ok. imgSurface[%p].\n", image->imgSurface);
	
	return 0;
}

int
Widget::Rectangle(Canvas* canvas, const Rect& rect, const WidgetColor& color, bool bFill)
{
    if(canvas == NULL)
        return 0;
    cairo_t* cr = canvas->mCairo;
    cairo_save(cr);
    cairo_set_source_rgba(cr, (double)color.r/255.0, (double)color.g/255.0, (double)color.b/255.0, (double)color.a/255.0);

    cairo_rectangle(cr, rect.fLeft, rect.fTop, rect.width(), rect.height());
    if(bFill)
        cairo_fill(cr);

    cairo_restore(cr);
    return 0;

}

int Widget::Arc(Canvas* canvas, const Point& pt, int radius, const WidgetColor& color, bool bFill,  double angle1, double angle2)
{
    if(canvas == NULL)
        return 0;

    cairo_t* cr = canvas->mCairo;
    cairo_save(cr);
    cairo_set_source_rgba(cr, (double)color.r/255.0, (double)color.g/255.0, (double)color.b/255.0, (double)color.a/255.0);

    cairo_arc(cr, pt.fX, pt.fY, radius, angle1, angle2);

    if(bFill)
        cairo_fill(cr);

    cairo_restore(cr);
    return 0;
}


} // namespace Hippo



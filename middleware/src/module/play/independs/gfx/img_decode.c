
#include "int_gfx.h"
#include "ind_gfx.h"
#include "img_decode.h"
#include "ind_mem.h"
#include <stdint.h>

#define RGBA_5551(r, g, b) (((b & 0xF8) >> 3) |    \
                            ((g & 0xF8) << 2) |    \
                            ((r & 0xF8) << 7) |    0x8000)

#define RGBA_8888(r, g, b, a) (    (b << 0) |    \
                                (g << 8) |    \
                                (r <<16) |    \
                                (a <<24))

static int img_draw5551(ind_plane_t plane, uint32_t trans, int x, int y, Rawimage *image, int i_x, int i_y, int i_width, int i_height)
{
    int i, j, idx, width, height, cmaplen;
    uint8_t *rp, *gp, *bp, *cmap, *inp;
    uint16_t *outp;

    cmap = image->cmap;
    cmaplen = image->cmaplen;
    width = image->width;
    height = image->height;

    //PRINTF("x = %d, y = %d, width = %d, height = %d\n", x, y, width, height);
    //PRINTF("i_x = %d, i_y = %d, i_width = %d, i_height = %d\n", i_x, i_y, i_width, i_height);

    if (i_x + i_width > width || i_y + i_height > height)
        ERR_OUT("i_x = %d, i_width = %d/%d, i_y = %d, i_height = %d/%d\n", i_x, i_width, width, i_y, i_height, height);

    if (i_width == 0)
        i_width = width - i_x;
    if (i_height == 0)
        i_height = height - i_y;

    if (x >= plane->width || y >= plane->height)
        ERR_OUT("%d %d, %d %d out rang exist!\n", x, y, plane->width, plane->height);

    if (x + i_width > plane->width)
        i_width = plane->width - x;
    if (y + i_height > plane->height)
        i_height = plane->height - y;

    switch(image->chandesc) {
    case CY_:
        if (image->nchans != 1)
            ERR_OUT("remap: Y image has %d chans\n", image->nchans);
        if (width * height != image->chanlen)
            ERR_OUT("width = %d, height = %d, chanlen = %d\n", width, height, image->chanlen);

        for (i = 0; i < i_height; i ++) {
            inp = image->chans[0] + (i_y + i) * width + i_x;
            outp = (uint16_t *)plane->buffer + plane->width * (y + i) + x;
            for (j = 0; j < i_width; j ++) {
                *outp = RGBA_5551((uint16_t)inp[0], (uint16_t)inp[0], (uint16_t)inp[0]);
                outp ++;
                inp ++;
            }
        }
        break;

    case CRGB1:
        if (image->nchans != 1)
            ERR_OUT("remap: can't handle nchans %d\n", image->nchans);
        if (image->gif_width * image->gif_height != image->chanlen)
            ERR_OUT("gif_width = %d, gif_height = %d, chanlen = %d\n", image->gif_width, image->gif_height, image->chanlen);
        if (cmap == nil)
            ERR_OUT("remap: image has no color map\n");

        if (i_x < image->gif_x) {
            if (i_x + i_width <= image->gif_x)
                return 0;
            i_width -= image->gif_x - i_x;
            if (i_width > image->gif_width)
                i_width = image->gif_width;
            i_x = image->gif_x;
        } else {
            int right = image->gif_x + image->gif_width;

            if (i_x >= right)
                return 0;
            if (i_x + i_width > right)
                i_width = right - i_x;
        }

        if (i_y < image->gif_y) {
            if (i_y + i_height <= image->gif_y)
                return 0;
            i_height -= image->gif_y - i_y;
            if (i_width > image->gif_height)
                i_width = image->gif_height;
            i_y = image->gif_y;
        } else {
            int bottom = image->gif_y + image->gif_height;

            if (i_y >= bottom)
                return 0;
            if (i_y + i_height > bottom)
                i_height = bottom - i_y;
        }

        for (i = 0; i < i_height; i ++) {
            //printf("line %02d ", i);
            inp = image->chans[0] + (i_y - image->gif_y + i) * image->gif_width + (i_x - image->gif_x);
            outp = (uint16_t *)plane->buffer + plane->width * (y + i) + x;
            for (j = 0; j < i_width; j ++, inp ++, outp ++) {
                //printf("%02x", *inp);
                idx = *inp;
                if (idx == image->gif_trindex)
                    continue;
                idx *= 3;
                if (idx < 0 || idx > cmaplen)
                    ERR_OUT("idx = %d, cmaplen = %d\n", idx, cmaplen);
                *outp = RGBA_5551((uint16_t)cmap[idx + 0], (uint16_t)cmap[idx + 1], (uint16_t)cmap[idx + 2]);
            }
            //printf("\n");
        }
        break;

    case CRGB_:
        if (image->nchans != 3)
            ERR_OUT("remap: can't handle nchans %d\n", image->nchans);
        if (width * height != image->chanlen)
            ERR_OUT("width = %d, height = %d, chanlen = %d\n", width, height, image->chanlen);

        for (i = 0; i < i_height; i ++) {
            rp = image->chans[0] + (i_y + i) * width + i_x;
            gp = image->chans[1] + (i_y + i) * width + i_x;
            bp = image->chans[2] + (i_y + i) * width + i_x;
            outp = (uint16_t *)plane->buffer + plane->width * (y + i) + x;
            for (j = 0; j < i_width; j ++) {
                *outp = RGBA_5551((uint16_t)rp[0], (uint16_t)gp[0], (uint16_t)bp[0]);
                outp ++;
                rp ++;
                gp ++;
                bp ++;
            }
        }
        break;

    case CRGBA32:
        if (image->nchans != 1)
            ERR_OUT("remap: can't handle nchans %d\n", image->nchans);
        if (width * height * 4 != image->chanlen)
            ERR_OUT("width = %d, height = %d, chanlen = %d\n", width, height, image->chanlen);

        for (i = 0; i < i_height; i ++) {
            inp = image->chans[0] + ((i_y + i) * width + i_x) * 4;    
            outp = (uint16_t *)plane->buffer + plane->width * (y + i) + x;
            for (j = 0; j < i_width; j ++) {
                if (inp[3])
                    *outp = RGBA_5551((uint32_t)inp[0], (uint32_t)inp[1], (uint32_t)inp[2]);
                outp ++;
                inp += 4;
            }
        }

    default:
        ERR_OUT("remap: can't recognize channel type %d\n", image->chandesc);
    }

    return 0;
Err:
    return -1;
}

//trans Í¸Ã÷É«
static int img_draw8888(ind_plane_t plane, uint32_t trans, int x, int y, Rawimage *image, int i_x, int i_y, int i_width, int i_height)
{
    int i, j, idx, width, height, cmaplen;
    uint8_t *rp, *gp, *bp, *cmap, *inp;
    uint32_t *outp, alpha;

    cmap = image->cmap;
    cmaplen = image->cmaplen;
    width = image->width;
    height = image->height;
    alpha = plane->alpha;

    if ((trans & 0xff000000) == 0xff000000)
        trans = (alpha << 24) | (trans & 0xffffff);
    else
        trans = 0;

    if (i_x + i_width > width || i_y + i_height > height)
        ERR_OUT("i_x = %d, i_width = %d/%d, i_y = %d, i_height = %d/%d\n", i_x, i_width, width, i_y, i_height, height);

    if (i_width == 0)
        i_width = width - i_x;
    if (i_height == 0)
        i_height = height - i_y;

    if (x >= plane->width || y >= plane->height)
        ERR_OUT("%d %d, %d %d out rang exist!\n", x, y, plane->width, plane->height);

    if (x + i_width > plane->width)
        i_width = plane->width - x;
    if (y + i_height > plane->height)
        i_height = plane->height - y;

    switch(image->chandesc) {
    case CY_:
        if (image->nchans != 1)
            ERR_OUT("remap: Y image has %d chans\n", image->nchans);
        if (width * height != image->chanlen)
            ERR_OUT("width = %d, height = %d, chanlen = %d\n", width, height, image->chanlen);

        for (i = 0; i < i_height; i ++) {
            inp = image->chans[0] + (i_y + i) * width + i_x;
            outp = (uint32_t *)plane->buffer + plane->width * (y + i) + x;
            for (j = 0; j < i_width; j ++) {
                *outp = RGBA_8888((uint32_t)inp[0], (uint32_t)inp[0], (uint32_t)inp[0], alpha);
                outp ++;
                inp ++;
            }
        }
        break;

    case CRGB1:
        if (image->nchans != 1)
            ERR_OUT("remap: can't handle nchans %d\n", image->nchans);
        if (image->gif_width * image->gif_height != image->chanlen)
            ERR_OUT("gif_width = %d, gif_height = %d, chanlen = %d\n", image->gif_width, image->gif_height, image->chanlen);
        if (cmap == nil)
            ERR_OUT("remap: image has no color map\n");

        if (i_x < image->gif_x) {
            if (i_x + i_width <= image->gif_x)
                return 0;
            i_width -= image->gif_x - i_x;
            if (i_width > image->gif_width)
                i_width = image->gif_width;
            i_x = image->gif_x;
        } else {
            int right = image->gif_x + image->gif_width;

            if (i_x >= right)
                return 0;
            if (i_x + i_width > right)
                i_width = right - i_x;
        }

        if (i_y < image->gif_y) {
            if (i_y + i_height <= image->gif_y)
                return 0;
            i_height -= image->gif_y - i_y;
            if (i_width > image->gif_height)
                i_width = image->gif_height;
            i_y = image->gif_y;
        } else {
            int bottom = image->gif_y + image->gif_height;

            if (i_y >= bottom)
                return 0;
            if (i_y + i_height > bottom)
                i_height = bottom - i_y;
        }

        for (i = 0; i < i_height; i ++) {
            inp = image->chans[0] + (i_y - image->gif_y + i) * image->gif_width + (i_x - image->gif_x);
            outp = (uint32_t *)plane->buffer + plane->width * (y + i) + x;
            for (j = 0; j < i_width; j ++, inp ++, outp ++) {
                idx = *inp;
                if (idx == image->gif_trindex)
                    continue;
                idx *= 3;
                if (idx < 0 || idx > cmaplen)
                    ERR_OUT("idx = %d, cmaplen = %d\n", idx, cmaplen);
                *outp = RGBA_8888((uint32_t)cmap[idx + 0], (uint32_t)cmap[idx + 1], (uint32_t)cmap[idx + 2], alpha);
            }
        }
        break;

    case CRGB_:
        if (image->nchans != 3)
            ERR_OUT("remap: can't handle nchans %d\n", image->nchans);
        if (width * height != image->chanlen)
            ERR_OUT("width = %d, height = %d, chanlen = %d\n", width, height, image->chanlen);

        for (i = 0; i < i_height; i ++) {
            rp = image->chans[0] + (i_y + i) * width + i_x;
            gp = image->chans[1] + (i_y + i) * width + i_x;
            bp = image->chans[2] + (i_y + i) * width + i_x;
            outp = (uint32_t *)plane->buffer + plane->width * (y + i) + x;
            if (trans) {
                uint32_t color;
                for (j = 0; j < i_width; j ++) {
                    color = RGBA_8888((uint32_t)rp[0], (uint32_t)gp[0], (uint32_t)bp[0], alpha);
                    if (color != trans)
                        *outp = color;
                    outp ++;
                    rp ++;
                    gp ++;
                    bp ++;
                }
            } else {
                for (j = 0; j < i_width; j ++) {
                    *outp = RGBA_8888((uint32_t)rp[0], (uint32_t)gp[0], (uint32_t)bp[0], alpha);
                    outp ++;
                    rp ++;
                    gp ++;
                    bp ++;
                }
            }
        }
        break;

    case CRGBA32://png
        if (image->nchans != 1)
            ERR_OUT("remap: can't handle nchans %d\n", image->nchans);
        if (width * height * 4 != image->chanlen)
            ERR_OUT("width = %d, height = %d, chanlen = %d\n", width, height, image->chanlen);

        for (i = 0; i < i_height; i ++) {
            uint32_t r, g, b, a, r0, g0, b0, a0;
            inp = image->chans[0] + ((i_y + i) * width + i_x) * 4;    
            outp = (uint32_t *)plane->buffer + plane->width * (y + i) + x;
            for (j = 0; j < i_width; j ++) {
                a = (uint32_t)inp[0];
                if (a) {
                    r = (uint32_t)inp[3];
                    g = (uint32_t)inp[2];
                    b = (uint32_t)inp[1];
                    if (a != 0xff) {
                        a0 = (outp[0] >> 24) & 0xff;
                        if (a0) {
                            r0 = (outp[0] >> 16) & 0xff;
                            g0 = (outp[0] >>  8) & 0xff;
                            b0 = (outp[0] >>  0) & 0xff;

                            r = (r0 * (255 - a) + r * a) / 255;
                            g = (g0 * (255 - a) + g * a) / 255;
                            b = (b0 * (255 - a) + b * a) / 255;

                            a = a0;
                        }
                    }
                    if (alpha != 0xff)
                        a = alpha;
                    outp[0] = RGBA_8888(r, g, b, a);
                }
                outp ++;
                inp += 4;
            }
        }
        break;

    case CRGB24://png
        if (image->nchans != 1)
            ERR_OUT("remap: can't handle nchans %d\n", image->nchans);
        if (width * height * 3 != image->chanlen)
            ERR_OUT("width = %d, height = %d, chanlen = %d\n", width, height, image->chanlen);

        for (i = 0; i < i_height; i ++) {
            inp = image->chans[0] + ((i_y + i) * width + i_x) * 3;    
            outp = (uint32_t *)plane->buffer + plane->width * (y + i) + x;
            for (j = 0; j < i_width; j ++) {
                outp[0] = RGBA_8888((uint32_t)inp[2], (uint32_t)inp[1], (uint32_t)inp[0], alpha);
                outp ++;
                inp += 3;
            }
        }
        break;

    default:
        ERR_OUT("remap: can't recognize channel type %d\n", image->chandesc);
    }

    return 0;
Err:
    return -1;
}

int ind_img_info(uint8_t *imgbuf, int imglen, int *pwidth, int *pheight)
{
    if (imgbuf[0] == 'S' && imgbuf[1] == 'T' && imgbuf[2] == 'B' && imgbuf[3] == 0) {
        imglen = (int)(((uint32_t)imgbuf[4] << 24) | ((uint32_t)imgbuf[5] << 16) | ((uint32_t)imgbuf[6] << 8) | ((uint32_t)imgbuf[7] << 0));
        imgbuf += 16;
    }

    if (imgbuf[0] == 'B' && imgbuf[1] == 'M') {
        if (img_info_bmp(imgbuf, imglen, pwidth, pheight))
            ERR_OUT("img_info_bmp\n");
    } else if (imgbuf[0] == 'G' && imgbuf[1] == 'I' && imgbuf[2] == 'F' && imgbuf[3] == '8') {
        if (img_info_gif (imgbuf, imglen, pwidth, pheight))
            ERR_OUT("img_info_gif\n");
    } else if (imgbuf[0] == 0xFF && imgbuf[1] == 0xD8 && imgbuf[2] == 0xFF && (imgbuf[3] == 0xE0 || imgbuf[3] == 0xE1)) {
        if (img_info_jpg(imgbuf, imglen, pwidth, pheight))
            ERR_OUT("img_info_jpg\n");
    } else if (imgbuf[0] == 0x89 && imgbuf[1] == 0x50 && imgbuf[2] == 0x4E && imgbuf[3] == 0x47) {
        if (img_info_png(imgbuf, imglen, pwidth, pheight))
            ERR_OUT("img_info_png\n");
    } else {
        ERR_OUT("unknow image!\n");
    }

    return 0;
Err:
    return -1;
}

int int_img_draw(int depth, ind_plane_t plane, uint32_t trans, int x, int y, uint8_t *imgbuf, int imglen, int i_x, int i_y, int i_width, int i_height, unsigned int alpha)
{
    int i;
    int ret = -1;
    Rawimage *image = NULL;

    if (plane == NULL || imgbuf == NULL)
        ERR_OUT("plane == %p, imgbuf == %p\n", plane, imgbuf);

    if (imgbuf[0] == 'S' && imgbuf[1] == 'T' && imgbuf[2] == 'B' && imgbuf[3] == 0) {
        imglen = (int)(((uint32_t)imgbuf[4] << 24) | ((uint32_t)imgbuf[5] << 16) | ((uint32_t)imgbuf[6] << 8) | ((uint32_t)imgbuf[7] << 0));
        imgbuf += 16;
    }

    if (imgbuf[0] == 'B' && imgbuf[1] == 'M') {
        image = img_decode_bmp(imgbuf, imglen);
        if (image == NULL)
            ERR_OUT("img_decode_bmp\n");
    } else if (imgbuf[0] == 'G' && imgbuf[1] == 'I' && imgbuf[2] == 'F' && imgbuf[3] == '8') {
        if (int_gif_check(imgbuf, imglen))
            ERR_OUT("ind_gif_check\n");
        image = img_decode_gif (imgbuf, imglen);
        if (image == NULL)
            ERR_OUT("img_decode_gif\n");
    } else if (imgbuf[0] == 0xFF && imgbuf[1] == 0xD8 && imgbuf[2] == 0xFF && (imgbuf[3] == 0xE0 || imgbuf[3] == 0xE1)) {
        if (int_jpg_check(imgbuf, imglen))
            ERR_OUT("ind_gif_check\n");
        image = img_decode_jpg(imgbuf, imglen);
        if (image == NULL)
            ERR_OUT("img_decode_jpg\n");
    } else if (imgbuf[0] == 0x89 && imgbuf[1] == 0x50 && imgbuf[2] == 0x4E && imgbuf[3] == 0x47) {
        image = img_decode_png(imgbuf, imglen);
        if (image == NULL)
            ERR_OUT("img_decode_png\n");
    } else {
        ERR_OUT("unknow image!\n");
    }

    if (depth == 16)
        img_draw5551(plane, trans, x, y, image, i_x, i_y, i_width, i_height);
    else if (depth == 32)
        img_draw8888(plane, trans, x, y, image, i_x, i_y, i_width, i_height);
    else
        ERR_OUT("depth = %d not support!\n", depth);

    ret = 0;
Err:
    if (image) {
        for (i=0; i<image->nchans; i++)
            IND_FREE(image->chans[i]);
        if (image->cmap)
            IND_FREE(image->cmap);
        IND_FREE(image);
    }
    return ret;
}

int ind_img_draw(int depth, char* osd_buf, int osd_width, int osd_height, int osd_x, int osd_y, 
    char *img_buf, int img_len)
{
    struct ind_plane plane;
    int img_width, img_height;

    if (osd_buf == NULL || img_buf == NULL)
        ERR_OUT("plane == %p, imgbuf == %p\n", osd_buf, img_buf);

    plane.buffer = (uint8_t*)osd_buf;
    plane.width = osd_width;
    plane.height = osd_height;
    plane.alpha = 0xff;

    if (ind_img_info((uint8_t*)img_buf, img_len, &img_width, &img_height))
        ERR_OUT("ind_img_info\n");
    int_img_draw(depth, &plane, 0, osd_x, osd_y, (uint8_t*)img_buf, img_len, 0, 0, img_width, img_height, 0);

    return 0;
Err:
    return -1;
}

int ind_img_check(uint8_t *buf, int len)
{
    if (buf[0] == 'G' && buf[1] == 'I' && buf[2] == 'F' && buf[3] == '8')
        return int_gif_check(buf, len);

    if (buf[0] == 0xFF && buf[1] == 0xD8 && buf[2] == 0xFF && (buf[3] == 0xE0 || buf[3] == 0xE1))
        return int_jpg_check(buf, len);

    ERR_PRN("unsupported format!\n");
    return -1;
}

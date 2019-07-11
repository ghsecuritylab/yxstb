
#include "BootImagesCheck.h"

#include "Assertions.h"

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#include "gfx/img_decode.h"
#include "png.h"
extern "C"{
#include "jpeglib.h"
}

#define _htole32(data) (data & 0x000000ff) << 24 | (data & 0x0000ff00) << 8 | (data & 0x00ff0000) >> 8 | (data & 0xff000000) >> 24

struct JPGErrorMgr {
    struct jpeg_error_mgr nErrMgr;
    jmp_buf nJmpBuff;
};

typedef struct JPGErrorMgr* JPGErrorMgr_t;

typedef struct _BmpFileHead {
    unsigned char sType[2];
    unsigned int sSize;
    unsigned int sReserved;
    unsigned int sDataOff;
}BmpFileHead_t;

typedef struct _BmpInfoHead {
    unsigned int sSize;
    unsigned int sWidth;
    unsigned int sHeight;
    unsigned short sPlanes;
    unsigned short sBitCount;
    unsigned int sCompression;
    unsigned int sSizeImage;
    unsigned int sXPelsPerMeter;
    unsigned int sYPelsPerMeter;
    unsigned int sClrUsed;
    unsigned int sClrImportant;
}BmpInfoHead_t;

namespace Hippo {

/* JPG */
static void _JPGErrorMessageFunc(j_common_ptr cInfo)
{
    LogSysOperError("\n");
    JPGErrorMgr_t jpgErrMgr = (JPGErrorMgr_t)cInfo->err;
    (*cInfo->err->output_message) (cInfo);
    longjmp(jpgErrMgr->nJmpBuff, 1);
}

static void _JPGWarnMessageFunc(j_common_ptr cInfo, int code)
{
    JPGErrorMgr_t jpgErrMgr = (JPGErrorMgr_t)cInfo->err;
    if (code < 0) {
        LogSysOperWarn("warn code[%d]\n", code);
        (*cInfo->err->output_message) (cInfo);
        longjmp(jpgErrMgr->nJmpBuff, 1);
    }
}

static void _PNGErrorMessageFunc(png_structp pngStructp, png_const_charp msg)
{
    LogSysOperError("%s\n", msg);
    longjmp(png_jmpbuf(pngStructp), 1);
}

static void _PNGWarnMessageFunc(png_structp pngStructp, png_const_charp msg)
{
    LogSysOperWarn("%s\n", msg);
    longjmp(png_jmpbuf(pngStructp), 1);
}


int BootImagesCheckJPG(const char* filePath, unsigned long* width, unsigned long* height)
{/*{{{*/
    struct JPGErrorMgr jpgErrMgr;
    struct jpeg_decompress_struct jpgDecompress;
    JSAMPARRAY jpgBuffer;

    FILE* jpgFp = fopen(filePath, "rb");
    if (!jpgFp)
        return -1;

    jpgDecompress.err = jpeg_std_error(&jpgErrMgr.nErrMgr);
    jpgErrMgr.nErrMgr.error_exit = _JPGErrorMessageFunc;
    jpgErrMgr.nErrMgr.emit_message = _JPGWarnMessageFunc;

    if (setjmp(jpgErrMgr.nJmpBuff)) {
        jpeg_destroy_decompress(&jpgDecompress);
        fclose(jpgFp);
        return -1;
    }

    jpeg_create_decompress(&jpgDecompress);
    jpeg_stdio_src(&jpgDecompress, jpgFp);
    jpeg_read_header(&jpgDecompress, TRUE);

    if (width)
        *width = jpgDecompress.image_width;
    if (height)
        *height = jpgDecompress.image_height;

    jpeg_start_decompress(&jpgDecompress);

    jpgBuffer = (*jpgDecompress.mem->alloc_sarray)((j_common_ptr)&jpgDecompress, JPOOL_IMAGE, jpgDecompress.output_width * jpgDecompress.output_components, 1);

    while ( jpgDecompress.output_scanline < jpgDecompress.output_height )
        jpeg_read_scanlines(&jpgDecompress, jpgBuffer, 1);

    jpeg_finish_decompress(&jpgDecompress);
    jpeg_destroy_decompress(&jpgDecompress);

    fclose(jpgFp);
    return 0;
}/*}}}*/

/* PNG */
int BootImagesCheckPNG(const char* filePath, unsigned long* width, unsigned long* height)
{/*{{{*/
    png_structp pngStructp;
    png_infop pngInfop;
    png_uint_32 lwidth = 0, lheight = 0;
    int lDepth = 0, lColor = 0;

    FILE* pngFp;
    pngFp = fopen(filePath, "rb");
    if (!pngFp)
        return -1;

    pngStructp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, _PNGErrorMessageFunc, _PNGWarnMessageFunc);
    if (!pngStructp) {
        fclose(pngFp);
        return -1;
    }

    pngInfop = png_create_info_struct(pngStructp);
    if(!pngInfop) {
        png_destroy_read_struct(&pngStructp, NULL, NULL);
        fclose(pngFp);
        return -1;
    }

    if (setjmp(png_jmpbuf(pngStructp))) {
        png_destroy_read_struct(&pngStructp, &pngInfop, NULL);
        fclose(pngFp);
        return -1;
    }

    png_init_io(pngStructp, pngFp);
	png_read_png(pngStructp, pngInfop, PNG_TRANSFORM_IDENTITY, NULL);
    png_get_IHDR(pngStructp, pngInfop, &lwidth, &lheight, &lDepth, &lColor, NULL, NULL, NULL);

    if (width)
        *width = lwidth;
    if (height)
        *height = lheight;
    png_destroy_read_struct(&pngStructp, &pngInfop, NULL);
    fclose(pngFp);
    return 0;
}/*}}}*/

/* GIF */
int BootImagesCheckGIF(const char* filePath, unsigned long* width, unsigned long* height)
{/*{{{*/
    //TODO has not find best method yet; use old method instead, but someting is wrong with some gif pictures.
    FILE* gifFp = NULL;
    long lSize = 0;
    unsigned char* buff = NULL;
    int ret = -1;
    int imgWidth = 0, imgHeight = 0;

    gifFp = fopen(filePath, "rb");
    if (!gifFp)
        return -1;
    fseek(gifFp, 0L, SEEK_END);
    lSize = ftell(gifFp);
    rewind(gifFp);
    if (!(buff = (unsigned char*)calloc(1, lSize + 1))) {
        fclose(gifFp);
        return -1;
    }
    if (fread(buff, 1, lSize, gifFp) == lSize) {
        ret = int_gif_check(buff, lSize);
        if (0 == ret) {
            img_info_gif(buff, lSize, &imgWidth, &imgHeight);
            if (width)
                *width = imgWidth;
            if (height)
                *height = imgHeight;
        }
    }
    fclose(gifFp);
    free(buff);
    return ret;
}/*}}}*/

/* BMP */
int BootImagesCheckBMP(const char* filePath, unsigned long* width, unsigned long* height)
{/*{{{*/
    //Warn: this is a simple checker, only compare file size.
    FILE* bmpFp = NULL;
    bmpFp = fopen(filePath, "rb");
    if (!bmpFp)
        return -1;
    BmpFileHead_t bFileHead;
    BmpInfoHead_t bInfoHead;

    fseek(bmpFp, 0L, SEEK_END);
    long lSize = ftell(bmpFp);
    rewind(bmpFp);

    //Read file type
    if (fread(&bFileHead.sType, sizeof(bFileHead.sType), 1, bmpFp) <= 0)
        goto Err;
    if ('B' != bFileHead.sType[0] && 'M' != bFileHead.sType[1]) {
        LogSysOperWarn("%c%c type not match!\n", bFileHead.sType[0], bFileHead.sType[1]);
        goto Err;
    }
    //Read file size
    if (fread(&bFileHead.sSize, sizeof(bFileHead.sSize), 1, bmpFp) <= 0)
        goto Err;
    if (lSize != bFileHead.sSize) {
		bFileHead.sSize = _htole32(bFileHead.sSize);
        if(lSize != bFileHead.sSize) {
            LogSysOperWarn("[%ld] vs [%d] size not match!\n", lSize, bFileHead.sSize);
            goto Err;
        }
        goto out;
    }
    //Skip some bytes
    fseek(bmpFp, sizeof(bFileHead.sReserved) + sizeof(bFileHead.sDataOff), SEEK_CUR);

    //Read info size
    if (fread(&bInfoHead.sSize, sizeof(bInfoHead.sSize), 1, bmpFp) <= 0)
        goto Err;
    if (bInfoHead.sSize > lSize) {
        LogSysOperWarn("size[%d]!\n", bInfoHead.sSize);
        goto Err;
    }

    //Read info width
    if (fread(&bInfoHead.sWidth, sizeof(bInfoHead.sWidth), 1, bmpFp) <= 0)
        goto Err;
    if (width)
        *width = bInfoHead.sWidth;
    //Read info height
    if (fread(&bInfoHead.sHeight, sizeof(bInfoHead.sHeight), 1, bmpFp) <= 0)
        goto Err;
    if (height)
        *height = bInfoHead.sHeight;

    LogSysOperDebug("bmp w[%d] h[%d]\n", *width, *height);
    fclose(bmpFp);
    return 0;

out:
    fseek(bmpFp, sizeof(bFileHead.sReserved) + sizeof(bFileHead.sDataOff), SEEK_CUR);

    //Read info size
    if (fread(&bInfoHead.sSize, sizeof(bInfoHead.sSize), 1, bmpFp) <= 0)
        goto Err;
    bInfoHead.sSize = _htole32(bInfoHead.sSize);
    if (bInfoHead.sSize > lSize) {
        LogSysOperWarn("size[%d]!\n", bInfoHead.sSize);
        goto Err;
    }
    //Read info width
    if (fread(&bInfoHead.sWidth, sizeof(bInfoHead.sWidth), 1, bmpFp) <= 0)
        goto Err;
    bInfoHead.sWidth = _htole32(bInfoHead.sWidth);
    if (width)
        *width = bInfoHead.sWidth;
    //Read info height
    if (fread(&bInfoHead.sHeight, sizeof(bInfoHead.sHeight), 1, bmpFp) <= 0)
        goto Err;
    bInfoHead.sHeight = _htole32(bInfoHead.sHeight);
    if (height)
        *height = bInfoHead.sHeight;

    LogSysOperDebug("bmp w[%d] h[%d]\n", *width, *height);
    fclose(bmpFp);
    return 0;

Err:
    LogSysOperWarn("error!\n");
    fclose(bmpFp);
    return -1;
}/*}}}*/

}

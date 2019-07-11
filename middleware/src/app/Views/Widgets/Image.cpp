/**
* Copyright (c) 2014,广东海博电子科技股份有限公司（北京分公司）
* All rights reserved.
* 
* 文件名称：Image.cpp
* 文件标识：
* 摘 要： 此文件为Image类的实现文件；
* 
* 当前版本：1.0
* 作 者：
* Mail : 
* 完成日期：
*
* 取代版本：1.0 
* 原作者：Sun
* Mail  : echo_eric@yeah.net 
* 完成日期：2014年01月22日 18:45:23 PM CST
*/

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>   
#include <sys/stat.h>   
#include <sys/mman.h>

#include "Image.h"
#include "ViewAssertions.h"

#define IN
#define OUT
#define RESULT_ERROR -1
#define RESULT_SUCCESS 0

extern "C" {
int ind_img_draw(int depth, unsigned char* osd_buf, int osd_width, int osd_height, int osd_x, int osd_y, char *img_buf, int img_len);
int img_info_bmp(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight);
int img_info_gif(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight);
int img_info_jpg(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight);
int img_info_png(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight);
}


namespace Hippo {

Image::Image(IN unsigned char* path)
			: mPath(path)
{
	mImageLen = 0;
	mWidth = 0;
	mHeight = 0;
	imgSurface = 0;
	mMappingStartP = NULL;
	createSurface();
}

Image::~Image()
{
}


/**
* @FuncName : loadImage
* @return   : imageFd=SUCCESS , NULL=ERROR;
*             imageFd, image file descriptor, is used for image memory management with in the form of a file;
**/
int 
Image::loadImage()  
{  
	VIEW_LOG("image open [%s] \n", mPath); 
	struct stat imageStat;
	int imageFd = open((const char*)mPath, O_RDONLY);  
	if(imageFd < 0){  
		VIEW_LOG("open %s error\n", mPath);  
		return RESULT_ERROR;  
	}  
	if (stat((const char*)mPath, &imageStat) < 0){			
		VIEW_LOG(" open %s error\n", mPath);  
		return RESULT_ERROR;	
	}
	mImageLen = imageStat.st_size;
	
	mMappingStartP = (unsigned char*)mmap(NULL, mImageLen, PROT_READ, MAP_SHARED, imageFd, 0);  
	if(mMappingStartP == MAP_FAILED){  
		VIEW_LOG("mmap failed\n");  
		return RESULT_ERROR;  
	} 
	VIEW_LOG("image open [%s], mImageLen[%d], mmap mMappingStartP[%p]\n", mPath, mImageLen, mMappingStartP);
	return imageFd;
} 

/**
* @FuncName : getSize
* @width   : OUT, the image width ;
* @height  : OUT, the image height;
* @return  : 0=SUCCESS, -1=ERROR;
**/	
int 
Image::getSize(OUT int* width, OUT int* height)
{
	if (mMappingStartP == NULL)
		return RESULT_ERROR;
		
	if (mWidth == 0 || mHeight == 0){
		VIEW_LOG("get size mWidth=mHeight= 0.\n"); 
		if (mMappingStartP[0] == 'B' && mMappingStartP[1] == 'M') {
			if (img_info_bmp(mMappingStartP, mImageLen, &mWidth, &mHeight))
				return RESULT_ERROR;
		} else if (mMappingStartP[0] == 'G' && mMappingStartP[1] == 'I' && mMappingStartP[2] == 'F' && mMappingStartP[3] == '8') {
			if (img_info_gif (mMappingStartP, mImageLen, &mWidth, &mHeight))
				return RESULT_ERROR;
		} else if (mMappingStartP[0] == 0xFF && mMappingStartP[1] == 0xD8 && mMappingStartP[2] == 0xFF && (mMappingStartP[3] == 0xE0 || mMappingStartP[3] == 0xE1)) {
			if (img_info_jpg(mMappingStartP, mImageLen, &mWidth, &mHeight))
				return RESULT_ERROR;
		} else if (mMappingStartP[0] == 0x89 && mMappingStartP[1] == 0x50 && mMappingStartP[2] == 0x4E && mMappingStartP[3] == 0x47) {
			if (img_info_png(mMappingStartP, mImageLen, &mWidth, &mHeight))
				return RESULT_ERROR;
		} else {
			return RESULT_ERROR;
		}
	} 
	*width = mWidth;
	*height = mHeight;
	VIEW_LOG("get size, width[%d], height[%d]\n", *width, *height);
	return RESULT_SUCCESS;
}

/**
* @FuncName : decodeImage
* @return  :  0=SUCCESS, -1=ERROR;
**/	
int 
Image::decodeImage()
{	
	int width, height;
	unsigned char *buffer = NULL;
	VIEW_LOG("decode, imgSurface[%p].\n", imgSurface);
	
	if (mMappingStartP[0] == 0x89 && mMappingStartP[1] == 0x50 && mMappingStartP[2] == 0x4E && mMappingStartP[3] == 0x47) {
		VIEW_LOG("image is png ok.\n");
		imgSurface = cairo_image_surface_create_from_png((const char*)mPath);
	} else {
		VIEW_LOG("image is other format.\n");
		buffer = (unsigned char *)malloc(mWidth * 4 * mHeight);
		MEM_CHECK(buffer);
		memset((void *)buffer, 0, mWidth * 4 * mHeight);
		imgSurface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_ARGB32, mWidth, mHeight, mWidth * 4);
		if (0 != ind_img_draw(32, cairo_image_surface_get_data(imgSurface), mWidth, mHeight, 0 ,0, (char *)mMappingStartP, mImageLen)){
			free(buffer);
			buffer = NULL;
			VIEW_LOG("decode error.\n");
			return RESULT_ERROR;
		}
		free(buffer);
		buffer = NULL;
	}
	VIEW_LOG("decode success.\n");
	return RESULT_SUCCESS;
}

/**
* @FuncName : createSurface
* @return   : 0=SUCCESS, -1=ERROR;
**/	
int 
Image::createSurface()
{
	int imageWidth, imageHeight;
	int fd = loadImage();
	if (-1 == fd){ //mapping
		VIEW_LOG("image map error.\n");
		return RESULT_ERROR;
	}
	if (!getSize(&imageWidth, &imageHeight)){
		VIEW_LOG("===image info : imageWidth=[%d],imageHeight=[%d].\n", imageWidth, imageHeight);
		decodeImage(); //decode & create surface
		if (-1 == freeImage(fd)){
			VIEW_LOG("imgFree failed\n");  
			return RESULT_ERROR; 
		}
		VIEW_LOG("munmap success.\n");  
	}

	return RESULT_SUCCESS;
}

/**
* @FuncName: freeImage
* @imageFd : IN, image file descriptor;
* @return  :  0=SUCCESS, -1=ERROR;
**/	
int 
Image::freeImage( IN int imageFd)  
{  
	VIEW_LOG("munmap mMappingStartP[%p]\n", mMappingStartP);  
	
	if(mMappingStartP){  
		if(-1 == munmap(mMappingStartP, mImageLen)){
			VIEW_LOG("munmap failed\n");  
			return RESULT_ERROR;   
		}
		VIEW_LOG("munmap success\n");
	}
	VIEW_LOG("munmap end success mMappingStartP[%p]\n", mMappingStartP);  
    if(imageFd > 0) {  
        close(imageFd);  
        imageFd = -1;  
    }  
	VIEW_LOG("imgClose\n");  
	return RESULT_SUCCESS;
}  

} //end namespace Hippo





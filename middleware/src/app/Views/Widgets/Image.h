	/**
* Copyright (c) 2014,广东海博电子科技股份有限公司（北京分公司）
* All rights reserved.
* 
* 文件名称：Image.h
* 文件标识：
* 摘 要：此文件为Image类的声明文件；
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
#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "cairo/cairo.h"
#include "View.h"
#include "StandardScreen.h"

#define MEM_CHECK(expr)\
do{						\
	if(NULL == expr){	\
		printf("[%s,%d]", __FUNCTION__, __LINE__); \
		printf("Allocate memory failed \n");	\
	}	\
}while(0)

#ifdef __cplusplus

namespace Hippo {

class Image{
public:
	Image(unsigned char* imagePath);
	~Image();
	
	int getSize(int* width, int* height);
	
	cairo_surface_t *imgSurface; //返回解码后的 surface
protected:
	int loadImage();
	int freeImage(int imageFd);
	int decodeImage();
	
	int createSurface();
	
	int getImageLen(){ return mImageLen;}
	int setImageLen(int length){
		if(length == 0)
			return -1;
		mImageLen = length;
		return 0;
	}
	int mImageLen;  //image length
	int mWidth;
	int mHeight;
	unsigned char* mPath;          //image open path
	unsigned char* mMappingStartP; //start address of image mapping in memory
};
} // namespace Hippo
#endif // __cplusplus

#endif //__IMAGE_H__



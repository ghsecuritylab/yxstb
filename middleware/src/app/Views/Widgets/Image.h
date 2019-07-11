	/**
* Copyright (c) 2014,�㶫�������ӿƼ��ɷ����޹�˾�������ֹ�˾��
* All rights reserved.
* 
* �ļ����ƣ�Image.h
* �ļ���ʶ��
* ժ Ҫ�����ļ�ΪImage��������ļ���
* 
* ��ǰ�汾��1.0
* �� �ߣ�
* Mail : 
* ������ڣ�
*
* ȡ���汾��1.0 
* ԭ���ߣ�Sun
* Mail  : echo_eric@yeah.net 
* ������ڣ�2014��01��22�� 18:45:23 PM CST
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
	
	cairo_surface_t *imgSurface; //���ؽ����� surface
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



#ifndef __YX_CONFIGPARTITION_H__
#define __YX_CONFIGPARTITION_H__


/*yx common */
//#include "yx_type.h"

unsigned int GetChecksumCFG( void );
char* GetSTBID( void );
unsigned int GetPCBVer( void );
unsigned int GetChipVer( void );
char* GetConfigVer(void);
char* GetKernelVer(void);
char* GetRoofsVer(void);

char* GetSerialCode( char* buf );
int SetSerialCode( char * SN );
unsigned int GetBaseMacAdrFromCFG( void );
int SetBaseMacAdrFromCFG( unsigned int macAdr );
unsigned char* GetMacAdrFromCFG( char *buf );
int SetMacAdrForCFG( unsigned char* macAdr );

short GetProgressBar_X( void );
int SetProgressBar_X( short x );
short GetProgressBar_Y( void );
int SetProgressBar_Y( short x );
short GetProgressBarWidth( void );
int SetProgressBarWidth( short width );
short GetProgressBarHeight( void );
int SetProgressBarHeight( short height );
short GetProgressBarStyle( void );
int SetProgressBarStyle( short barStyle );
unsigned int GetProgressBarColor( void );
int SetProgressBarColor( unsigned int rgb );

short GetVideoFormat( void );//vedio_system
int SetVideoFormat( short format );
short GetColorMode( void );
int SetColorMode( short mode );
short GetCompressMode( void );
short SetCompressMode( short mode );
short GetLogoWidth( void );
int SetLogoWidth( short width );
short GetLogoHeight( void );
int SetLogoHeight( short height );
unsigned int GetOSMemorySize( void );
int SetOSMemorySize( unsigned int size );
int ConfigInit( void );

int GetLogoSize( void );

/*
	函数功能：获取配置区内logo图片的数据
	参数说明：
		pDest		保存获取到logo数据的区域地址指针
		bufSize		保存获取到logo数据的区域的大小
	返回值：
		int		函数成功返回logo图像数据的大小,任何错误返回-1
	函数说明：调用该函数需要自己提前分配128K存储logo图片的空间
*/
int GetLogoDataFromCFG( char *pDest, int bufSize );

#endif /* __YX_CONFIGPARTITION_H__ */


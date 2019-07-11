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
	�������ܣ���ȡ��������logoͼƬ������
	����˵����
		pDest		�����ȡ��logo���ݵ������ַָ��
		bufSize		�����ȡ��logo���ݵ�����Ĵ�С
	����ֵ��
		int		�����ɹ�����logoͼ�����ݵĴ�С,�κδ��󷵻�-1
	����˵�������øú�����Ҫ�Լ���ǰ����128K�洢logoͼƬ�Ŀռ�
*/
int GetLogoDataFromCFG( char *pDest, int bufSize );

#endif /* __YX_CONFIGPARTITION_H__ */


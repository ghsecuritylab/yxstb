/****************************************************************************************
说明:
	这些函数都是用来读取/设置nandflash配置区中头部的一些信息的。在读取的时候仅仅将其中的
     内容读取到一个全局的结构体中。在设置其内容的时候，一次性读取2k的数据到本地的buf中，
     然后再将头部的一些信息拷贝到结构中，然后就可以使用set系列函数来设置相应的值。最后执行
     saveConfigInfo()将修改的数据保存到nandflash中。这些函数适用于5531A平台

潜在的问题：
	(1)一次性读取2k的内容，则配置区有用的信息是否全部包含在其中了，比如说logo数据
	(2)在写nandflash调用的是nandwrite()和nand_eraseall()这两个函数在该平台上可能还会改动
****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/*yx common */
#include "yx_api.h"

#define CFGMTDDEV0 "/dev/mtd/3"
#define CFGMTDDEV1 "/dev/mtd/4"
#define MAXCONFIGSIZE (128*1024) 
#define MAXVERSIONLEN  128
#define OSD_OFFSET (0x1000)

#ifndef BLOCK_LEN
#define BLOCK_LEN (128*1024)
#endif

#ifndef PAGESIZE
#define PAGESIZE (128*1024)
#endif

static int cfgGetInited;
static int cfgSetInited;
static unsigned char *localBuf = NULL;
YS_CFG_INFO cfgInfo;

int BurnToFlash( char * mtdDev );

int ConfigSetInit( void )
{
	int configFd;
	int ret;
	
//	printf("%s(): entering...\n", __func__);

	memset( &cfgInfo, 0, sizeof( YS_CFG_INFO ) );
	localBuf = ( unsigned char* )malloc( MAXCONFIGSIZE );
	if( NULL == localBuf ){
		fprintf(stderr, "no memory alloc!%s(%s,%d)\n", __func__,__FILE__,__LINE__ );
		return -1;
	}
	
	memset( localBuf, 0, MAXCONFIGSIZE );
	configFd = open( CFGMTDDEV0, O_RDWR );
	if( configFd < 0 ){
		fprintf(stderr, " can't open config paritition!\n");
		return -1;
	}
	
	ret = read( configFd, localBuf, MAXCONFIGSIZE );
	if( ret != MAXCONFIGSIZE ) {
		fprintf(stderr, " read normal config pritition error,using backup!\n");
		close( configFd );
		
		configFd = open( CFGMTDDEV1, O_RDWR );
		if( configFd < 0 ){
			fprintf(stderr, " can't open config paritition!\n");
			return -1;
		}
		ret = read( configFd, localBuf, MAXCONFIGSIZE );
		if( ret != MAXCONFIGSIZE ){
			fprintf(stderr, " read backup config pritition error!\n");
			close( configFd );
			return -1;
		}
		
		memcpy( ( unsigned char* )&cfgInfo, localBuf, sizeof( YS_CFG_INFO ) );
		
		close( configFd );
		return 0; 
	}
	
	memcpy( ( unsigned char* )&cfgInfo, localBuf, sizeof( YS_CFG_INFO ) );
	
	close( configFd );
	cfgSetInited = 1;
	cfgGetInited = 0;
	
	return 0;		
}

int ConfigGetInit( void )
{
	int configFd;
	int ret;
	
	configFd = open( CFGMTDDEV0, O_RDONLY );
	if( configFd < 0 ){
		fprintf(stderr, " can't open config paritition!\n");
		return -1;
	}
	
	ret = read( configFd, ( unsigned char* )&cfgInfo, sizeof( YS_CFG_INFO ) );
	if( ret != sizeof( YS_CFG_INFO ) ) {
		fprintf(stderr, " read normal config pritition error,using backup!\n");
		close( configFd );
		
		configFd = open( CFGMTDDEV1, O_RDONLY );
		if( configFd < 0 ){
			fprintf(stderr, " can't open config paritition!\n");
			return -1;
		}
		ret = read( configFd, ( unsigned char* )&cfgInfo, sizeof( YS_CFG_INFO ) );
		if( ret != sizeof( YS_CFG_INFO ) ){
			fprintf(stderr, " read backup config pritition error!\n");
			close( configFd );
			return -1;
		}
		
		close( configFd );
		return 0; 
	}
	
	close( configFd );
	cfgGetInited = 1;
	return 0;		
}

unsigned int GetChecksumCFG( void )
{
	if ( !cfgGetInited && !cfgSetInited ) {
		if ( 0 > ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.checksum;	
}

int SetChecksumCFG( void *buffer,int len )
{	
	unsigned char *p = (unsigned char *)buffer + 4;
	unsigned int checksum = 0;
	
	if (p == NULL)
	{
		return 0;
	}
	while (len)
	{
		checksum += (unsigned int)*p;
		p++;
		len--;
	}
	cfgInfo.checksum = checksum;
	return 0;	
}



char* GetSTBID( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return NULL;
	}
	
	return cfgInfo.product_id;
}

unsigned int  GetPCBVer( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.pcb_version;	
}

unsigned int GetChipVer( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.chip_version;	
}

char* GetConfigVer(void)
{
	char *configVer;
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return NULL;
	}
	
	configVer = ( char* )malloc( MAXVERSIONLEN );
	memset( configVer, 0, MAXVERSIONLEN );
	
	
	snprintf( configVer, MAXVERSIONLEN, "%d.%d.%d"
		                            ,cfgInfo.fat[3].major_version
	                                    ,cfgInfo.fat[3].mino_version
	                                    ,cfgInfo.fat[3].svn_version );
	
	return configVer;
}

char* GetKernelVer(void)
{
	char *kernelVer;
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return NULL;
	}
	
	kernelVer = ( char* )malloc( MAXVERSIONLEN );
	memset( kernelVer, 0, MAXVERSIONLEN );
	
	snprintf( kernelVer, MAXVERSIONLEN, "%d.%d.%d"
		                            ,cfgInfo.fat[5].major_version
	                                    ,cfgInfo.fat[5].mino_version
	                                    ,cfgInfo.fat[5].svn_version );
	
	return kernelVer;	
}
char* GetRoofsVer(void)
{
	char *rootfsVer;
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return NULL;
	}
	
	rootfsVer = ( char* )malloc( MAXVERSIONLEN );
	memset( rootfsVer, 0, MAXVERSIONLEN );
	
	snprintf( rootfsVer, MAXVERSIONLEN, "%d.%d.%d"
		                            ,cfgInfo.fat[7].major_version
	                                    ,cfgInfo.fat[7].mino_version
	                                    ,cfgInfo.fat[7].svn_version );
	
	return rootfsVer;	
}

char* GetSerialCode( char *buf )
{
	char *lp_tmp = NULL;
	unsigned int l_number = 0;
	unsigned int l_max = 8160;	//32*255
	int i;
	
	if (( !cfgGetInited) &&(!cfgSetInited) ){
		if ( ConfigGetInit() )
			return NULL;
	}
	
	//由于以前配置区规划的是16位串号，目前使用的串号都是17位的。导致原来放串号的地方
	//搁不下了,所以我们将串号放在cfg的保留区,该保留区的大小事32字节
	lp_tmp = cfgInfo.reserve2;
	for( i = 0; i < 32; i++ ){
		l_number = l_number + ( unsigned int )(*lp_tmp);
		lp_tmp++;
	}
	
	//判断盒子里面是否有串号,前提只要flash这个地方有东西,就认为有串号
	if( 0 == l_number || l_max == l_number ){
		return NULL;
	} else {
		if( NULL != buf ){
			memcpy( buf, cfgInfo.reserve2, 17 ); 
		}
		
		return cfgInfo.reserve2;
	}
}

int SetSerialCode( char * SN )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	memcpy( cfgInfo.product_serial_code, SN, 16 );
	memcpy( cfgInfo.reserve2, SN, 17 );	
	return 0;
}

unsigned int GetBaseMacAdrFromCFG( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.net_mac_base_addr;
}

int SetBaseMacAdrFromCFG( unsigned int macAdr )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.net_mac_base_addr = macAdr;
	
	return 0;	
}

unsigned char* GetMacAdrFromCFG( char* buf )
{
	char *lp_tmp = NULL;
	unsigned int l_number = 0;
	unsigned int l_max = 1530;	//6*255
	int i;
	
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return NULL;
	}
	
	lp_tmp = cfgInfo.net_mac_addr;
	for( i = 0; i < 6; i++ ){
		l_number = l_number + ( unsigned int )(*lp_tmp);
		lp_tmp++;
	}	 
	
	//判断盒子里面是否有MAC,前提只要flash这个地方有东西,就认为有串号
	if( 0 == l_number || l_max == l_number ){
		return NULL;
	} else {
		if( NULL != buf ){
			memcpy( buf, cfgInfo.net_mac_addr, 6 );
		}
		
		return cfgInfo.net_mac_addr;
	}	
	
	
}

int SetMacAdrForCFG( unsigned char* macAdr )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	memcpy( cfgInfo.net_mac_addr, macAdr, 8 );
	
	return 0;
}

unsigned int GetOSMemorySize( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osmem_size;	
}

int SetOSMemorySize( unsigned int size )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osmem_size = size;
	
	return 0;
}

short GetVideoFormat( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.video_system;
}

int SetVideoFormat( short format )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osd_info.video_system = format;
	
	return 0;	
}

short GetLogoWidth( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.width;
}

int SetLogoWidth( short width )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osd_info.width = width;
	
	return 0;	
}

short GetLogoHeight( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.height;
}

int SetLogoHeight( short height )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osd_info.height = height;
	
	return 0;	
}

int GetLogoSize( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.osd_size;	
}

int GetLogoDataFromCFG( char *pDest, int bufSize )
{	
	int l_logoSize = -1;
	int l_offset = OSD_OFFSET;

	if( ( NULL == pDest ) || ( bufSize < MAXCONFIGSIZE ) ){
		return -1;
	}
	
	//此处保证localbuf中存在数据
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
		
	l_logoSize = GetLogoSize();
	if( l_logoSize < 0 ){
		return -1;
	}

	memcpy( pDest, localBuf + l_offset, l_logoSize );
	
	if(localBuf != NULL)
	   free(localBuf);
	localBuf = NULL;
	
	cfgSetInited = 0;
	
	return l_logoSize;
	
}

short GetColorMode( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.color_mode;
}

int SetColorMode( short mode )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osd_info.color_mode = mode;
	
	return 0;	
}

short GetCompressMode( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.compress_mode;
}

short SetCompressMode( short mode )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osd_info.compress_mode = mode;
	
	return 0;
}

short GetProgressBar_X( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.progress_bar_x;	
}

int SetProgressBar_X( short x )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osd_info.progress_bar_x = x;
	
	return 0;		
}

short GetProgressBar_Y( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.progress_bar_y;		
}

int SetProgressBar_Y( short y )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osd_info.progress_bar_y = y;
	
	return 0;	
}

short GetProgressBarWidth( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.progress_bar_width;	
}

int SetProgressBarWidth( short width )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osd_info.progress_bar_width = width;
	
	return 0;	
}

short GetProgressBarHeight( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.progress_bar_height;
}

int SetProgressBarHeight( short height )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osd_info.progress_bar_height = height;
	
	return 0;	
}

short GetProgressBarStyle( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.progress_bar_style;	
}

int SetProgressBarStyle( short barStyle )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osd_info.progress_bar_style = barStyle;
	
	return 0;	
}

unsigned int GetProgressBarColor( void )
{
	if ( !cfgGetInited ) {
		if ( ConfigGetInit() )
			return -1;
	}
	
	return cfgInfo.osd_info.progress_bar_color;
}

int SetProgressBarColor( unsigned int rgb )
{
	if ( !cfgSetInited ) {
		if ( ConfigSetInit() )
			return -1;
	}
	
	cfgInfo.osd_info.progress_bar_color = rgb;
	
	return 0;
}

static int GetChecksum( void )
{
	//we need to calculate checksum of hw part and partition part respecly
	unsigned char *p = (unsigned char *)localBuf;
	unsigned int checksum = 0;
	int i;
	YS_CFG_INFO *pCFGInfo = ( YS_CFG_INFO * )localBuf;
	pCFGInfo->checksum = 0;
	
	if (p == NULL){
		return -1;
	}
	for( i = 0; i < 128; i++ ){
		checksum += *p;
		p++;
	}
	
	pCFGInfo->checksum = checksum;
	pCFGInfo->checksumPatition = 0;
	p = (unsigned char *)&( pCFGInfo->partition_num );
	
	for( checksum = 0, i = 0; i < 368; i++ ){
		checksum += *p;
		p++;
	}
	
	pCFGInfo->checksumPatition = checksum;
	
	return 0;
}

int saveConfigInfo( void )
{
	if(localBuf!=NULL){
	SetChecksumCFG( ( unsigned char* )&cfgInfo, 124 );
	memcpy( localBuf, ( unsigned char* )&cfgInfo, sizeof( YS_CFG_INFO ));
	
	GetChecksum();
	BurnToFlash( CFGMTDDEV0 );
	BurnToFlash( CFGMTDDEV1 );
    	}
    
	if(localBuf!=NULL)
	   free( localBuf );
	localBuf = NULL;
	
	memset( &cfgInfo, 0, sizeof( YS_CFG_INFO ) );
	cfgSetInited = 0;
    	cfgGetInited = 0;
	return 0;
}

int BurnToFlash( char * mtdDev )
{
	int blocksize, oobsize, pagesize;
	
	nand_init( "p\0", mtdDev );
	nand_getinfo(&pagesize, &oobsize, &blocksize);
	fprintf(stderr, "====Erase %s %d:%d...\n\n", mtdDev,blocksize,oobsize);
	nand_eraseall( mtdDev, NULL, BLOCK_LEN );
	nand_write( localBuf, BLOCK_LEN, 0 );
	nandUnlock();
	
	return 0;
}

/*
int main( int argc, char *argv[] )
{
	//使用hexdump查看配置区的时候要注意其字节的顺序
	//比如说在checksum的位置的值是 90 4e 00 00，则读出的值是20112(0x4e90)
	unsigned int checksum;
	unsigned int chipVer;
	unsigned int macBase;
	unsigned char *netMacAdr;
	
	checksum = GetChecksumCFG();
	chipVer = GetChipVer();
	macBase = GetBaseMacAdrFromCFG();
	netMacAdr = GetMacAdrFromCFG();
	
	printf( "1,checksum = %d, chipVer = %x, macBase = %x,  netMacAdr = %s\n", checksum,checksum,macBase,netMacAdr);

	SetBaseMacAdrFromCFG( 0X90000 );
	
		
	saveConfigInfo();
	
	macBase = GetBaseMacAdrFromCFG();
	printf( "2,checksum = %d, chipVer = %x, macBase = %x,  netMacAdr = %s\n", checksum,checksum,macBase,netMacAdr);
		
	return 0;
}
*/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "yx_gpio.h"
#include "hi_type.h" 
#include "hi_unf_ecs.h"
#include "linux/reboot.h"

#define DEFAULT_MD_LEN 4
#define PAGE_SIZE 0x1000
#define PAGE_SIZE_MASK 0xfffff000

typedef struct tag_MMAP_Node
{
	unsigned int Start_P;
	unsigned int Start_V;
	unsigned int length;
    unsigned int refcount;  /* map后的空间段的引用计数 */
	struct tag_MMAP_Node * next;
}MMAP_Node_t;

MMAP_Node_t * pMMAPNode = NULL;


static int fd = -1;
static const char dev[]="/dev/mem";

static int sg_isGpioInit = 0;

static int hisi_gpioOpen_yx( void );
static int hisi_gpioClose_yx( void );
static int hisi_gpioConfig_yx( int which );
static int  hisi_gpioRead_yx( int whichGPIO );
static int hisi_gpioWrite_yx( int whichGPIO, int value );


static int hisi_gpioOpen_yx( void )
{
	HI_S32 l_ret = HI_FAILURE;
	
	l_ret = HI_UNF_GPIO_Open();
	if( HI_SUCCESS != l_ret ){
		fprintf( stderr, "[%s,%d]:distroy higo error\n", __func__,__LINE__ );
		return l_ret;	
	}
	
	return l_ret;
}

static int hisi_gpioClose_yx( void )
{
	HI_S32 l_ret = HI_FAILURE;
	
	
	l_ret = HI_UNF_GPIO_Close();
	if( HI_SUCCESS != l_ret ){
		fprintf( stderr, "[%s,%d]:distroy higo error\n", __func__,__LINE__ );
		return l_ret;	
	}
	
	return l_ret;	 
}

//配置所用GPIO的方向,至于复合管脚的冲突问题,需要调用者考虑
static int hisi_gpioConfig_yx( int which )
{
	HI_S32 l_ret = HI_FAILURE;
	
	
	//HI_UNF_GPIO_SetDirBit()第一个参数的含义是 第几组*8＋该组的第几bit，范围是0－71(9组)
	//比如 GPIO0_0 就是 0*8+0=0，GPIO2_3 就是 2*8+3=19
	//第二个参数为HI_FALSE表示配置为输出,为HI_TRUE表示配置为输入
	switch( which ){
		case GPIO_FOR_MUTE:
			l_ret = HI_UNF_GPIO_SetDirBit( MUTE_PIN, HI_FALSE );
			break;
		case GPIO_FOR_NET_LAMP:	
			l_ret = HI_UNF_GPIO_SetDirBit( NET_LAMP_PIN0, HI_FALSE );
			l_ret = HI_UNF_GPIO_SetDirBit( NET_LAMP_PIN1, HI_FALSE );
			break;
		case GPIO_FOR_WIFI_LAMP:	
			l_ret = HI_UNF_GPIO_SetDirBit( WIFI_LAMP_PIN0, HI_FALSE );
			l_ret = HI_UNF_GPIO_SetDirBit( WIFI_LAMP_PIN1, HI_FALSE );
			break;	
		case GPIO_FOR_POWER_LAMP:
			l_ret = HI_UNF_GPIO_SetDirBit( POWER_LAMP_PIN, HI_FALSE );
			break;
		case GPIO_FOR_IR_LAMP:
			l_ret = HI_UNF_GPIO_SetDirBit( IR_LAMP_PIN, HI_FALSE );
			break;
		case GPIO_FOR_VER:
			l_ret = HI_UNF_GPIO_SetDirBit( VER_PIN1, HI_TRUE );
			l_ret = HI_UNF_GPIO_SetDirBit( VER_PIN2, HI_TRUE );
			l_ret = HI_UNF_GPIO_SetDirBit( VER_PIN3, HI_TRUE );
			l_ret = HI_UNF_GPIO_SetDirBit( VER_PIN4, HI_TRUE );
			l_ret = HI_UNF_GPIO_SetDirBit( VER_PIN5, HI_TRUE );
			l_ret = HI_UNF_GPIO_SetDirBit( VER_PIN6, HI_TRUE );
			break;
		case GPIO_FOR_RESET:
			l_ret = HI_UNF_GPIO_SetDirBit( GPIO_RESET_PIN, HI_FALSE );
			break;
		default:
			break;
	}
	
	if( HI_SUCCESS != l_ret ){
		fprintf( stderr, "[%s,%d]:HI_UNF_GPIO_SetDirBit error\n", __func__,__LINE__ );	
	}
	
	return l_ret;	
}

int hisi_gpioInit_yx( void )
{
	HI_S32 l_ret = HI_FAILURE;
		
	if( 1 == sg_isGpioInit ){
		fprintf( stderr, "[%s,%d]:gpio is init yet!\n", __func__,__LINE__ );
		return HI_SUCCESS;
	}
	
	l_ret = hisi_gpioOpen_yx();
	
	//配置电源指示灯
	l_ret = hisi_gpioConfig_yx( GPIO_FOR_POWER_LAMP );
	
	//配置网络指示灯
	l_ret = hisi_gpioConfig_yx( GPIO_FOR_NET_LAMP );
	
	//配置WIFI网络指示灯
	l_ret = hisi_gpioConfig_yx( GPIO_FOR_WIFI_LAMP );
	
	//配置红外指示灯
	l_ret = hisi_gpioConfig_yx( GPIO_FOR_IR_LAMP );
	
	//配置静音功能
	l_ret = hisi_gpioConfig_yx( GPIO_FOR_MUTE );

	//配置读取硬件版本号
	l_ret = hisi_gpioConfig_yx( GPIO_FOR_VER );

	
	if( HI_SUCCESS != l_ret ){
		fprintf( stderr, "[%s,%d]:hisi_gpioInit_yx error\n", __func__,__LINE__ );
		return l_ret;	
	}
	
	sg_isGpioInit = 1;

	//置初始状态
	hisi_powerLamp_yx( PILOT_LAMP_GREEN );
	hisi_remoteLamp_yx( 0 );
	
	return l_ret;
}

int hisi_gpioQuit_yx( void )
{
	HI_S32 l_ret = HI_FAILURE;

	if( 0 == sg_isGpioInit ){
		fprintf( stderr, "[%s,%d]:gpio is not open!\n", __func__,__LINE__ );
		return l_ret;	
	}
	
	//置结束状态
	hisi_powerLamp_yx( PILOT_LAMP_GREEN );
	hisi_remoteLamp_yx( 0 );
	hisi_netLamp_yx( PILOT_LAMP_OFF );
		
	l_ret = hisi_gpioClose_yx();
	if( HI_SUCCESS != l_ret ){
		fprintf( stderr, "[%s,%d]:hisi_gpioQuit_yx error\n", __func__,__LINE__ );
		return l_ret;	
	}
	
	
	
	sg_isGpioInit = 0;	
	return l_ret;
}

static int  hisi_gpioRead_yx( int whichGPIO )
{
	HI_S32 l_ret = HI_FAILURE;
	HI_BOOL l_value = HI_FALSE;
		
	if( 0 == sg_isGpioInit ){
		fprintf( stderr, "[%s,%d]:you need init gpio frist!\n", __func__,__LINE__ );
		return l_ret;
	}

	l_ret = HI_UNF_GPIO_ReadBit( whichGPIO, &l_value );
	if( HI_SUCCESS != l_ret ){
		fprintf( stderr, "[%s,%d]:HI_UNF_GPIO_ReadBit error\n", __func__,__LINE__ );
		return -1;	
	}

	if( l_value ){
		return 1;
	} else{
		return 0;
	}	
}

int hisi_getBOMVer_yx( void )
{
	int l_ret = -1;
	int l_value = 0;
	
	l_ret = hisi_gpioRead_yx( VER_PIN1 );
	if( 0 == l_ret || 1 == l_ret ){
		l_value |= l_ret << 0;
		l_ret = -1;
	} else {
		fprintf( stderr, "[%s,%d]:hisi_gpioRead_yx error\n", __func__,__LINE__ );
		return -1;		
	}
	
	l_ret = hisi_gpioRead_yx( VER_PIN2 );
	if( 0 == l_ret || 1 == l_ret ){
		l_value |= l_ret << 1;
		l_ret = -1;
	} else {
		fprintf( stderr, "[%s,%d]:hisi_gpioRead_yx error\n", __func__,__LINE__ );
		return -1;		
	}
	
	l_ret = hisi_gpioRead_yx( VER_PIN3 );
	if( 0 == l_ret || 1 == l_ret ){
		l_value |= l_ret << 2;
		l_ret = -1;
	} else {
		fprintf( stderr, "[%s,%d]:hisi_gpioRead_yx error\n", __func__,__LINE__ );
		return -1;		
	}
	
	return l_value;
}

int hisi_getPCBVer_yx( void )
{
	int l_ret = -1;
	int l_value = 0;
	
	l_ret = hisi_gpioRead_yx( VER_PIN4 );
	if( 0 == l_ret || 1 == l_ret ){
		l_value |= l_ret << 0;
		l_ret = -1;
	} else {
		fprintf( stderr, "[%s,%d]:hisi_gpioRead_yx error\n", __func__,__LINE__ );
		return -1;		
	}
	
	l_ret = hisi_gpioRead_yx( VER_PIN5 );
	if( 0 == l_ret || 1 == l_ret ){
		l_value |= l_ret << 1;
		l_ret = -1;
	} else {
		fprintf( stderr, "[%s,%d]:hisi_gpioRead_yx error\n", __func__,__LINE__ );
		return -1;		
	}
	
	l_ret = hisi_gpioRead_yx( VER_PIN6 );
	if( 0 == l_ret || 1 == l_ret ){
		l_value |= l_ret << 2;
		l_ret = -1;
	} else {
		fprintf( stderr, "[%s,%d]:hisi_gpioRead_yx error\n", __func__,__LINE__ );
		return -1;		
	}
	
	return l_value;
}

//'1'表示高电平 '0'表示低电平
static int hisi_gpioWrite_yx( int whichGPIO, int value )
{
	HI_S32 l_ret = HI_FAILURE;
	
	if( 0 == sg_isGpioInit ){
		fprintf( stderr, "[%s,%d]:you need init gpio frist!\n", __func__,__LINE__ );
		return l_ret;
	}
	
	if( 1 == value ){
		l_ret = HI_UNF_GPIO_WriteBit( whichGPIO, HI_TRUE );
	} else if( 0 == value ){
		l_ret = HI_UNF_GPIO_WriteBit( whichGPIO, HI_FALSE );
	}
	if( HI_SUCCESS != l_ret ){
		fprintf( stderr, "[%s,%d]:HI_UNF_GPIO_WriteBit error\n", __func__,__LINE__ );
		return l_ret;	
	}
		
	return l_ret;
}

//'1'使能静音,‘0’是关闭静音
int hisi_audioMute_yx( int onoff )
{
	hisi_gpioWrite_yx( MUTE_PIN, onoff );
	
	return 0;
} 

int hisi_netLamp_yx( int color )
{
	switch( color ){
		case PILOT_LAMP_RED:
			hisi_gpioWrite_yx( NET_LAMP_PIN0, 0 );
			hisi_gpioWrite_yx( NET_LAMP_PIN1, 1 );
			//wifi net
			//hisi_gpioWrite_yx( WIFI_LAMP_PIN0, 0 );
			//hisi_gpioWrite_yx( WIFI_LAMP_PIN1, 1 );
			break;
		case PILOT_LAMP_GREEN:
			hisi_gpioWrite_yx( NET_LAMP_PIN0, 1 );
			hisi_gpioWrite_yx( NET_LAMP_PIN1, 0 );
			//wifi net
			//hisi_gpioWrite_yx( WIFI_LAMP_PIN0, 1 );
			//hisi_gpioWrite_yx( WIFI_LAMP_PIN1, 0 );
			break;
		case PILOT_LAMP_OFF:
			hisi_gpioWrite_yx( NET_LAMP_PIN0, 0 );
			hisi_gpioWrite_yx( NET_LAMP_PIN1, 0 );
			//wifi net
			//hisi_gpioWrite_yx( WIFI_LAMP_PIN0, 0 );
			//hisi_gpioWrite_yx( WIFI_LAMP_PIN1, 0 );
			
			break;
		default:
			break;
	}
		
	return 0;
} 

int hisi_Wifi_netLamp_yx( int color )
{
	switch( color ){
		case PILOT_LAMP_RED:
			hisi_gpioWrite_yx( WIFI_LAMP_PIN0, 0 );
			hisi_gpioWrite_yx( WIFI_LAMP_PIN1, 1 );
			break;
		case PILOT_LAMP_GREEN:
			hisi_gpioWrite_yx( WIFI_LAMP_PIN0, 1 );
			hisi_gpioWrite_yx( WIFI_LAMP_PIN1, 0 );
			break;
		case PILOT_LAMP_OFF:
			hisi_gpioWrite_yx( WIFI_LAMP_PIN0, 0 );
			hisi_gpioWrite_yx( WIFI_LAMP_PIN1, 0 );
			
			break;
		default:
			break;
	}
		
	return 0;
} 

int hisi_powerLamp_yx( int color )
{

	switch( color ){
		case PILOT_LAMP_RED:
			hisi_gpioWrite_yx( POWER_LAMP_PIN, 1 );
			break;
		case PILOT_LAMP_GREEN:
			hisi_gpioWrite_yx( POWER_LAMP_PIN, 0 );
			break;
		default:
			break;
	}
	
	return 0;
}  

int hisi_remoteLamp_yx( int onoff )
{
	hisi_gpioWrite_yx( IR_LAMP_PIN, onoff );

	return 0;
}

static int hisi_reset_yx( void )
{	
	system("himm 0x101e005c 0xe4020010");
	usleep(1000*1000);

	//配置重启功能的gpio
	hisi_gpioConfig_yx( GPIO_FOR_RESET );
	
	//复位引脚200ms的低电平
	hisi_gpioWrite_yx( GPIO_RESET_PIN, 0 );
	
	usleep(1000*200);
	
	//重新拉高
	hisi_gpioWrite_yx( GPIO_RESET_PIN, 1 );
	
	return 0;
}

//=======================系统软复位==================================
void* memmap_yx(unsigned int phy_addr, unsigned int size)
{
	unsigned int phy_addr_in_page;
	unsigned int page_diff;

	unsigned int size_in_page;

	MMAP_Node_t * pTmp;
	MMAP_Node_t * pNew;
	
	void *addr=NULL;

	if(size == 0)
	{
		printf("memmap_yx():size can't be zero!\n");
		return NULL;
	}


	/* not mmaped yet */
	if(fd < 0)
	{
		/* dev not opened yet, so open it */
		fd = open (dev, O_RDWR | O_SYNC);
		if (fd < 0)
		{
			printf("memmap_yx():open %s error!\n", dev);
			return NULL;
		}
	}

	/* addr align in page_size(4K) */
	phy_addr_in_page = phy_addr & PAGE_SIZE_MASK;
	page_diff = phy_addr - phy_addr_in_page;

	/* size in page_size */
	size_in_page =((size + page_diff - 1) & PAGE_SIZE_MASK) + PAGE_SIZE;

	addr = mmap ((void *)0, size_in_page, PROT_READ|PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);
	if (addr == MAP_FAILED)
	{
		printf("memmap_yx():mmap @ 0x%x error!\n", phy_addr_in_page);
		return NULL;
	}



	return (void *)(addr+page_diff);

}

static int  change_vou_reg(int phy_addr,int uValue)
{
	int* pMem  = NULL;
	pMem = memmap_yx(phy_addr, DEFAULT_MD_LEN);
	
	*(int*)pMem = uValue;

	return 0;
}

//==========================暂时放两个空函数，以后要移除=============
//=================================================================

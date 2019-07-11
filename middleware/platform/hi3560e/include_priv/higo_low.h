
#ifndef _HIGO_LOW_H_
#define _HIGO_LOW_H_


#include "hi_go.h"
/** 像素格式 */
typedef enum
{
	YX_HIGO_PF_CLUT8 = 0,  /**< 调色板 */
	YX_HIGO_PF_CLUT1,
	YX_HIGO_PF_CLUT4,
	YX_HIGO_PF_4444,       /**< 一个像素占16bit，ARGB每分量占4bit，按地址由高至低排列 */
	YX_HIGO_PF_0444,       /**< 一个像素占16bit，ARGB每分量占4bit，按地址由高至低排列, A分量不起作用 */

	YX_HIGO_PF_1555,       /**< 一个像素占16bit，RGB每分量占5bit，A分量1bit，按地址由高至低排列 */
	YX_HIGO_PF_0555,       /**< 一个像素占16bit，RGB每分量占5bit，A分量1bit，按地址由高至低排列, A分量不起作用 */
	YX_HIGO_PF_565,        /**< 一个像素占16bit，RGB每分量分别占5bit、6bit和5bit，按地址由高至低排列 */
	YX_HIGO_PF_8565,       /**< 一个像素占24bit，ARGB每分量分别占8bit、5bit、6bit和5bit，按地址由高至低排列 */
	YX_HIGO_PF_8888,       /**< 一个像素占32bit，ARGB每分量占8bit，按地址由高至低排列 */
	YX_HIGO_PF_0888,       /**< 一个像素占32bit，ARGB每分量占8bit，按地址由高至低排列，A分量不起作用 */

	YX_HIGO_PF_YUV400,     /**< 海思定义的semi-planar YUV 400格式 */    
	YX_HIGO_PF_YUV420,     /**< 海思定义的semi-planar YUV 420格式 */
	YX_HIGO_PF_YUV422,     /**< 海思定义的semi-planar YUV 422格式  水平采样格式*/
	YX_HIGO_PF_YUV422_V,   /**< 海思定义的semi-planar YUV 422格式  垂直采样格式*/    
	YX_HIGO_PF_YUV444,     /**< 海思定义的semi-planar YUV 444格式 */

	YX_HIGO_PF_A1, 
	YX_HIGO_PF_A8,
	YX_HIGO_PF_BUTT
} YX_HIGO_PF_E;

typedef enum
{
	YX_HIGO_COMPOPT_NONE = 0, /**< 不使用混合，即copy */

	YX_HIGO_COMPOPT_SRCOVER,  /**< Porter/Duff SrcOver混合操作 */
	YX_HIGO_COMPOPT_AKS,      /**< 假设目标surface为不透明,简单alpha混合，结果保留源alpha */
	YX_HIGO_COMPOPT_AKD,      /**< 假设目标surface为不透明,简单alpha混合，结果保留目标alpha */

	YX_HIGO_COMPOPT_BUTT
} YX_HIGO_COMPOPT_E;

typedef enum
{
	YX_HIGO_ZORDER_MOVETOP = 0,  /*移到最顶部*/  
	YX_HIGO_ZORDER_MOVEUP,	  /*向上移*/  
	YX_HIGO_ZORDER_MOVEBOTTOM,	  /*移到最底部*/  
	YX_HIGO_ZORDER_MOVEDOWN,     /*向下移*/  
	YX_HIGO_ZORDER_BUTT,  
} YX_HIGO_ZORDER_E;

typedef struct higo_handle_yx{
	int		isHigoInit;
	HI_HANDLE	hBrowser;
	HI_HANDLE	hBrowserDeskTop;
	HI_HANDLE	hBrowserMainSur;
	HI_HANDLE	hOsd;
	HI_HANDLE	hOsdDeskTop; 
	HI_HANDLE	hOsdMainSur;
	HI_HANDLE	hTextHandle;
}HIGOHANDLEYX, *HIGOHANDLEYX_T;

typedef enum {
	BROWSERHANDEL = 0,
	BROWSERDESKTOP,
	BROWSERMAINSUR,
	OSDHANDEL,
	OSDDESKTOP,
	OSDMAINSUR
}YX_HANDELCLASS;

/* 建立一个链表来管理桌面上创建的窗口 */
typedef struct osd_window_link{
	HI_HANDLE hWindow;
	struct osd_window_link* pNext;
}OSDWINLINK, *OSDWINLINK_T;

/*
	函数功能：初始化higo,并且创建字体句柄
	参数说明：
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：调用该函数进行HIGO相关的初始化,包括了Bliter模块,解码器模块,显示设备模块,
	内存表面模块的初始化
*/
int higo_init_yx( void );

/*
	函数功能：去初始化higo,并且销毁字体句柄
	参数说明：
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：调用该函数进行HIGO相关的去初始化,包括了Bliter模块,解码器模块,显示设备模块,
	内存表面模块的去初始化
*/
int higo_quit_yx( void );

/*
	函数功能：创建osd图形层
	参数说明：
		width		osd层的宽度
		height		osd层的高度
		YX_HIGO_PF_E	osd层的像素格式
		flip_count	用做Flip的内存块数
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：此函数中已经将图层的刷新模式写死为抗闪烁,双缓冲(此处需要和海思沟通,可能会改)
	另外在该函数中创建的桌面的颜色为0xFF00FF00
*/
int higo_osdCreate_yx( int width, int height, int offset_x, int offset_y, YX_HIGO_PF_E color_format, int flip_count );

/*
	函数功能：销毁osd图形层
	参数说明：
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：调用此函数的时候,应该保证在该层创建的窗口和与该层相关的内存表层都已经销毁
*/
int higo_osdDistory_yx( void );

/*
	函数功能：获取osd图形层的宽
	参数说明：
	返回值：
		int		函数成功返回osd图形层的宽，失败返回 ‘-1’
	函数说明：
*/
int higo_getOsdWidth_yx( void );

/*
	函数功能：获取osd图形层高
	参数说明：
	返回值：
		int		函数成功返回osd图形层的高，失败返回 ‘-1’
	函数说明：
*/
int higo_getOsdHeight_yx( void );
/*
	函数功能：创建浏览器图形层
	参数说明：
		width		浏览器层的宽度
		height		浏览器层的高度
		YX_HIGO_PF_E	浏览器层的像素格式
		flip_count	用做Flip的内存块数
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：此函数中已经将图层的刷新模式写死为抗闪烁,双缓冲(此处需要和海思沟通,可能会改)
	另外在该函数中创建的桌面的颜色为0xFF00FF00
*/
int higo_browserCreate_yx( int width, int height, int offset_x, int offset_y, YX_HIGO_PF_E color_format, int flip_count );

/*
	函数功能：销毁osd图形层
	参数说明：
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：调用此函数的时候,应该保证在该层创建的窗口和与该层相关的内存表层都已经销毁
*/
int higo_browserDistory_yx( void );

/*
	函数功能：获取指定surface的内存首地址
	参数说明：
		surface		需要获取surface的句柄
	返回值：
		HI_VOID*	函数成功返回指定surface的内存首地址，失败返回 NULL
	函数说明：调用此函数时应该确保该surface句柄是真实有效的，且不能对同一surface重复锁定
*/
HI_VOID* higo_getSurfaceBuf_yx( HI_HANDLE surface );

/*
	函数功能：设置图层的透明度
	参数说明：
		layer		图层的句柄
		Alpha		图层的透明度(0-255)
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：接口中的透明度是32位的，会在该接口中转换为8位
*/
int higo_setLayerAlpha_yx( HI_HANDLE layer, HI_S32 Alpha );

/*
	函数功能：设置图层在端显示设备上的起始坐标
	参数说明：
		layer		图层的句柄
		x		图层在终端显示设备上的起始横坐标
		y		图层在终端显示设备上的起始纵坐标
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：
*/
int higo_setLayerCoords_yx( HI_HANDLE layer, HI_S32 x, HI_S32 y );

/*
	函数功能：设置图层在端显示设备上的显示状态
	参数说明：
		layer		图层的句柄
		flag		‘1’表示显示该图层，‘0’表示隐藏该图层
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：立即生效,无需刷新
*/
int higo_layerViewStatus_yx( HI_HANDLE layer, int flag );

/*
	函数功能：将主surface上的内容刷新到终端显示设备上
	参数说明：
		layer		需要刷新图形层的句柄
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：
*/
int higo_refresh_yx( HI_HANDLE layer );

/*
	函数功能：获取HIGOHANDLEYX全局结构体中指定元素的数据值
	参数说明：
		handle		图层相关的句柄
	返回值：
		int		函数成功返回 全局结构体中指定元素的数据值，失败返回 -1
	函数说明：能够获取的句柄只包括已经在HIGOHANDLEYX中声明的句柄
*/
unsigned int higo_gethandel_yx( int handle );

/*
	函数功能：在OSD层的桌面上创建一个窗口
	参数说明：
		h_window	【输出】即将创建窗口的句柄
		x		即将创建窗口在中断显示屏幕上的横坐标
		y		即将创建窗口在中断显示屏幕上的纵坐标
		width		即将创建窗口的宽
		height		即将创建窗口的高
		color		即将创建窗口的颜色,颜色格式8888
		opacity		即将创建窗口的透明度（0-255）
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：需要和higo_osdDistoryWindow_yx配套使用释放资源
*/
int higo_osdAddWindow_yx( HI_HANDLE* h_window, int x, int y, int width, int height, int color, int opacity );

/*
	函数功能：销毁一个窗口
	参数说明：
		h_window	即将销毁窗口的句柄

	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：
*/
int higo_osdDistoryWindow_yx( HI_HANDLE h_window );

/*
	函数功能：往指定的窗口中输出字符串
	参数说明：
		h_window	字符串的输出窗口
		x		输出字符串在窗口中的横坐标
		y		输出字符串在窗口中的纵坐标
		width		输出字符串在窗口中的宽
		height		输出字符串在窗口中的高
		bk_color	输出字符串的背景颜色,颜色格式8888
		color		输出字符串的颜色,颜色格式8888
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：这函数主要是提供给工厂测试程序显示界面使用
*/
int higo_textoutWindow_yx( HI_HANDLE h_window, char* str, int x, int y, int bk_color, int color );

/*
	函数功能：往指定的窗口中填充矩形框
	参数说明：
		h_window	矩形框的输出窗口
		x		矩形框在窗口中的横坐标
		y		矩形框在窗口中的纵坐标
		width		矩形框在窗口中的宽
		height		矩形框在窗口中的高
		color		矩形框填充的颜色,颜色格式8888
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：这函数主要是提供给工厂测试程序显示界面使用
*/
int higi_fillRectWindow_yx( HI_HANDLE h_window, int x, int y, int width, int height, int color );

/*
	函数功能：在指定的surface上填充特定的矩形
	参数说明：
		surface		填充矩形的surface
		x		矩形框在surface上的横坐标
		y		矩形框在surface上的纵坐标
		width		矩形框在surface上的宽
		height		矩形框在surface上的高
		color		矩形框的颜色,颜色格式8888
		operators	画图时的操作算子
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：参数中的operators请参考海思文档上所支持的操作算子
*/
int higo_fillRect_yx( HI_HANDLE surface, int x, int y, int width, int height, int color, YX_HIGO_COMPOPT_E operators );

/*
	函数功能：在指定的surface上输出特定的字符串
	参数说明：
		surface		输出字符串的surface
		x		输出字符串在窗口中的横坐标
		y		输出字符串在窗口中的纵坐标
		width		输出字符串在窗口中的宽
		height		输出字符串在窗口中的高
		bk_color	输出字符串的背景颜色,颜色格式8888
		color		输出字符串的颜色,颜色格式8888
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：
*/
int higo_textout_yx( HI_HANDLE surface, char *str, int x, int y, int bk_color, int color );

/*
	函数功能：调整osd层的层序
	参数说明：
		flag		‘1’表示将OSD层调整至最上层,'0'表示将OSD层本身的位置向下挪动一层

	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：海思包括视频层,图层0(osd层),图层1(browser层),硬件鼠标层
*/
int higo_osdUpDown_yx( int flag );

/*
	函数功能：设置指定窗口的不透明度
	参数说明：
		deskTop		要设置窗口依附的桌面句柄
		h_window	要设置窗口句柄
		opacity		透明度（0-255）
	返回值：
		int		函数成功返回 HI_SUCCESS，失败返回 HI_FAILURE
	函数说明：
*/
int higo_setWindowOpacity_yx( HI_HANDLE deskTop,  HI_HANDLE h_window, int opacity );

//提供一个显示隐藏某个矩形的接口(台标需求)


//提供一个在图层创建窗口的接口
int fill_rectangel_window( HI_HANDLE h_window, int x, int y, int width, int height, int color );


//解码器的使用
//要考虑到以下几个应用
//                     a、数据的来源，可能来自文件，也可能来自内存
//                     b、解码后的数据可能并不需要显示，只需要保存
//                     c、解码后的数据需要在指定的位置显示出来
//                     d、需要考虑动态gif的解码
//                     e、需要向上兼容特效的处理

typedef union {
	//输入源为内存块时需要的信息
	struct{
		char* pAddr;      /**< 内存指针地址*/
		int Length;       /**< 长度*/
	}MemInfo;

	//图片文件名
	const char* pFileName;

} YX_HIGO_DEC_SRCINFO_U;

typedef struct picture_dec_info{
	short isSaveToFile;	//'0'表示直接显示,'1'表示存入文件
	short isDataFormFile;   //'0'表示数据来源于内存,'1'表示数据来源于文件
	short x;
	short y;
	short width;
	short height;
	char savedFilename[256];
	YX_HIGO_DEC_SRCINFO_U decodeSrcInfo;
}PICTUREDECINFO,*PICTUREDECINFO_T;
int processPIC( HI_HANDLE surface, PICTUREDECINFO_T decInfo );

//纯RGB数据的显示(供浏览器,或者osd层某些特殊的应用)
int RGBstuff( HI_HANDLE surface, const unsigned char* buf, int lenght );

//还需要重点测试higo的blit相关的操作,另外是对代码的整理

#endif /*_HIGO_LOW_H_*/


#ifndef _HIGO_APP_H_
#define _HIGO_APP_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "dpf_comm_func.h"


#define yresult   int

int higo_appFillRect_yx( int x, int y, int width, int height, int color );
int higo_appOsdRefresh_yx( void );
int higo_appBrowserRefresh_yx( void );



typedef enum{
	YGP_EFFECT_RANDOM	 						= -1, 										/*随机效果*/
	YGP_EFFECT_NONE 							= 0,							/*拷贝显示*/
	YGP_EFFECT_TURNPAGE						,						/*翻页*/
	YPG_EFFECT_ROLLPAGE						, 					/*卷轴*/
	YGP_EFFECT_VERTICALSHUTTER		,			/* 垂直百叶窗*/
	YGP_EFFECT_HORIZONTALSHUTTER	,		/* 水平百叶窗*/
	YGP_EFFECT_LEFTIN							,							/* 从左抽出*/
	YGP_EFFECT_TOPIN							, 							/* 从上抽出*/
	YGP_EFFECT_TRANSIN						, 						/* 渐进渐出*/
	YGP_EFFECT_ROTATE							,  						/*螺旋*/
	YGP_EFFECT_CENTEROUT					,  					/*中央渐出*/
	YGP_EFFECT_CENTERIN						 					/*中央渐入*/
}YGP_EFFECT_TYPE_E;


typedef enum{
	YGP_HIDE_TOP_PIC  = 0,		/*隐藏当前显示的最上层的图片*/
//	YGP_HIDE_ALL_PIC,			/*隐藏显示多张图片的图层*/
								/*2009-08-03与满熹明讨论后去掉该选项*/
	YGP_HIDE_ENTILE_LAYER	/*隐藏整个图层*/
}YGP_HIDE_TYPE_E;


/*旋转角度*/
typedef enum 
{
	YGP_REEL_ANGLE_0 		= 0 ,		//	旋转角度 0度
	YGP_REEL_ANGLE_90 	,			//	旋转角度 90 度
	YGP_REEL_ANGLE_180	,		//   旋转角度 180 度
	YGP_REEL_ANGLE_270	,		//   旋转角度 270 度
	YGP_REEL_ANGLE_BUTT  
}YGP_REEL_ANGLE_E; 


typedef enum{
	YGP_LAYER_ALPHA_BROWER = 0,	/*设置浏览器图层的alpha值*/
	YGP_LAYER_ALPHA_PIC				/*设置图形显示图层的alpha值*/
}YGP_LAYER_ALPHA_TYPE_E;




typedef enum 
{
	YGP_RECT_SOLID 	 ,		//	实心矩形
	YGP_RECT_EMPTY	 		//	空心矩形
} YGP_RECT_TYPE_E;

typedef enum{
	YGP_MOVE_LEFT,			/*屏幕左移动*/
	YGP_MOVE_RIGHT,		/*屏幕右移动*/
	YGP_MOVE_UP,
	YGP_MOVE_DOWN
}YGP_MOVE_TYPE_E;




/*图片显示的相关信息*/
typedef struct  {
	int                                            pic_w;			/*图片的实际宽度*/
	int                                           pic_h;			/*图片的实际高度*/
}YGP_PIC_INFO_S;




/*************************************************
    Function:		dpf_pic_shrink
    Description:	为指定图像生成缩略图
    Input:		
   		 Path	指定的图像的路径
			 Shrink_path	缩略图的路径
			 *w	指定缩略图的宽，底层会返回缩略图实际的宽度
			 *h	指定缩略图的高，底层会放缓缩略图实际的高度
			Color	补齐时候用的颜色
			Param	未定义

    Return:			
        DPF_SUCCESS:		成功
  	   DPF_FAILURE:		失败
    Err Code:
    	ERR_DPF_FILEPATH_NOVILID	文件路径无效
			ERR_DPF_NO_SHRINK_PIC	指定的图片没有缩略图

*************************************************/
yresult  ygp_pic_shrink (char *path, char *shrink_path, int *w, int *h, uint32 color, void *param) ;

/*************************************************
    Function:		ygp_pic_display
    Description:	在指定位置画出指定宽高的图片
    Input:		
   		path	指定的图像的路径
			x	图片的x坐标
			Y	图片的y坐标
			*W	图片的宽,底层会返回显示的实际的宽高
			*H	图片的高,底层会返回显示的实际的宽高
			mode	特效类型共11种
			Color	补齐时候用的颜色
			Param	:slide_show时候的速度,范围在1~16,越大越快,推荐值是8.

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
    	ERR_DPF_FILE_NOEXIST	指定文件不存在
			ERR_DPF_SIZE_TOOBIG	传入的尺寸太大
*************************************************/
yresult  ygp_pic_display(char *path, int x, int y, int *w, int *h, YGP_EFFECT_TYPE_E mode, uint32 color, void *param);



/*************************************************
    Function:		ygp_pic_all_screen_display
    Description:	全屏显示图片，且带缩略图
    Input:		
   		path	指定的图像的路径
		src_x	原图的x坐标
		src_y	原图的y坐标
		W	欲在屏幕上放大显示的原图中一部分的宽度
		h	欲在屏幕上放大显示的原图中一部分的高度
		Shrink_x	缩略图显示的x坐标
		Shringk_y	缩略图显示的y坐标

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
    	
*************************************************/
yresult  ygp_pic_all_screen_display(uint8 *path,int32 src_pic_x,int32 src_pic_y,int32 w,int32 h, int32 shrink_x,int32 shrink_y,int32 shrink_w,int32 shrink_h);



/*************************************************
    Function:		ygp_pic_show_all_quick
    Description:	全屏显示图片，且带缩略图
    Input:		
   		path	指定的图像的路径
		src_x	原图的x坐标
		src_y	原图的y坐标
		W	欲在屏幕上放大显示的原图中一部分的宽度
		h	欲在屏幕上放大显示的原图中一部分的高度
		Shrink_x	缩略图显示的x坐标
		Shringk_y	缩略图显示的y坐标

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
    	
*************************************************/
yresult  ygp_pic_show_all_quick(void);

/*************************************************
    Function:		ygp_pic_hide
    Description:	隐藏显示图片的图层，或隐藏放大显示的图片
    Input:		
   		无

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
    	ERR_DPF_FILE_NOEXIST	指定文件不存在
			ERR_DPF_SIZE_TOOBIG	传入的尺寸太大
*************************************************/
void ygp_pic_hide(YGP_HIDE_TYPE_E mode);



/*************************************************
    Function:		ygp_pic_turn
    Description:	对显示中的jpeg进行旋转操作
    Input:		
   		无

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/
yresult ygp_pic_turn(YGP_REEL_ANGLE_E angle);

/*************************************************
    Function:		ygp_pic_memMove
    Description:	把图像层的部分数据移动到指定位置上
    Input:		
   		无

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/
yresult ygp_pic_memMove(int x, int y, int w, int h, int dst_x, int dst_y);







/*************************************************
    Function:		ygp_set_page_show
    Description:	设置一页图片显示完成标志
    Input:		
   		无

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/
yresult ygp_set_page_show();

/*************************************************
    Function:		dpf_browse_2_front
    Description:	将浏览器所在图层置于图形层之上
    Input:		
   		无

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/
yresult ygp_browse_2_front();


/*************************************************
    Function:		dpf_pic_2_front
    Description:	将图形层所在图层置于图形层之上
    Input:		
   		无

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/
yresult ypg_pic_2_front();




/*************************************************
    Function:		dpf_set_layer_alpha
    Description:	设置图层的alpha值
    Input:		
   		无

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/
yresult ygp_set_layer_alpha(YGP_LAYER_ALPHA_TYPE_E layer,int32 x,int32 y,int32 w,int32 h,int32 alpha);



/*************************************************
    Function:		ygp_virtual_pic_display
    Description:	在虚拟图层解码图片
    Input:		
   		无

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/
yresult ygp_virtual_pic_display(char *path, int x, int y, int *w, int *h);


/*************************************************
    Function:		dpf_get_pic_info
    Description:	获取指定的图片信息
    Input:		
   		无

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/



/*************************************************
    Function:		ygp_virtual_mem_move
    Description:	移动虚拟图层中的数据
    					
    Input:		
   		无
   		ms:移动过程的时间
   		frame:移动过程中帧的个数

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/
yresult ygp_virtual_mem_move(YGP_MOVE_TYPE_E move_type, int32 w,int32 ms,int32 move_times);



/*************************************************
    Function:		ygp_draw_frame
    Description:	画矩形框
    					
    Input:		
   		
    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/
yresult ygp_draw_frame(YGP_RECT_TYPE_E rect_type, int32 x,int32 y,int32 w,int32 h,int32 thiness,int32 color);




/*************************************************
    Function:		ygp_fill_rect
    Description:	以指定颜色填充矩形框
    Input:		
   		无

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/
int ygp_fill_rect( int x, int y, int width, int height, int color );





/*************************************************
    Function:		ygp_fill_virtual_rect
    Description:	以指定颜色填充虚拟内存中的矩形框
    
    Input:		
   		width,height 有一个为0,则填充整个屏幕
		
    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
*************************************************/
int ygp_fill_virtual_rect( int x, int y, int width, int height, int color );



/*************************************************
    Function:		ygp_set_virtual_offset
    Description:	设置虚拟内存移动时的偏移量,两个都必须为正值
    Input:		
	
    Return:			
        DPF_SUCCESS:		成功
  	   DPF_FAILURE:		失败
    Err Code:

*************************************************/
yresult ygp_set_virtual_offset(int32 left_move_offset,int32 right_move_offset);




/*************************************************
    Function:		ygp_set_pic_layer_pos
    Description:	设置图形层的实际显示区域
    Input:		
	
    Return:			
        DPF_SUCCESS:		成功
  	   DPF_FAILURE:		失败
    Err Code:

*************************************************/
yresult ygp_set_pic_layer_pos(int32 x,int32 y,int32 w,int h);



/*************************************************
    Function:		ygp_pic_display_from_src
    Description:	从原图解指定的大小在指定位置画出指定宽高的图片
    Input:		
   		path	指定的图像的路径
			src_x	原图片的x坐标
			src_y	原图片的y坐标
			src_w	原图片的宽
			src_h	原图片的高
			dst_x 目标图层显示的x坐标
			dst_y 目标图层显示的y坐标
			dst_w 目标图层显示w宽度
			dst_h 目标图层显示的高度 

    Return:			
        DPF_SUCCESS:		成功
  	    DPF_FAILURE:		  失败
  	    
    Err Code:
    	ERR_DPF_FILE_NOEXIST	指定文件不存在
			ERR_DPF_SIZE_TOOBIG	传入的尺寸太大
*************************************************/
yresult  ygp_pic_display_from_src(char *path, int src_x, int src_y, int src_w, int src_h,int dst_x,int dst_y,int dst_w,int dst_h);



/*************************************************
    Function:		ygp_pic_show_shrink
    Description:	显示缩略图
    Input:		
   		 Path	指定的图像的路径
			 
			 *w	指定缩略图的宽，底层会返回缩略图实际的宽度
			 *h	指定缩略图的高，底层会放缓缩略图实际的高度
			Color	补齐时候用的颜色
			Param	未定义

    Return:			
        DPF_SUCCESS:		成功
  	   DPF_FAILURE:		失败
    Err Code:
    	ERR_DPF_FILEPATH_NOVILID	文件路径无效
			ERR_DPF_NO_SHRINK_PIC	指定的图片没有缩略图

*************************************************/
yresult  ygp_pic_show_shrink (char *path, int x,int y, int *w, int *h);

/*************************************************
    Function:		dpf_get_effect_state
    Description:	获取当前的特效播放是否完成
    					
    Input:		
	
    Return:			
        DPF_SUCCESS:		成功
  	   DPF_FAILURE:		失败
    Err Code:

*************************************************/
yresult ygp_get_effect_state(int32 *value);



/*************************************************
    Function:		ygp_stop_effect
    Description:	停止当前的特效播放
    					
    Input:		
	
    Return:			
        DPF_SUCCESS:		成功
  	   DPF_FAILURE:		失败
    Err Code:

*************************************************/
yresult ygp_stop_effect(void);


/*设置图片缓存的相关信息*/
yresult ygp_set_mem_pic_info(int32 width,int32 height,int32 count);


/*显示图片的移动*/
yresult ygp_pic_move(YGP_MOVE_TYPE_E movetype,int32 pixel);


char *ypg_get_browse_surface_buf();


int higo_appDrawFrame_yx( int x, int y, int width, int height, int thickness, int color );

int higo_appClearFrame_yx( int x, int y, int width, int height, int thickness );
int higo_appFillRect_yx( int x, int y, int width, int height, int color );
int higo_appClearRect_yx( int x, int y, int width, int height );
int higo_appTextout_yx( char* str, int x, int y, int bk_color, int color );

int higo_appOsdRefresh_yx( void );



#endif /*_HIGO_APP_H_*/


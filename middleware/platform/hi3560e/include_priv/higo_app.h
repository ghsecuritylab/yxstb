
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
	YGP_EFFECT_RANDOM	 						= -1, 										/*���Ч��*/
	YGP_EFFECT_NONE 							= 0,							/*������ʾ*/
	YGP_EFFECT_TURNPAGE						,						/*��ҳ*/
	YPG_EFFECT_ROLLPAGE						, 					/*����*/
	YGP_EFFECT_VERTICALSHUTTER		,			/* ��ֱ��Ҷ��*/
	YGP_EFFECT_HORIZONTALSHUTTER	,		/* ˮƽ��Ҷ��*/
	YGP_EFFECT_LEFTIN							,							/* ������*/
	YGP_EFFECT_TOPIN							, 							/* ���ϳ��*/
	YGP_EFFECT_TRANSIN						, 						/* ��������*/
	YGP_EFFECT_ROTATE							,  						/*����*/
	YGP_EFFECT_CENTEROUT					,  					/*���뽥��*/
	YGP_EFFECT_CENTERIN						 					/*���뽥��*/
}YGP_EFFECT_TYPE_E;


typedef enum{
	YGP_HIDE_TOP_PIC  = 0,		/*���ص�ǰ��ʾ�����ϲ��ͼƬ*/
//	YGP_HIDE_ALL_PIC,			/*������ʾ����ͼƬ��ͼ��*/
								/*2009-08-03�����������ۺ�ȥ����ѡ��*/
	YGP_HIDE_ENTILE_LAYER	/*��������ͼ��*/
}YGP_HIDE_TYPE_E;


/*��ת�Ƕ�*/
typedef enum 
{
	YGP_REEL_ANGLE_0 		= 0 ,		//	��ת�Ƕ� 0��
	YGP_REEL_ANGLE_90 	,			//	��ת�Ƕ� 90 ��
	YGP_REEL_ANGLE_180	,		//   ��ת�Ƕ� 180 ��
	YGP_REEL_ANGLE_270	,		//   ��ת�Ƕ� 270 ��
	YGP_REEL_ANGLE_BUTT  
}YGP_REEL_ANGLE_E; 


typedef enum{
	YGP_LAYER_ALPHA_BROWER = 0,	/*���������ͼ���alphaֵ*/
	YGP_LAYER_ALPHA_PIC				/*����ͼ����ʾͼ���alphaֵ*/
}YGP_LAYER_ALPHA_TYPE_E;




typedef enum 
{
	YGP_RECT_SOLID 	 ,		//	ʵ�ľ���
	YGP_RECT_EMPTY	 		//	���ľ���
} YGP_RECT_TYPE_E;

typedef enum{
	YGP_MOVE_LEFT,			/*��Ļ���ƶ�*/
	YGP_MOVE_RIGHT,		/*��Ļ���ƶ�*/
	YGP_MOVE_UP,
	YGP_MOVE_DOWN
}YGP_MOVE_TYPE_E;




/*ͼƬ��ʾ�������Ϣ*/
typedef struct  {
	int                                            pic_w;			/*ͼƬ��ʵ�ʿ��*/
	int                                           pic_h;			/*ͼƬ��ʵ�ʸ߶�*/
}YGP_PIC_INFO_S;




/*************************************************
    Function:		dpf_pic_shrink
    Description:	Ϊָ��ͼ����������ͼ
    Input:		
   		 Path	ָ����ͼ���·��
			 Shrink_path	����ͼ��·��
			 *w	ָ������ͼ�Ŀ��ײ�᷵������ͼʵ�ʵĿ��
			 *h	ָ������ͼ�ĸߣ��ײ��Ż�����ͼʵ�ʵĸ߶�
			Color	����ʱ���õ���ɫ
			Param	δ����

    Return:			
        DPF_SUCCESS:		�ɹ�
  	   DPF_FAILURE:		ʧ��
    Err Code:
    	ERR_DPF_FILEPATH_NOVILID	�ļ�·����Ч
			ERR_DPF_NO_SHRINK_PIC	ָ����ͼƬû������ͼ

*************************************************/
yresult  ygp_pic_shrink (char *path, char *shrink_path, int *w, int *h, uint32 color, void *param) ;

/*************************************************
    Function:		ygp_pic_display
    Description:	��ָ��λ�û���ָ����ߵ�ͼƬ
    Input:		
   		path	ָ����ͼ���·��
			x	ͼƬ��x����
			Y	ͼƬ��y����
			*W	ͼƬ�Ŀ�,�ײ�᷵����ʾ��ʵ�ʵĿ��
			*H	ͼƬ�ĸ�,�ײ�᷵����ʾ��ʵ�ʵĿ��
			mode	��Ч���͹�11��
			Color	����ʱ���õ���ɫ
			Param	:slide_showʱ����ٶ�,��Χ��1~16,Խ��Խ��,�Ƽ�ֵ��8.

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
    	ERR_DPF_FILE_NOEXIST	ָ���ļ�������
			ERR_DPF_SIZE_TOOBIG	����ĳߴ�̫��
*************************************************/
yresult  ygp_pic_display(char *path, int x, int y, int *w, int *h, YGP_EFFECT_TYPE_E mode, uint32 color, void *param);



/*************************************************
    Function:		ygp_pic_all_screen_display
    Description:	ȫ����ʾͼƬ���Ҵ�����ͼ
    Input:		
   		path	ָ����ͼ���·��
		src_x	ԭͼ��x����
		src_y	ԭͼ��y����
		W	������Ļ�ϷŴ���ʾ��ԭͼ��һ���ֵĿ��
		h	������Ļ�ϷŴ���ʾ��ԭͼ��һ���ֵĸ߶�
		Shrink_x	����ͼ��ʾ��x����
		Shringk_y	����ͼ��ʾ��y����

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
    	
*************************************************/
yresult  ygp_pic_all_screen_display(uint8 *path,int32 src_pic_x,int32 src_pic_y,int32 w,int32 h, int32 shrink_x,int32 shrink_y,int32 shrink_w,int32 shrink_h);



/*************************************************
    Function:		ygp_pic_show_all_quick
    Description:	ȫ����ʾͼƬ���Ҵ�����ͼ
    Input:		
   		path	ָ����ͼ���·��
		src_x	ԭͼ��x����
		src_y	ԭͼ��y����
		W	������Ļ�ϷŴ���ʾ��ԭͼ��һ���ֵĿ��
		h	������Ļ�ϷŴ���ʾ��ԭͼ��һ���ֵĸ߶�
		Shrink_x	����ͼ��ʾ��x����
		Shringk_y	����ͼ��ʾ��y����

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
    	
*************************************************/
yresult  ygp_pic_show_all_quick(void);

/*************************************************
    Function:		ygp_pic_hide
    Description:	������ʾͼƬ��ͼ�㣬�����طŴ���ʾ��ͼƬ
    Input:		
   		��

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
    	ERR_DPF_FILE_NOEXIST	ָ���ļ�������
			ERR_DPF_SIZE_TOOBIG	����ĳߴ�̫��
*************************************************/
void ygp_pic_hide(YGP_HIDE_TYPE_E mode);



/*************************************************
    Function:		ygp_pic_turn
    Description:	����ʾ�е�jpeg������ת����
    Input:		
   		��

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/
yresult ygp_pic_turn(YGP_REEL_ANGLE_E angle);

/*************************************************
    Function:		ygp_pic_memMove
    Description:	��ͼ���Ĳ��������ƶ���ָ��λ����
    Input:		
   		��

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/
yresult ygp_pic_memMove(int x, int y, int w, int h, int dst_x, int dst_y);







/*************************************************
    Function:		ygp_set_page_show
    Description:	����һҳͼƬ��ʾ��ɱ�־
    Input:		
   		��

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/
yresult ygp_set_page_show();

/*************************************************
    Function:		dpf_browse_2_front
    Description:	�����������ͼ������ͼ�β�֮��
    Input:		
   		��

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/
yresult ygp_browse_2_front();


/*************************************************
    Function:		dpf_pic_2_front
    Description:	��ͼ�β�����ͼ������ͼ�β�֮��
    Input:		
   		��

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/
yresult ypg_pic_2_front();




/*************************************************
    Function:		dpf_set_layer_alpha
    Description:	����ͼ���alphaֵ
    Input:		
   		��

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/
yresult ygp_set_layer_alpha(YGP_LAYER_ALPHA_TYPE_E layer,int32 x,int32 y,int32 w,int32 h,int32 alpha);



/*************************************************
    Function:		ygp_virtual_pic_display
    Description:	������ͼ�����ͼƬ
    Input:		
   		��

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/
yresult ygp_virtual_pic_display(char *path, int x, int y, int *w, int *h);


/*************************************************
    Function:		dpf_get_pic_info
    Description:	��ȡָ����ͼƬ��Ϣ
    Input:		
   		��

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/



/*************************************************
    Function:		ygp_virtual_mem_move
    Description:	�ƶ�����ͼ���е�����
    					
    Input:		
   		��
   		ms:�ƶ����̵�ʱ��
   		frame:�ƶ�������֡�ĸ���

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/
yresult ygp_virtual_mem_move(YGP_MOVE_TYPE_E move_type, int32 w,int32 ms,int32 move_times);



/*************************************************
    Function:		ygp_draw_frame
    Description:	�����ο�
    					
    Input:		
   		
    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/
yresult ygp_draw_frame(YGP_RECT_TYPE_E rect_type, int32 x,int32 y,int32 w,int32 h,int32 thiness,int32 color);




/*************************************************
    Function:		ygp_fill_rect
    Description:	��ָ����ɫ�����ο�
    Input:		
   		��

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/
int ygp_fill_rect( int x, int y, int width, int height, int color );





/*************************************************
    Function:		ygp_fill_virtual_rect
    Description:	��ָ����ɫ��������ڴ��еľ��ο�
    
    Input:		
   		width,height ��һ��Ϊ0,�����������Ļ
		
    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
*************************************************/
int ygp_fill_virtual_rect( int x, int y, int width, int height, int color );



/*************************************************
    Function:		ygp_set_virtual_offset
    Description:	���������ڴ��ƶ�ʱ��ƫ����,����������Ϊ��ֵ
    Input:		
	
    Return:			
        DPF_SUCCESS:		�ɹ�
  	   DPF_FAILURE:		ʧ��
    Err Code:

*************************************************/
yresult ygp_set_virtual_offset(int32 left_move_offset,int32 right_move_offset);




/*************************************************
    Function:		ygp_set_pic_layer_pos
    Description:	����ͼ�β��ʵ����ʾ����
    Input:		
	
    Return:			
        DPF_SUCCESS:		�ɹ�
  	   DPF_FAILURE:		ʧ��
    Err Code:

*************************************************/
yresult ygp_set_pic_layer_pos(int32 x,int32 y,int32 w,int h);



/*************************************************
    Function:		ygp_pic_display_from_src
    Description:	��ԭͼ��ָ���Ĵ�С��ָ��λ�û���ָ����ߵ�ͼƬ
    Input:		
   		path	ָ����ͼ���·��
			src_x	ԭͼƬ��x����
			src_y	ԭͼƬ��y����
			src_w	ԭͼƬ�Ŀ�
			src_h	ԭͼƬ�ĸ�
			dst_x Ŀ��ͼ����ʾ��x����
			dst_y Ŀ��ͼ����ʾ��y����
			dst_w Ŀ��ͼ����ʾw���
			dst_h Ŀ��ͼ����ʾ�ĸ߶� 

    Return:			
        DPF_SUCCESS:		�ɹ�
  	    DPF_FAILURE:		  ʧ��
  	    
    Err Code:
    	ERR_DPF_FILE_NOEXIST	ָ���ļ�������
			ERR_DPF_SIZE_TOOBIG	����ĳߴ�̫��
*************************************************/
yresult  ygp_pic_display_from_src(char *path, int src_x, int src_y, int src_w, int src_h,int dst_x,int dst_y,int dst_w,int dst_h);



/*************************************************
    Function:		ygp_pic_show_shrink
    Description:	��ʾ����ͼ
    Input:		
   		 Path	ָ����ͼ���·��
			 
			 *w	ָ������ͼ�Ŀ��ײ�᷵������ͼʵ�ʵĿ��
			 *h	ָ������ͼ�ĸߣ��ײ��Ż�����ͼʵ�ʵĸ߶�
			Color	����ʱ���õ���ɫ
			Param	δ����

    Return:			
        DPF_SUCCESS:		�ɹ�
  	   DPF_FAILURE:		ʧ��
    Err Code:
    	ERR_DPF_FILEPATH_NOVILID	�ļ�·����Ч
			ERR_DPF_NO_SHRINK_PIC	ָ����ͼƬû������ͼ

*************************************************/
yresult  ygp_pic_show_shrink (char *path, int x,int y, int *w, int *h);

/*************************************************
    Function:		dpf_get_effect_state
    Description:	��ȡ��ǰ����Ч�����Ƿ����
    					
    Input:		
	
    Return:			
        DPF_SUCCESS:		�ɹ�
  	   DPF_FAILURE:		ʧ��
    Err Code:

*************************************************/
yresult ygp_get_effect_state(int32 *value);



/*************************************************
    Function:		ygp_stop_effect
    Description:	ֹͣ��ǰ����Ч����
    					
    Input:		
	
    Return:			
        DPF_SUCCESS:		�ɹ�
  	   DPF_FAILURE:		ʧ��
    Err Code:

*************************************************/
yresult ygp_stop_effect(void);


/*����ͼƬ����������Ϣ*/
yresult ygp_set_mem_pic_info(int32 width,int32 height,int32 count);


/*��ʾͼƬ���ƶ�*/
yresult ygp_pic_move(YGP_MOVE_TYPE_E movetype,int32 pixel);


char *ypg_get_browse_surface_buf();


int higo_appDrawFrame_yx( int x, int y, int width, int height, int thickness, int color );

int higo_appClearFrame_yx( int x, int y, int width, int height, int thickness );
int higo_appFillRect_yx( int x, int y, int width, int height, int color );
int higo_appClearRect_yx( int x, int y, int width, int height );
int higo_appTextout_yx( char* str, int x, int y, int bk_color, int color );

int higo_appOsdRefresh_yx( void );



#endif /*_HIGO_APP_H_*/


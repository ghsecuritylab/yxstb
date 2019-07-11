
#ifndef _HIGO_LOW_H_
#define _HIGO_LOW_H_


#include "hi_go.h"
/** ���ظ�ʽ */
typedef enum
{
	YX_HIGO_PF_CLUT8 = 0,  /**< ��ɫ�� */
	YX_HIGO_PF_CLUT1,
	YX_HIGO_PF_CLUT4,
	YX_HIGO_PF_4444,       /**< һ������ռ16bit��ARGBÿ����ռ4bit������ַ�ɸ��������� */
	YX_HIGO_PF_0444,       /**< һ������ռ16bit��ARGBÿ����ռ4bit������ַ�ɸ���������, A������������ */

	YX_HIGO_PF_1555,       /**< һ������ռ16bit��RGBÿ����ռ5bit��A����1bit������ַ�ɸ��������� */
	YX_HIGO_PF_0555,       /**< һ������ռ16bit��RGBÿ����ռ5bit��A����1bit������ַ�ɸ���������, A������������ */
	YX_HIGO_PF_565,        /**< һ������ռ16bit��RGBÿ�����ֱ�ռ5bit��6bit��5bit������ַ�ɸ��������� */
	YX_HIGO_PF_8565,       /**< һ������ռ24bit��ARGBÿ�����ֱ�ռ8bit��5bit��6bit��5bit������ַ�ɸ��������� */
	YX_HIGO_PF_8888,       /**< һ������ռ32bit��ARGBÿ����ռ8bit������ַ�ɸ��������� */
	YX_HIGO_PF_0888,       /**< һ������ռ32bit��ARGBÿ����ռ8bit������ַ�ɸ��������У�A������������ */

	YX_HIGO_PF_YUV400,     /**< ��˼�����semi-planar YUV 400��ʽ */    
	YX_HIGO_PF_YUV420,     /**< ��˼�����semi-planar YUV 420��ʽ */
	YX_HIGO_PF_YUV422,     /**< ��˼�����semi-planar YUV 422��ʽ  ˮƽ������ʽ*/
	YX_HIGO_PF_YUV422_V,   /**< ��˼�����semi-planar YUV 422��ʽ  ��ֱ������ʽ*/    
	YX_HIGO_PF_YUV444,     /**< ��˼�����semi-planar YUV 444��ʽ */

	YX_HIGO_PF_A1, 
	YX_HIGO_PF_A8,
	YX_HIGO_PF_BUTT
} YX_HIGO_PF_E;

typedef enum
{
	YX_HIGO_COMPOPT_NONE = 0, /**< ��ʹ�û�ϣ���copy */

	YX_HIGO_COMPOPT_SRCOVER,  /**< Porter/Duff SrcOver��ϲ��� */
	YX_HIGO_COMPOPT_AKS,      /**< ����Ŀ��surfaceΪ��͸��,��alpha��ϣ��������Դalpha */
	YX_HIGO_COMPOPT_AKD,      /**< ����Ŀ��surfaceΪ��͸��,��alpha��ϣ��������Ŀ��alpha */

	YX_HIGO_COMPOPT_BUTT
} YX_HIGO_COMPOPT_E;

typedef enum
{
	YX_HIGO_ZORDER_MOVETOP = 0,  /*�Ƶ����*/  
	YX_HIGO_ZORDER_MOVEUP,	  /*������*/  
	YX_HIGO_ZORDER_MOVEBOTTOM,	  /*�Ƶ���ײ�*/  
	YX_HIGO_ZORDER_MOVEDOWN,     /*������*/  
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

/* ����һ�����������������ϴ����Ĵ��� */
typedef struct osd_window_link{
	HI_HANDLE hWindow;
	struct osd_window_link* pNext;
}OSDWINLINK, *OSDWINLINK_T;

/*
	�������ܣ���ʼ��higo,���Ҵ���������
	����˵����
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵�������øú�������HIGO��صĳ�ʼ��,������Bliterģ��,������ģ��,��ʾ�豸ģ��,
	�ڴ����ģ��ĳ�ʼ��
*/
int higo_init_yx( void );

/*
	�������ܣ�ȥ��ʼ��higo,��������������
	����˵����
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵�������øú�������HIGO��ص�ȥ��ʼ��,������Bliterģ��,������ģ��,��ʾ�豸ģ��,
	�ڴ����ģ���ȥ��ʼ��
*/
int higo_quit_yx( void );

/*
	�������ܣ�����osdͼ�β�
	����˵����
		width		osd��Ŀ��
		height		osd��ĸ߶�
		YX_HIGO_PF_E	osd������ظ�ʽ
		flip_count	����Flip���ڴ����
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵�����˺������Ѿ���ͼ���ˢ��ģʽд��Ϊ����˸,˫����(�˴���Ҫ�ͺ�˼��ͨ,���ܻ��)
	�����ڸú����д������������ɫΪ0xFF00FF00
*/
int higo_osdCreate_yx( int width, int height, int offset_x, int offset_y, YX_HIGO_PF_E color_format, int flip_count );

/*
	�������ܣ�����osdͼ�β�
	����˵����
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵�������ô˺�����ʱ��,Ӧ�ñ�֤�ڸò㴴���Ĵ��ں���ò���ص��ڴ��㶼�Ѿ�����
*/
int higo_osdDistory_yx( void );

/*
	�������ܣ���ȡosdͼ�β�Ŀ�
	����˵����
	����ֵ��
		int		�����ɹ�����osdͼ�β�Ŀ�ʧ�ܷ��� ��-1��
	����˵����
*/
int higo_getOsdWidth_yx( void );

/*
	�������ܣ���ȡosdͼ�β��
	����˵����
	����ֵ��
		int		�����ɹ�����osdͼ�β�ĸߣ�ʧ�ܷ��� ��-1��
	����˵����
*/
int higo_getOsdHeight_yx( void );
/*
	�������ܣ����������ͼ�β�
	����˵����
		width		�������Ŀ��
		height		�������ĸ߶�
		YX_HIGO_PF_E	�����������ظ�ʽ
		flip_count	����Flip���ڴ����
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵�����˺������Ѿ���ͼ���ˢ��ģʽд��Ϊ����˸,˫����(�˴���Ҫ�ͺ�˼��ͨ,���ܻ��)
	�����ڸú����д������������ɫΪ0xFF00FF00
*/
int higo_browserCreate_yx( int width, int height, int offset_x, int offset_y, YX_HIGO_PF_E color_format, int flip_count );

/*
	�������ܣ�����osdͼ�β�
	����˵����
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵�������ô˺�����ʱ��,Ӧ�ñ�֤�ڸò㴴���Ĵ��ں���ò���ص��ڴ��㶼�Ѿ�����
*/
int higo_browserDistory_yx( void );

/*
	�������ܣ���ȡָ��surface���ڴ��׵�ַ
	����˵����
		surface		��Ҫ��ȡsurface�ľ��
	����ֵ��
		HI_VOID*	�����ɹ�����ָ��surface���ڴ��׵�ַ��ʧ�ܷ��� NULL
	����˵�������ô˺���ʱӦ��ȷ����surface�������ʵ��Ч�ģ��Ҳ��ܶ�ͬһsurface�ظ�����
*/
HI_VOID* higo_getSurfaceBuf_yx( HI_HANDLE surface );

/*
	�������ܣ�����ͼ���͸����
	����˵����
		layer		ͼ��ľ��
		Alpha		ͼ���͸����(0-255)
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵�����ӿ��е�͸������32λ�ģ����ڸýӿ���ת��Ϊ8λ
*/
int higo_setLayerAlpha_yx( HI_HANDLE layer, HI_S32 Alpha );

/*
	�������ܣ�����ͼ���ڶ���ʾ�豸�ϵ���ʼ����
	����˵����
		layer		ͼ��ľ��
		x		ͼ�����ն���ʾ�豸�ϵ���ʼ������
		y		ͼ�����ն���ʾ�豸�ϵ���ʼ������
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵����
*/
int higo_setLayerCoords_yx( HI_HANDLE layer, HI_S32 x, HI_S32 y );

/*
	�������ܣ�����ͼ���ڶ���ʾ�豸�ϵ���ʾ״̬
	����˵����
		layer		ͼ��ľ��
		flag		��1����ʾ��ʾ��ͼ�㣬��0����ʾ���ظ�ͼ��
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵����������Ч,����ˢ��
*/
int higo_layerViewStatus_yx( HI_HANDLE layer, int flag );

/*
	�������ܣ�����surface�ϵ�����ˢ�µ��ն���ʾ�豸��
	����˵����
		layer		��Ҫˢ��ͼ�β�ľ��
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵����
*/
int higo_refresh_yx( HI_HANDLE layer );

/*
	�������ܣ���ȡHIGOHANDLEYXȫ�ֽṹ����ָ��Ԫ�ص�����ֵ
	����˵����
		handle		ͼ����صľ��
	����ֵ��
		int		�����ɹ����� ȫ�ֽṹ����ָ��Ԫ�ص�����ֵ��ʧ�ܷ��� -1
	����˵�����ܹ���ȡ�ľ��ֻ�����Ѿ���HIGOHANDLEYX�������ľ��
*/
unsigned int higo_gethandel_yx( int handle );

/*
	�������ܣ���OSD��������ϴ���һ������
	����˵����
		h_window	������������������ڵľ��
		x		���������������ж���ʾ��Ļ�ϵĺ�����
		y		���������������ж���ʾ��Ļ�ϵ�������
		width		�����������ڵĿ�
		height		�����������ڵĸ�
		color		�����������ڵ���ɫ,��ɫ��ʽ8888
		opacity		�����������ڵ�͸���ȣ�0-255��
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵������Ҫ��higo_osdDistoryWindow_yx����ʹ���ͷ���Դ
*/
int higo_osdAddWindow_yx( HI_HANDLE* h_window, int x, int y, int width, int height, int color, int opacity );

/*
	�������ܣ�����һ������
	����˵����
		h_window	�������ٴ��ڵľ��

	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵����
*/
int higo_osdDistoryWindow_yx( HI_HANDLE h_window );

/*
	�������ܣ���ָ���Ĵ���������ַ���
	����˵����
		h_window	�ַ������������
		x		����ַ����ڴ����еĺ�����
		y		����ַ����ڴ����е�������
		width		����ַ����ڴ����еĿ�
		height		����ַ����ڴ����еĸ�
		bk_color	����ַ����ı�����ɫ,��ɫ��ʽ8888
		color		����ַ�������ɫ,��ɫ��ʽ8888
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵�����⺯����Ҫ���ṩ���������Գ�����ʾ����ʹ��
*/
int higo_textoutWindow_yx( HI_HANDLE h_window, char* str, int x, int y, int bk_color, int color );

/*
	�������ܣ���ָ���Ĵ����������ο�
	����˵����
		h_window	���ο���������
		x		���ο��ڴ����еĺ�����
		y		���ο��ڴ����е�������
		width		���ο��ڴ����еĿ�
		height		���ο��ڴ����еĸ�
		color		���ο�������ɫ,��ɫ��ʽ8888
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵�����⺯����Ҫ���ṩ���������Գ�����ʾ����ʹ��
*/
int higi_fillRectWindow_yx( HI_HANDLE h_window, int x, int y, int width, int height, int color );

/*
	�������ܣ���ָ����surface������ض��ľ���
	����˵����
		surface		�����ε�surface
		x		���ο���surface�ϵĺ�����
		y		���ο���surface�ϵ�������
		width		���ο���surface�ϵĿ�
		height		���ο���surface�ϵĸ�
		color		���ο����ɫ,��ɫ��ʽ8888
		operators	��ͼʱ�Ĳ�������
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵���������е�operators��ο���˼�ĵ�����֧�ֵĲ�������
*/
int higo_fillRect_yx( HI_HANDLE surface, int x, int y, int width, int height, int color, YX_HIGO_COMPOPT_E operators );

/*
	�������ܣ���ָ����surface������ض����ַ���
	����˵����
		surface		����ַ�����surface
		x		����ַ����ڴ����еĺ�����
		y		����ַ����ڴ����е�������
		width		����ַ����ڴ����еĿ�
		height		����ַ����ڴ����еĸ�
		bk_color	����ַ����ı�����ɫ,��ɫ��ʽ8888
		color		����ַ�������ɫ,��ɫ��ʽ8888
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵����
*/
int higo_textout_yx( HI_HANDLE surface, char *str, int x, int y, int bk_color, int color );

/*
	�������ܣ�����osd��Ĳ���
	����˵����
		flag		��1����ʾ��OSD����������ϲ�,'0'��ʾ��OSD�㱾���λ������Ų��һ��

	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵������˼������Ƶ��,ͼ��0(osd��),ͼ��1(browser��),Ӳ������
*/
int higo_osdUpDown_yx( int flag );

/*
	�������ܣ�����ָ�����ڵĲ�͸����
	����˵����
		deskTop		Ҫ���ô���������������
		h_window	Ҫ���ô��ھ��
		opacity		͸���ȣ�0-255��
	����ֵ��
		int		�����ɹ����� HI_SUCCESS��ʧ�ܷ��� HI_FAILURE
	����˵����
*/
int higo_setWindowOpacity_yx( HI_HANDLE deskTop,  HI_HANDLE h_window, int opacity );

//�ṩһ����ʾ����ĳ�����εĽӿ�(̨������)


//�ṩһ����ͼ�㴴�����ڵĽӿ�
int fill_rectangel_window( HI_HANDLE h_window, int x, int y, int width, int height, int color );


//��������ʹ��
//Ҫ���ǵ����¼���Ӧ��
//                     a�����ݵ���Դ�����������ļ���Ҳ���������ڴ�
//                     b�����������ݿ��ܲ�����Ҫ��ʾ��ֻ��Ҫ����
//                     c��������������Ҫ��ָ����λ����ʾ����
//                     d����Ҫ���Ƕ�̬gif�Ľ���
//                     e����Ҫ���ϼ�����Ч�Ĵ���

typedef union {
	//����ԴΪ�ڴ��ʱ��Ҫ����Ϣ
	struct{
		char* pAddr;      /**< �ڴ�ָ���ַ*/
		int Length;       /**< ����*/
	}MemInfo;

	//ͼƬ�ļ���
	const char* pFileName;

} YX_HIGO_DEC_SRCINFO_U;

typedef struct picture_dec_info{
	short isSaveToFile;	//'0'��ʾֱ����ʾ,'1'��ʾ�����ļ�
	short isDataFormFile;   //'0'��ʾ������Դ���ڴ�,'1'��ʾ������Դ���ļ�
	short x;
	short y;
	short width;
	short height;
	char savedFilename[256];
	YX_HIGO_DEC_SRCINFO_U decodeSrcInfo;
}PICTUREDECINFO,*PICTUREDECINFO_T;
int processPIC( HI_HANDLE surface, PICTUREDECINFO_T decInfo );

//��RGB���ݵ���ʾ(�������,����osd��ĳЩ�����Ӧ��)
int RGBstuff( HI_HANDLE surface, const unsigned char* buf, int lenght );

//����Ҫ�ص����higo��blit��صĲ���,�����ǶԴ��������

#endif /*_HIGO_LOW_H_*/

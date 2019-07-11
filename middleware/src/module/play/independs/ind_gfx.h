#ifndef __IND_GFX_H__
#define __IND_GFX_H__

#ifdef __cplusplus
extern "C" {
#endif

enum {
	PLANE_TYPE_NORMAL = 0,
	PLANE_TYPE_DYNAMIC,
	PLANE_TYPE_TOP,
	PLANE_TYPE_BOTTOM,
	PLANE_TYPE_PIP,
	PLANE_TYPE_MAX
};

#define PLANE_NUM_MAX	32
#define FRAME_NUM_MAX	32

enum {
	FRAME_TYPE_IMAGE = 1,
	FRAME_TYPE_PLANE
};

enum {
	RECT_CMP_ERROR = -1,//交叉
	RECT_CMP_BROTHER = 0,
	RECT_CMP_EQUAL,
	RECT_CMP_CHILD,
	RECT_CMP_FATHER
};

struct ind_rect {
	int left;
	int top;
	int right;
	int bottom;
};

typedef struct ind_plane* ind_plane_t;
typedef struct ind_gfx* ind_gfx_t;

typedef void (*gfx_refresh_f)(void);
typedef void (*gfx_notice_f)(void);

ind_gfx_t ind_gfx_create(int depth, int width, int height, char *framebuffer, gfx_refresh_f refresh);

int ind_gfx_destroy(struct ind_gfx* gfx);

void ind_gfx_notice_regist(gfx_notice_f dirty, gfx_notice_f empty);

int ind_gfx_alpha_set(ind_gfx_t gfx, unsigned int alpha);

unsigned int ind_gfx_alpha_get(ind_gfx_t gfx);

int ind_gfx_trans_set(unsigned int alpha);

unsigned int ind_gfx_trans_get(void);

void ind_gfx_init_font(ind_gfx_t gfx, unsigned char* ascii, unsigned char* gb2312);

ind_plane_t ind_gfx_plane_root(ind_gfx_t gfx);

ind_plane_t ind_gfx_plane_create(ind_gfx_t gfx, int width, int height);

int ind_gfx_plane_info(ind_gfx_t gfx, ind_plane_t planet, unsigned char** buffer, int* width, int* height);

int ind_gfx_image(ind_gfx_t gfx, ind_plane_t plane, int x, int y, unsigned char *imgbuf, int imglen);

int ind_gfx_image_range(ind_gfx_t gfx, ind_plane_t plane, int x, int y, unsigned char *imgbuf, int imglen, 
					int r_x, int r_y, int r_width, int r_height);

int ind_gfx_plane(ind_gfx_t gfx, ind_plane_t plane, int x, int y, ind_plane_t child);

int ind_gfx_plane_range(ind_gfx_t gfx, ind_plane_t plane, int x, int y, ind_plane_t child, 
					int r_x, int r_y, int r_width, int r_height);

int ind_gfx_fillrect(ind_gfx_t gfx, ind_plane_t plane, int x, int y, int width, int height, unsigned int color);

int ind_gfx_fillbuf(ind_gfx_t gfx, ind_plane_t plane, int x, int y, char *buffer, int width, int height);

int ind_gfx_text16(ind_gfx_t gfx, ind_plane_t plane, int x, int y, unsigned char *str, int num, unsigned int color);

int ind_gfx_text24(ind_gfx_t gfx, ind_plane_t plane, int x, int y, unsigned char *str, int num, unsigned int color);

int ind_gfx_frame_image(ind_gfx_t gfx, int x, int y, unsigned char *imgbuf, int imglen, int type);

int ind_gfx_frame_plane(ind_gfx_t gfx, int x, int y, ind_plane_t plane, int type);

int ind_gfx_frame_delete(ind_gfx_t gfx, void *imgbuf);

int ind_rect_comp(struct ind_rect *rect, int left, int top, int right, int bottom);

/*
	depth OSD颜色深度 16 或 32
	osd_buf OSD缓冲
	osd_width, osd_hieght OSD尺寸
	x, y 图像要显示在OSD上的位置
	img_buf, img_len 图像缓冲和缓冲长度
 */
int ind_img_draw(int depth, char* osd_buf, int osd_width, int osd_hieght, int x, int y, 
	char *img_buf, int img_len);

/*
	获取图像宽高
 */
int ind_img_info(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight);

/*
	检测图像文件有效性，目前只支持jpg和gif
 */
int ind_img_check(unsigned char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* __IND_GFX_H__ */

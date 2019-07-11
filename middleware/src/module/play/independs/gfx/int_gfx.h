
#ifndef __INT_GFX_H__
#define __INT_GFX_H__

#include "ind_gfx.h"

enum {
	PLANE_INDEX_BOTTOM = 0,
	PLANE_INDEX_PIP,
	PLANE_INDEX_TOP,
	PLANE_INDEX_FIXED
};

struct ind_plane
{
	unsigned char*	buffer;
	int	width;
	int	height;
	unsigned int	alpha;
};

struct ind_frame
{
	int					index;
	int					type;//FRAME_TYPE_IMAGE or FRAME_TYPE_PLANE
	unsigned int		alpha;

	int					state;

	int					plane_type;
	unsigned int 		plane_mask;

	struct ind_frame*	father;
	struct ind_frame*	next;
	struct ind_frame*	fst_child;
	struct ind_frame*	lst_child;

	unsigned char*		imgbuf;
	int					imglen;

	struct ind_rect		rect;
};

typedef	struct ind_frame*	ind_frame_t;

struct ind_gfx {
	unsigned int trans;//Í¸Ã÷É«

	void (*init_font)(unsigned char* ascii, unsigned char* gb2312);
	ind_plane_t (*plane_root)(void);
	ind_plane_t (*plane_create)(int width, int height);
	void (*plane_delete)(ind_plane_t plane);
	int (*alpha_set)(unsigned int alpha);
	unsigned int (*alpha_get)(void);
	int (*plane_info)(ind_plane_t planet, unsigned char** buffer, int *width, int *height);
	int (*image)(ind_plane_t plane, int x, int y, unsigned char *midimg, int imglen, 
						int r_x, int r_y, int r_width, int r_height, unsigned int alpha);
	int (*plane)(ind_plane_t plane, int x, int y, ind_plane_t child, 
						int r_x, int r_y, int r_width, int r_height, unsigned int alpha);
	int (*fillrect)(ind_plane_t plane, int x, int y, int width, int height, unsigned int color);
	int (*fillbuf)(ind_plane_t plane, int x, int y, char *buffer, int width, int height);
	int (*text16)(ind_plane_t plane, int x, int y, unsigned char *str, int num, unsigned int color);
	int (*text24)(ind_plane_t plane, int x, int y, unsigned char *str, int num, unsigned int color);
	int (*frame_image)(int x, int y, unsigned char *imgbuf, int imglen, int type, unsigned int alpha);
	int (*frame_plane)(int x, int y, ind_plane_t plane, int type, unsigned int alpha);
	int (*frame_delete)(void *imgbuf);
};

extern unsigned char g_gfx_asc16[];

void int_gfx_refresh(void);
void int_gfx_notice_dirty(void);
void int_gfx_notice_empty(void);

int ind_gfx32_init(struct ind_gfx* gfx, int width, int height, char *framebuffer);
int ind_gfx16_init(struct ind_gfx* gfx, int width, int height, char *framebuffer);

int ind_img_info(unsigned char *imgbuf, int imglen, int *pwidth, int *pheight);
int int_img_draw(int depth, ind_plane_t plane, unsigned int trans, int x, int y, unsigned char *imgbuf, int imglen, 
				int i_x, int i_y, int i_width, int i_height, unsigned int alpha);

unsigned int gfx_mem2uint(unsigned char *buf);

#endif//__INT_GFX_H__

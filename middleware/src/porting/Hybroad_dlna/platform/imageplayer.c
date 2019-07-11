#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "libzebra.h"
#include "HySDK.h"
#include "ind_mem.h"

typedef struct _hysdk_image_player_ HYSDK_IMG_PLAYER;
struct _hysdk_image_player_ {
	int scale;
	int w;
	int h;
};

int HySDK_IPlayer_Play(HYSDK_IMGP player, char *path, int slideMode)
{
	int ret, w, h, width, height;
	int speed = 8;
	HYSDK_IMG_PLAYER* me = (HYSDK_IMG_PLAYER*)player;

	ret = ygp_pic_getInfo(path, &(me->w), &(me->h));
	printf("w ======== %d\th ======= %d\n", me->w, me->h);
	ygp_layer_getScreenSize(&width, &height);
	//if(w <= 0 || h <= 0 || w >= 1280 || h >= 720)
	{
		w = 1280;
		h = 720;
	}
	me->scale = 100;
	return ygp_pic_display(path, 0, 0, &w, &h, 0, 0xff000000, (void *)speed);
}

int HySDK_IPlayer_GetMediaInfo(HYSDK_IMGP player, int *w, int *h)
{
	HYSDK_IMG_PLAYER* me = (HYSDK_IMG_PLAYER*)player;
	
	if(w) *w = me->w;
	if(h) *h = me->h;
	return 0;
}

int HySDK_IPlayer_Rotate(HYSDK_IMGP player, int radian)
{
	int mode = 1;
	return ygp_pic_turn(mode);
}

int HySDK_IPlayer_Zoom(HYSDK_IMGP player, int percent)
{
	HYSDK_IMG_PLAYER* me = (HYSDK_IMG_PLAYER*)player;
	me->scale = percent;
	return ygp_pic_zoom(percent/100, 0, 0);
}

int HySDK_IPlayer_Move(HYSDK_IMGP player, int x_pixel, int y_pixel)
{
	HYSDK_IMG_PLAYER* me = (HYSDK_IMG_PLAYER*)player;
	
	return ygp_pic_zoom(me->scale/100, x_pixel, y_pixel);
}

int HySDK_IPlayer_Stop(HYSDK_IMGP player)
{
	ygp_pic_hide();
	return 0;
}

void HySDK_IPlayer_Release(HYSDK_IMGP player)
{
	HYSDK_IMG_PLAYER* me = (HYSDK_IMG_PLAYER*)player;
	if( me )
	{
		HySDK_IPlayer_Stop(player);
		IND_FREE( me );
	}
}

HYSDK_IMGP HySDK_IPlayer_Create(void)

{
	HYSDK_IMG_PLAYER* me = (HYSDK_IMG_PLAYER*)IND_MALLOC(sizeof(HYSDK_IMG_PLAYER));

	memset(me, 0, sizeof(HYSDK_IMG_PLAYER));
	return (int)me;
}



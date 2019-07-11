#include <stdio.h>
#include <pthread.h>

#include "cloud_api.h"
#include "libzebra.h"

extern int layerMixerDevice_middleLayerHandle(void);

static pthread_mutex_t g_draw_sem;
static int g_HandleAddr = 0;
static int preDis_x, preDis_y, preDis_w, preDis_h;
static int preCli_x, preCli_y, preCli_w, preCli_h;

void CStb_PreviousGraphicsPosSet(void)
{
	int handle = layerMixerDevice_middleLayerHandle();
	
	ygp_layer_setDisplayPosition(handle, preDis_x, preDis_y, preDis_w, preDis_h); 
	ygp_layer_setClipPosition(handle, preCli_x, preCli_y, preCli_w, preCli_h);
}
	
void CStb_GraphicsGetInfo(OUT C_U8 **pBuff, OUT C_U32 *pWidth, OUT C_U32 *pHeight, OUT C_ColorMode *pColorMode)
{
	int pitch = 0;
	int handle = layerMixerDevice_middleLayerHandle();
	
	CLOUD_LOG_TRACE("Get graphic infomation. \n");

	/* Get previous graphic positon. */
	ygp_layer_getDisplayPosition(handle, &preDis_x, &preDis_y, &preDis_w, &preDis_h); 
	ygp_layer_getClipPosition(handle, &preCli_x, &preCli_y, &preCli_w, &preCli_h); 
	
	CLOUD_LOG_TRACE("Get graphic infomation.handle:%d  pre_x:%d, pre_y:%d, pre_w:%d, pre_h%d\n",
						handle, preDis_x, preDis_y, preDis_w, preDis_h);
	CLOUD_LOG_TRACE("Get graphic infomation.handle:%d  preCli_x:%d, preCli_y:%d, preCli_w:%d, preCli_h%d\n",
					handle, preCli_x, preCli_y, preCli_w, preCli_h);

	/* Set default graphic position */
	ygp_layer_setDisplayPosition(handle, 0, 0, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT); 
	ygp_layer_setClipPosition(handle, 0, 0, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT);

	int ret = ygp_layer_getMemory(handle, &g_HandleAddr, &pitch);
	if(ret){
		CLOUD_LOG_TRACE("Get graphic memory failure\n");
		return;
	}

    *pBuff = (C_U8 *)g_HandleAddr;
    *pWidth = SCREEN_MAX_WIDTH;
    *pHeight = SCREEN_MAX_HEIGHT;
    *pColorMode = ColorMode_ARGB8888;
}

C_RESULT CStb_UpdateRegion(IN C_U32 uX, IN C_U32 uY, IN C_U32 uWidth, IN C_U32 uHeight)
{	
    pthread_mutex_lock(&g_draw_sem);
    ygp_layer_redraw(1);
    pthread_mutex_unlock(&g_draw_sem);
    return CLOUD_OK;
}

void CStb_CleanScreen(void)
{
	int handle = layerMixerDevice_middleLayerHandle();
	
    ygp_layer_drawFilledRect(handle, 0, 0, SCREEN_MAX_WIDTH, SCREEN_MAX_HEIGHT, 0);
    ygp_layer_redraw(1);
}


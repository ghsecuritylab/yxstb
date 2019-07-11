/***************************************************************************
 *     Copyright(c) 2007-2008, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 * $Create_Date: 2008-4-2 19:37 $
 * Revision History:
 * 1. by SunnyLi  2008-4-2 19:38 create

 * $contact at lizhaohui@yu-xing.com
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "hi_unf_sound.h"
#include "hi_unf_disp.h"
#include "hi_unf_vo.h"
#include "hi_unf_avplay.h"
#include "hi_unf_demux.h"
#include "hi_unf_subtitle.h"

#include "app/Assertions.h"

HI_HANDLE    g_DispHandle = 0;

int yx_drv_display_open(int pnmode)	//����������ʽ�����á�
{
    HI_UNF_DISP_INIT_PARA_S     DispPara;
    HI_U32                      LayerOrder[4];
    HI_UNF_OUTPUT_INTERFACE_S   IntfMode;
    HI_S32                      Ret;

    Ret = HI_UNF_DISP_Init();
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_DISP_Init\n");
    }

    DispPara.u32ID = 0;
    Ret = HI_UNF_DISP_Create(&DispPara, &g_DispHandle);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_DISP_Create\n");
    }

    Ret = HI_UNF_DISP_SetMacrovision(g_DispHandle, HI_UNF_MACROVISION_MODE_TYPE0, NULL);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_DISP_SetMacrovision\n");
    }
    //���Ͷȣ����ӱ���ɫ���ӿ�ģʽ��ɨ�跽ʽ��tv����lcd
    IntfMode.bScartEnable = HI_FALSE;//????????????????????????????????
    IntfMode.bScartEnable = HI_FALSE;

    IntfMode.enDacMode[0] = HI_UNF_DAC_MODE_SVIDEO_C;
    IntfMode.enDacMode[1] = HI_UNF_DAC_MODE_SILENCE;
    IntfMode.enDacMode[2] = HI_UNF_DAC_MODE_SVIDEO_Y;
    IntfMode.enDacMode[3] = HI_UNF_DAC_MODE_CVBS;

    Ret = HI_UNF_DISP_SetDacMode(g_DispHandle, &IntfMode);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_DISP_SetDacMode\n");
    }

    if(pnmode) {
        HI_UNF_DISP_SetFormat(g_DispHandle, HI_UNF_ENC_FMT_PAL);
    } else {
        HI_UNF_DISP_SetFormat(g_DispHandle, HI_UNF_ENC_FMT_NTSC);
    }

    LayerOrder[0] = 3;//��Ƶ��
    LayerOrder[1] = 1;//ͼ�β�0
    LayerOrder[2] = 0;//ͼ�β�1
    LayerOrder[3] = 2;//����

    Ret = HI_UNF_DISP_SetLayerOrder(g_DispHandle, LayerOrder, 4);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_DISP_SetLayerOrder\n");
    }
    HI_UNF_DISP_DisableLayer(g_DispHandle, HI_UNF_DISP_LAYER_OSD_2);

    Ret = HI_UNF_DISP_SetEnable(g_DispHandle, HI_TRUE);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_DISP_SetEnable\n");
    }

    return 0;
Err:
    return -1;
}

int yx_drv_display_set(int enable)
{
    HI_UNF_OUTPUT_INTERFACE_S   IntfMode;
    HI_S32                      Ret;

    if(enable) {
        IntfMode.bScartEnable = HI_FALSE;
        IntfMode.enDacMode[0] = HI_UNF_DAC_MODE_SVIDEO_C;
        IntfMode.enDacMode[1] = HI_UNF_DAC_MODE_SILENCE;
        IntfMode.enDacMode[2] = HI_UNF_DAC_MODE_SVIDEO_Y;
        IntfMode.enDacMode[3] = HI_UNF_DAC_MODE_CVBS;
    } else {
        IntfMode.bScartEnable = HI_FALSE;
        IntfMode.enDacMode[0] = HI_UNF_DAC_MODE_SILENCE;
        IntfMode.enDacMode[1] = HI_UNF_DAC_MODE_SILENCE;
        IntfMode.enDacMode[2] = HI_UNF_DAC_MODE_SILENCE;
        IntfMode.enDacMode[3] = HI_UNF_DAC_MODE_SILENCE;
    }

    Ret = HI_UNF_DISP_SetDacMode(g_DispHandle, &IntfMode);
    if(Ret != HI_SUCCESS) {
        ERR_OUT("HI_UNF_DISP_SetDacMode\n");
    }

    return 0;
Err:
    return -1;
}

int yx_drv_display_show_logo(void)
{

    int l_ret = -1;
    HI_U32	LayerOrder[4];

    LayerOrder[0] = 3;//��Ƶ��
    LayerOrder[1] = 0;//ͼ�β�0
    LayerOrder[2] = 1;//ͼ�β�1
    LayerOrder[3] = 2;//����


    l_ret = HI_UNF_DISP_SetLayerOrder(g_DispHandle, LayerOrder, 4);
    if(l_ret != HI_SUCCESS) {
        ERR_OUT("[%s,%d]:HI_UNF_DISP_SetLayerOrder failed!\n", __func__, __LINE__);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
Err:
    return -1;
}

int yx_drv_display_logoclear()
{
    int l_ret;

    l_ret = HI_UNF_DISP_DisableLayer(g_DispHandle, HI_UNF_DISP_LAYER_OSD_2);
    if(l_ret != HI_SUCCESS) {
        ERR_OUT("[%s,%d]:HI_UNF_DISP_DisableLayer failed!\n", __func__, __LINE__);
        return HI_FAILURE;
    }

    return l_ret;
Err:
    return -1;
}

int yx_drv_display_logo_hide(void)
{
    int l_ret;

    l_ret = HI_UNF_DISP_DisableLayer(g_DispHandle, HI_UNF_DISP_LAYER_OSD_2);
    if(l_ret != HI_SUCCESS) {
        ERR_OUT("[%s,%d]:HI_UNF_DISP_DisableLayer failed!\n", __func__, __LINE__);
        return HI_FAILURE;
    }

    return l_ret;
Err:
    return -1;
}


/**
����ͼ��֮���˳��,������order�ĸ�λ��ʮλ����λ��ǧλ�ֱ��ʾ��Ƶ�㡢ͼ��0��ͼ��1������
��λ--��Ƶ��
ʮλ--ͼ��0
��λ--ͼ��1
ǧλ--����
0��ʾ�������ϲ�,3��ʾ�������²�,���磺
	order = 231��ʾ�����������棬��Ƶ��������֮�£�ͼ��0����Ƶ��֮�£�ͼ��2��������
*/
int yx_drv_display_layerOrder_set(int order)
{
    int l_ret = -1;
    int l_videoLayer = -1;
    int l_grpahicLayer0 = -1;
    int l_graphicLayer1 = -1;
    int l_mouseLayer = -1;
    HI_U32	LayerOrder[4];

    l_videoLayer = order % 10;
    order = order / 10;
    l_grpahicLayer0 = order % 10;
    order = order / 10;
    l_graphicLayer1 = order % 10;
    order = order / 10;
    l_mouseLayer = order % 10;

    //�����������higo�²������������������������⣬���ڴ˴������ж�

    LayerOrder[0] = l_videoLayer;//��Ƶ��
    LayerOrder[1] = l_grpahicLayer0;//ͼ�β�0
    LayerOrder[2] = l_graphicLayer1;//ͼ�β�1
    LayerOrder[3] = l_mouseLayer;//����

    l_ret = HI_UNF_DISP_SetLayerOrder(g_DispHandle, LayerOrder, 4);
    if(l_ret != HI_SUCCESS) {
        fprintf(stderr, "[%s,%d]:HI_UNF_DISP_SetLayerOrder failed!\n", __func__, __LINE__);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


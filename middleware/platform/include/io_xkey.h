/***************************************************************************
 *     Copyright (c) 2005-2009, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 *
 * $ys_Workfile: YX_gfx_porting.c $
 * $ys_Revision: 01 $
 * $ys_Date: 11/26/08 14:30 $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $ys_Log: YX_input_porting.c $
 *
 * 1. create by mabaoquan  11/26/2008
 *
 ***************************************************************************/
#ifndef __YX_IO_XKEY_H_H__
#define __YX_IO_XKEY_H_H__


typedef enum{
    IokeyType_eUnkown,
    IokeyType_eKeyDown,
    IokeyType_eKeyUp,
    IokeyType_eKeyPress
}iokey_type_e;


/**
 * @@brief param: key  : Ir key value.
 * @@brief param: type : Ir key mode, as down keypress or keyup
 * input event callback function;
 */
typedef void (*xkey_call)(unsigned int key, iokey_type_e type, int stat);

#ifdef __cplusplus
extern "C" {
#endif

/*get input irTpye for sham standby of jiangsu_SD hi3560e, 1 : USB Ir */
#if defined(hi3560e)
int io_input_IRtype(void);
#endif

void io_xkey_loop(void);
void io_xkey_reg(xkey_call call);

#ifdef __cplusplus
}
#endif

#endif

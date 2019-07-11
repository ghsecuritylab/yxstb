
/*-----------------------------------------------*/
/*
* Yuxing Software CONFIDENTIAL
* Copyright (c) 2003, 2011 Yuxing Corporation.  All rights reserved.
*
* The computer program contained herein contains proprietary information
* which is the property of Yuxing Software Co., Ltd.  The program may be used
* and/or copied only with the written permission of Yuxing Software Co., Ltd.
* or in accordance with the terms and conditions stipulated in the
* agreement/contract under which the programs have been supplied.
*
*    filename:			js_dlna.h
*    author(s):			yanyongmeng
*    version:			0.1
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------*/
#ifndef __MID_DLNA_EX_H__
#define __MID_DLNA_EX_H__

/* if the macro as below is defined, the middle ware must links dlna's so libs that are debug-version */
#ifdef DEBUG_BUILD
//#define DLNA_ENABLE_LOG
#endif
//#define  ENABLE_DLNA_TEST
/*  when debugging you can define the macro as below or not.  BUT MUST undef it before using svn to commit source code to trunk*/
//#define ENABLE_DLNA_TEST

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#ifdef ENABLE_DLNA_TEST
    #define DLNA_TEST_NONE				1						/*test normal app, e.g, Qetal */
    #define DLNA_TEST_OVERSEA			(DLNA_TEST_NONE+1)		/*test yuxing js */
    #define DLNA_TEST_SINGLE_FUNC		(DLNA_TEST_NONE+2)		/*test single function*/
    #define DLNA_TEST_HW_JS				(DLNA_TEST_NONE+4)		/*self-test huawei's js interface*/

    #define DLNA_TEST_MODE				DLNA_TEST_OVERSEA

    int mid_dlna_start_ex(int argc, char* argv[]);
	int mid_dlna_getlocalhostname( char *out );
    int mid_dlna_JsRead(const char * params,char * buf,int length);
    int mid_dlna_JsWrite(const char * params,char * buf,int length);
	int mid_dlna_test(unsigned int msgno, int type, int stat);
#endif

int mid_dlna_init(void);
int mid_dlna_start(int mode);
int mid_dlna_stop(void);
int mid_dlna_restart(void);
void mid_dlna_GetEvent(unsigned int msgno, int type, int stat);
#ifdef INCLUDE_DMS
int mid_dlna_start_dms(void);
int mid_dlna_stop_dms(void);
int mid_dlna_StartDms_AddPvr(void);
int mid_dlna_StopDms_RemovePvr(void);
int mid_dlna_ResumeDms(void);
int mid_dlna_PauseDms(void);
#endif

#define	DLNA_CFG_PATH		"/root/dlna/"

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*__MID_DLNA_EX_H__ */


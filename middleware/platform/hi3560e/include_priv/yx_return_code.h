/***************************************************************************
 *     Copyright (c) 2007-2008, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 * $Create_Date: 2008-4-2 19:37 $
 * Revision History:
 * 1. by SunnyLi  2008-4-2 19:38 create

 * $contact at lizhaohui@yu-xing.com
 ***************************************************************************/
#ifndef __YX_API_RETURN_CODE_H__
#define __YX_API_RETURN_CODE_H__

//abnormal return code
#define YX_RET_FAILURE             (-1)
#define YX_RET_INVALID_PARAMETER   (-2)
#define YX_RET_UNSUPPORTED         (-3)
#define YX_RET_NO_MEMALLOC         (-4)
#define YX_RET_BUFFEREMPTY         (-5)
#define YX_RET_TIME_OUT            (-6)
#define YX_RET_FILENOTFOUND        (-7)
#define YX_RET_ACCESSDENIED        (-8)
#define YX_RET_BUSY                (-9)
#define YX_RET_FILE_EOF            (-10)
#define YX_RET_NO_CONNECT          (-11)
#define YX_RET_NO_DATA		   (-12)

//normal return code
#define YX_RET_SUCCESS             (0)
#define YX_RET_DEVICE_BUSY         (1)
#define YX_RET_DEVICE_EMPTY        (2)
#define YX_RET_NO_ENOUGH_BUFFER    (3)

//The type for the return of all the functions .
typedef enum {
	//abnormal return code
	YX_RET_STATE_FAILURE = -1,
	YX_RET_STATE_INVALID_PARAMETER = -2,
	YX_RET_STATE_UNSUPPORTED  = -3,
	YX_RET_STATE_NO_MEMALLOC  = -4,
	YX_RET_STATE_BUFFEREMPTY  = -5,
	YX_RET_STATE_TIME_OUT     = -6,
	YX_RET_STATE_FILENOTFOUND = -7,
	YX_RET_STATE_ACCESSDENIED = -8,
	YX_RET_STATE_BUSY         = -9,
	YX_RET_STATE_FILE_EOF     = -10,
	YX_RET_STATE_NO_CONNECT   = -11,
	YX_RET_STATE_NO_DATA	  = -12,
	//normal return code
	YX_RET_STATE_SUCCESS      = 0,
	YX_RET_STATE_DEVICE_BUSY  = 1,
	YX_RET_STATE_DEVICE_EMPTY = 2,
	YX_RET_STATE_NO_ENOUGH_BUFFER = 3,
}yx_ret_state;


#endif /* __YX_TYPE_H__ */



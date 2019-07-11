/***************************************************************************
 *     Copyright (c) 2007-2008, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 * $Create_Date: 2008-4-2 19:37 $
 * Revision History:
 * 1. by SunnyLi  2008-4-2 19:38 create

 * $contact at lizhaohui@yu-xing.com
 ***************************************************************************/
#ifndef __YX_TYPE_H__
#define __YX_TYPE_H__

typedef unsigned char           YX_U8;
typedef unsigned char           YX_UCHAR;
typedef unsigned short          YX_U16;
typedef unsigned int            YX_U32;
typedef unsigned long long      YX_U64;
typedef	unsigned long           YX_L64;

typedef char                    YX_S8;
typedef short                   YX_S16;
typedef int                     YX_S32;
typedef long long               YX_S64;

typedef char                    YX_CHAR;
typedef char*                   YX_PCHAR;

typedef float                   YX_FLOAT;
typedef double                  YX_DOUBLE;
typedef void                    YX_VOID;

typedef unsigned long long      YX_PTS_TIME;
typedef unsigned long           YX_SIZE_T;
typedef unsigned long           YX_LENGTH_T;


typedef enum {
    YX_FALSE    = 0,
    YX_TRUE     = 1,
} YX_BOOL;

#ifndef FALSE
#define FALSE YX_FALSE
#endif

#ifndef TRUE
#define TRUE  YX_TRUE
#endif

#ifndef BOOL
#define BOOL  unsigned int
#endif

#ifndef NULL
#define NULL             0L
#endif

#define YX_NULL          0L
#define YX_NULL_PTR      0L

#define YX_SUCCESS       0
#define YX_FAILURE       (-1)

#define  YX_LITTLE_ENDIAN   1234       /* 字节序，小端模式 */
#define  YX_BIG_ENDIAN      4321       /* 字节序，大端模式 */

#ifndef OK
#define OK		(0)
#endif

#ifndef ERROR
#define ERROR		(-1)
#endif

#define YXEXTERN extern

#endif /* __YX_TYPE_H__ */



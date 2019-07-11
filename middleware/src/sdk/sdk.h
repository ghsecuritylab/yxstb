#ifndef _SDK_H_
#define _SDK_H_

#include "libzebra.h"

#if defined(brcm7405)

#define HYW_vout_setVideoAlpha(a, b) yhw_vout_setVideoAlpha(b)
#define HYW_vout_getVideoAlpha(a, b) yhw_vout_getVideoAlpha(b)

#elif defined(hi3560e)

#define HYW_vout_setVideoAlpha(a, b) -1
#define HYW_vout_getVideoAlpha(a, b) *b=0xff

#elif defined(hi3716m)

#define HYW_vout_setVideoAlpha(a, b) yhw_vout_setVideoAlpha(a, b)
#define HYW_vout_getVideoAlpha(a, b) yhw_vout_getVideoAlpha(a, b)

#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __cplusplus
}
#endif

#endif //_SDK_H_



#pragma once

#ifndef __TOOLS_H__
#define __TOOLS_H__

#ifdef __cplusplus
extern "C" {
#endif

// /var目录剩余空间大小。
unsigned long GetFreeVarSize(void);

// 判断文件是否存在.
int IsFileExists(const char * filename);

#ifdef __cplusplus
}
#endif

#endif

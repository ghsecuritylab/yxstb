#ifndef _SCBT_GLOBAL_H_
#define _SCBT_GLOBAL_H_

#define SCBT_DEBUG_TRACE_ON			1	/* 1 open debug trace, 0 close debug trace */

/* set it to 1 when build library for uboot */
#define SCBT_BUILD_LIBRARY_FOR_BOOTLOADER		0	/* 1 for build bootloader, 0 for build library*/

#ifdef __cplusplus
extern "C" {
#endif

#if SCBT_BUILD_LIBRARY_FOR_BOOTLOADER == 0 /*not in bootloader*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <limits.h>
#include <time.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/stat.h>
#endif /* not in bootloader */

#if SCBT_BUILD_LIBRARY_FOR_BOOTLOADER != 0 /* in bootloader */
#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "cfe_loader.h"
#include "cfe_fileops.h"
#endif /* in bootloader */



/* secure boot information section */
typedef struct _stSecureBootInfo_
{
	char abInfoMark[32];
	int wKernelDataLen;
	int wImageDataLen;
	int wRootfsDataLen;
	int wPaddingBytes;
	char abHashData[256];
}stSecureBootInfo;

int scbt_api_verify_upgrade_file( char* pSrcImagePath, int wKernelStartOffset);
int scbt_api_verify_image_data( char* pKernelStartAddress, char* pSignInfo);
int scbt_api_verify_kernel_rootfs_image_separate(char* pKernelStartAddress, char* pRootfsStartAddress);
int scbt_api_verify_app_data( char* pApp0StartAddress, char* pApp1StartAddress, stSecureBootInfo* pSignInfo);

int scbt_api_decrypt_upgrade_file( char* pImagePath );
int scbt_api_decrypt_image_data(unsigned char* in, unsigned char *out, unsigned int datalen);
int scbt_api_decrypt_without_upgrade_header_file( char* pSrcPath );
int scbt_api_verify_only_without_header_file( char* pSrcImagePath );
int scbt_api_verify_and_generate_without_header_file( char* pSrcImagePath, char* pDestImagePath );

#ifdef __cplusplus
}
#endif

#endif



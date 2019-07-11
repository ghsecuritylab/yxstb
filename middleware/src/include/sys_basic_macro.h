#ifndef __SYS_BASIC_MACRO_H__
#define __SYS_BASIC_MACRO_H__

/* 
 * 本文件中放置一些常量，可能各层都需要调用，如url长度，端口号等
 */

#define PORT_BASE_VALUE 50000
#define PORT_MAX_OFFSET 5000

#define URL_MAX_LEN 512
#define LARGE_URL_MAX_LEN 1024

#define AREAID_LEN 128
#define URL_LEN 512
#define NET_LEN 16
#define USER_LEN (32 + 2)
#define ENCRYPT_PASSWORD_LEN 130  // 128 + 2
#define STRING_LEN_64 64

#define CTC_SERVICEENTRY_MAX_URL 256
#define CTC_SERVICEENTRY_MAX_NUM 8
#define CTC_PASSWD_LEN 320

#define EPG_SERVICEENTRY_MAX_URL 256
#define EPG_SERVICEENTRY_MAX_NUM 8
#define EPG_PASSWD_LEN 320

#define SETTING_FILE_LEN 2048
#define ALLOWBANDWIDTH_LEN 8

#define SQA_FCC_CODE 0x08
#define SQA_RET_CODE 0x02
#define SQA_FEC_CODE 0x04
#define SQA_NOER 0x00

typedef long long int64;

#endif

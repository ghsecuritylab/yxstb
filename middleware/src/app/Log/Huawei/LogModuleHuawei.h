#ifndef _LogModuleHuawei_H_
#define _LogModuleHuawei_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum _HuaweiLogLevel {
	HLL_ALL         = 0,
	HLL_ERROR       = 3,
	HLL_INFORMATION = 6,
	HLL_DEBUG       = 7
} HuaweiLogLevel;

typedef enum _HuaweiLogType {
	HLT_ALL        = 0,
	HLT_OPERATION  = 16,
	HLT_RUNNING    = 17,
	HLT_SECURITY   = 19,
	HLT_USER       = 20
} HuaweiLogType;

typedef enum _HuaweiLogTransfersType {
	HLTT_NO   = 0x00,
	HLTT_FTP  = 0x01,
	HLTT_UDP  = 0x02,
	HLTT_HTTP = 0x04
} HuaweiLogTransfersType;

void huaweiSetLogLevel(int level);
int  huaweiGetLogLevel();
void huaweiSetLogType(int type);
int  huaweiGetLogType();
void huaweiSetLogOutPutType(int value);
int huaweiGetLogOutPutType();

void huaweiSetLogUDPServer(char*);
void huaweiGetLogUDPServer(char*, int);
void huaweiSetLogFTPServer(const char*);
void huaweiGetLogFTPServer(char*, int);
int huaweiSetLogHTTPServer(const char*);
int huaweiGetLogHTTPServer(char*, int);

void huaweiSetLogForceSend(void);

void huaweiLog();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _LogModuleHuawei_H_

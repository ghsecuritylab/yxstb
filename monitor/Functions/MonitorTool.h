#ifndef MonitorTool_h
#define MonitorTool_h

#include "MonitorDef.h"

typedef enum {
    DEBUG_INFO_NONE = 0,
    DEBUG_INFO_TOOL,
    DEBUG_INFO_DFX
}STBDebugInfoType;

typedef enum {
    DEBUG_INFO_EXIT = -1,
    DEBUG_INFO_START = 0,
    DEBUG_INFO_STOP,
    DEBUG_INFO_UPLOAD
}STBDebugInfoStatus;

typedef enum {
    STARTUP_INFO_EXIT = -1,
    STARTUP_INFO_START = 0,
    STARTUP_INFO_STOP,
    STARTUP_INFO_UPLOAD
}STBStartupInfoStatus;


extern "C" int stopStartupLog();
extern "C" int writeLog(FILE *logFd);
extern "C" int stopLog();
extern "C" int startupLog(FILE *logFd);
extern "C" int stopStartupLog();


int getDebugInfoStatus();

extern "C"
int monitorGetStartupInfo();
int monitorStopStartupInfo();
void* monitorGetStartupPacket(void* parm);
void* monitorCheckStartupStatus(void* parm);
int monitorCheckStartupSize();
int monitorGetIfaceIndex(int fd, const char* interfaceName);
int monitorGetPidByName(char* processName);
int monitorGetMacAddress(char* mac);
void monitorExecCmd(const char* command, const char* type, const char* pFileName, const char* mode);
int monitorUploadFiles(char* folderName, char* user, char* pswd, char* uploadAddress);
long long int monitorGetFolderSize(char* folderName);
int monitorStartDebugInfo(char* parameter);
int monitorStopDebugInfo(char* parameter);
int monitorUploadDebugInfo(char* upload_addr);
void* monitorCheckDebugInfoStatus(void* param);
void monitorTimerGetDebugInfo(int parameter);
void* monitorGetDebugInfoFiles(void* parm);
void monitorTimerDelDebugInfo(int parm);
void* monitorGetDebugInfoPacket(void* parm);
void monitorGetFirstUSBDisk(char* USBDiskPath);
int monitorSftpUpload(char* uploadFileName, char* uploadURL);
int monitorRunCommand(char *command);
void monitorTimerDelStartupInfo(int parm);
int monitorUploadStartupInfo(char *uploadAddr);
long long monitorGetUDiskFreeCapacity(char *UDiskPath);


//  Ô¶³Ì×¥°ü
int monitorRemoteCapture(moni_buf_t buf, int len);
void* monitorRemoteCapureFunc(void* arg);
void* monitorStartRemotePcap(void* arg);
void monitorSendRemoteCaptureFile(const char* filename);
void monitorGetSTBPositionIP(char *ip);

#endif//MonitorTool_h


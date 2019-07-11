
#include "LogModuleHuawei.h"

#include "Assertions.h"
#include "LogModule.h"
#include "LogPool.h"
#include "LogPostUDP.h"
#include "LogPostFTP.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern Hippo::LogPool* g_logPool;

static int g_level  = HLL_ERROR;
static int g_type   = 0; // only record for combining log format.
static int g_output = 1;

static int g_UDPServerPort = 37027;
static char g_UDPServer[256] = {0};
static char g_FTPServer[256] = {0};   //"ftp://user1:123456@110.1.1.142";
static Hippo::LogPostUDP* g_postUDP = 0;
static Hippo::LogPostFTP* g_postFTP = 0;

extern "C"
void logModuleCustomerInit()
{
    setModuleFlag("system", HLG_RUNNING);
    setModuleFlag("udisk", HLG_RUNNING);
    setModuleFlag("rtsp", HLG_RUNNING);
    setModuleFlag("nativeHandler", HLG_RUNNING);
    setModuleFlag("network", HLG_RUNNING);
    setModuleFlag("view", HLG_RUNNING);
    setModuleFlag("tvms", HLG_RUNNING);
    setModuleFlag("jseglue", HLG_RUNNING);
    setModuleFlag("JseCall", HLG_RUNNING);

    setModuleFlag("browser", HLG_OPERATION);
    setModuleFlag("program", HLG_OPERATION);
    setModuleFlag("player", HLG_OPERATION);
    setModuleFlag("sqa", HLG_OPERATION);
    setModuleFlag("sqm", HLG_OPERATION);
    setModuleFlag("upgrade", HLG_OPERATION);
    setModuleFlag("dvb", HLG_OPERATION);
    setModuleFlag("tr069", HLG_OPERATION);
    setModuleFlag("dlna", HLG_OPERATION);
    setModuleFlag("monitor", HLG_OPERATION);
    setModuleFlag("xmpp", HLG_OPERATION);

    setModuleFlag("tools", HLG_USER);

    /* For Huawei Test */
    setModuleFlag("hwOper", HLG_OPERATION);
    setModuleFlag("hwRun" , HLG_RUNNING);
    setModuleFlag("hwSafe", HLG_SECURITY);
    setModuleFlag("hwUser", HLG_USER);
}

extern "C"
void huaweiSetLogLevel(int level)
{
    printf("%s level[%d]\n", __FUNCTION__, level);
    g_level = level;
}

extern "C"
int huaweiGetLogLevel()
{
    return g_level;
}

extern "C"
void huaweiSetLogType(int type)
{
    printf("%s type[%d]\n", __FUNCTION__, type);
    g_type = type;
}

extern "C"
int huaweiGetLogType()
{
    return g_type;
}

extern "C"
void huaweiSetLogOutPutType(int value)
{
    printf("%s outputType[%d]\n", __FUNCTION__, value);
    g_output = value;
}

extern "C"
int huaweiGetLogOutPutType()
{
    return g_output;
}

extern "C"
void huaweiLog()
{
    logSetExtensionStyle(1);
    int logType = 0, logLevel = 0;

    /*
     *   类别码      类别        定义
     *     16      操作日志    操作是指系统用户（指包括系统管理员、操作维护员、系统监控员等）发起或者系统定时任务发起的指令，而操作日志则是对这些操作的记录。
     *     17      运行日志    记录系统运行状况或执行流程中的一些关键信息，这里系统指我们自己开发的软件应用。
     *     19      安全日志    记录系统用户（指包括系统管理员、操作维护员、系统监控员等）登录、注销和鉴权等活动。
     *      */
    switch (g_type) {
        case HLT_OPERATION: //16
            logType = HLG_OPERATION; //1
            break;
        case HLT_RUNNING: //17
            logType = HLG_RUNNING; //2
            break;
        case HLT_SECURITY: //19
            logType = HLG_SECURITY; //4
            break;
        case HLT_USER: //20
            logType = HLG_USER; //8
            break;
        case HLT_ALL: //15
        default:
            logType = HLG_ALL;
            break;
    }

    clearModulesLevel( );
    /*
     *  错误级别            含义
     *     3       Error:只输出有报错的日志
     *     6       Informational: 输出事务日志（本文中所有定义的类别消息）
     *     7       Debug: 输出所有日志，包括机顶盒的内部调试信息、浏览器的调试信息。
     *   */
    switch (g_level) {
        case HLL_ERROR: //3
            logLevel = LOG_LEVEL_ERROR; //1
            break;
        case HLL_INFORMATION: //6
            logLevel = LOG_LEVEL_NORMAL; //3
            // logLevel = LOG_LEVEL_WARNING; //2
            break;
        case HLL_DEBUG://7
            logLevel = LOG_LEVEL_NORMAL; //3
            break;
        case HLL_ALL: //0
        default:
            logLevel = LOG_LEVEL_VERBOSE; //4
            break;
    }
    printf("logType[%d] logLevel[%d] output[%d]\n", logType, logLevel, g_output);
    setModulesLevel(logType, logLevel);

    if (g_postUDP) {
        g_logPool->detachFilter(g_postUDP);
        delete g_postUDP;
        g_postUDP = 0;
    }
    if (g_postFTP) {
        g_logPool->detachFilter(g_postFTP);
        delete g_postFTP;
        g_postFTP = 0;
    }

#ifdef ANDROID
    // ANDROID已经做了日志上传之类的功能了，我们不需要再做一遍。
    return;
#endif

    switch (g_output) {
        case 1: // only send ftp log
            if (g_FTPServer[0]) {
                g_postFTP = new Hippo::LogPostFTP(g_FTPServer, "");
                if (g_postFTP)
                    g_logPool->attachFilter(g_postFTP, 0);
            }
            break;
        case 2: // only send realtime log
            if (g_UDPServer[0]) {
                g_postUDP = new Hippo::LogPostUDP(g_UDPServer, g_UDPServerPort);
                if (g_postUDP)
                    g_logPool->attachFilter(g_postUDP, 0);
            }
            break;
        case 3: // send both ftp and realtime log
            if (g_UDPServer[0]) {
                g_postUDP = new Hippo::LogPostUDP(g_UDPServer, g_UDPServerPort);
                if (g_postUDP)
                    g_logPool->attachFilter(g_postUDP, 0);
            }

            if (g_FTPServer[0]) {
                g_postFTP = new Hippo::LogPostFTP(g_FTPServer, "");
                if (g_postFTP)
                    g_logPool->attachFilter(g_postFTP, 0);
            }
            break;
        default: { //0 is close
                clearModulesLevel( );
                logSetExtensionStyle(0);
                g_UDPServer[0] = 0;
                g_FTPServer[0] = 0;
                g_level  = HLL_ERROR;
                g_type   = 0;
                g_output = 1;
            }
    }
}

extern "C"
void huaweiSetLogUDPServer(char* server)
{
    if (server) {
        printf("%s [%s]\n", __FUNCTION__, server);
        strncpy(g_UDPServer, server, sizeof(g_UDPServer));
        g_UDPServer[sizeof(g_UDPServer) - 1] = 0;
        char* p = strchr(g_UDPServer, ':');
        if (p) {
            *p++ = 0;
            g_UDPServerPort = atoi(p);
        }
    }
}

extern "C"
void huaweiGetLogUDPServer(char* server, int length)
{
    if (server && length > 0) {
        printf("%s\n", __FUNCTION__);
        strncpy(server, g_UDPServer, length);
        server[length - 1] = 0;
    }
}

extern "C"
void huaweiSetLogFTPServer(const char* server)
{
    if (server) {
        printf("%s [%s]\n", __FUNCTION__, server);
        strncpy(g_FTPServer, server, sizeof(g_FTPServer));
        g_FTPServer[sizeof(g_FTPServer) - 1] = 0;
    }
}

extern "C"
void huaweiGetLogFTPServer(char* server, int length)
{
    if (server && length > 0) {
        printf("%s\n", __FUNCTION__);
        strncpy(server, g_FTPServer, length);
        server[length -1] = 0;
    }
}

extern "C"
int huaweiSetLogHTTPServer(const char*)
{
    printf("%s\n", __FUNCTION__);
    return -1;
}

extern "C"
int huaweiGetLogHTTPServer(char*, int)
{
    printf("%s\n", __FUNCTION__);
    return -1;
}

extern "C"
void huaweiSetLogForceSend(void)
{
    /* Huawei: Must upload ftp log at once after stb standby or give up trying within 3 seconds. */
    if (g_postFTP)
        g_postFTP->beginUpload();

}

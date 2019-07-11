#include "DataRecive.h"
#include "RingBuffer.h"
#include "LogPool.h"
#include "LogPostTerminal.h"
#include "LogPostUDP.h"
#include "MonitorToolLog.h"
#include "MonitorLogPostUDP.h"
#include "MonitorLogPostFTP.h"
#include "nm_dbg.h"

using namespace android;
#define LOG_BUFFER_SIZE	128 * 1024
MonitorLog* g_logMonitor;
LogPool* logPool = NULL;

int g_level  = 3;
int g_type   = 0;
int g_output = 1;
int g_UDPServerPort = 514;
char g_UDPServer[256] = {0};
char g_FTPServer[256] = {0};   //"ftp://user1:123456@110.1.1.142";
android::MonitorLogPostUDP* g_postUDP = 0;
android::MonitorLogPostFTP* g_postFTP = 0;



int initLog()
{    
	uint8_t* logBuffer = NULL;
    RingBuffer* logringBuffer = NULL;
    
    LogPostTerminal* logPostTerminal = NULL;
    LogPostUDP* logPostUdp = NULL;
    DataRecive* datareceive = NULL;
	
    if (!(logBuffer = (uint8_t*)malloc(LOG_BUFFER_SIZE))){
        printf ("Error, Malloc Failed ...\n");
        return -1;
    }
    logringBuffer = new RingBuffer(logBuffer, LOG_BUFFER_SIZE);
    logPool = new LogPool();
    datareceive = new DataRecive();
    //logPostTerminal = new LogPostTerminal();
	g_logMonitor = new MonitorLog();

    logPool->setBuffer(logringBuffer);
	
    //logPool->attachFilter(logPostTerminal, 0);
	logPool->attachFilter(g_logMonitor, 0);
	
    datareceive->setBuffer(logringBuffer);
    
    datareceive->attachSink(logPool);

    datareceive->readLogLoop();

    free(logBuffer);
    return 0;
}
void *attachUdpLogFilter(char *server_ip, int port, int log_type, int log_level) //server format:ip:port
{
	nm_track();
	if(server_ip && port > 0){
	nm_track();
		if(logPool){
	nm_track();
			LogPostUDP *udpFilter = new LogPostUDP(server_ip, port);
			udpFilter->logTypeSet(log_type);
			udpFilter->logLevelSet(log_level);
			logPool->attachFilter(udpFilter, 0);
	nm_track();
			return udpFilter;
		}
	}
	return NULL;
}

int detachUdpLogFilter(void *filter)
{
	if(filter && logPool){
		LogPostUDP *LogFilter = (LogPostUDP* )filter;
		logPool->detachFilter(LogFilter);
		return 0;
	}
	return -1;
}



/************************UDP Address****************/
void monitorSetUDPLogAddress(char *server)
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

char* monitorGetUDPLogAddress()
{
	return g_UDPServer;
}

/************************FTP Address****************/
void monitorSetFTPLogAddress(char *server)
{
    if (server) {
        //printf("%s [%s]\n", __FUNCTION__, server);
        strncpy(g_FTPServer, server, sizeof(g_FTPServer));
        g_FTPServer[sizeof(g_FTPServer) - 1] = 0;
    }

}

char* monitorGetFTPLogAddress()
{
	return g_FTPServer;
}


/************************Log Level*****************/
void monitorSetLogLevel(int level)
{
    printf("%s level[%d]\n", __FUNCTION__, level);
    g_level = level;
}

int monitorGetLogLevel()
{
	
    return g_level;
}


/************************Log Type*********************/
void monitorSetLogType(int type)
{
    printf("%s type[%d]\n", __FUNCTION__, type);
    g_type = type;
}

int monitorGetLogType()
{
    return g_type;
}


/************************Log OutPut Type*****************/
void monitorSetLogOutPutType(int value)
{
    printf("%s outputType[%d]\n", __FUNCTION__, value);
    g_output = value;
}

int monitorGetLogOutPutType()
{
    return g_output;
}


void MonitorLog()
{
    printf("MonitorLog() in\n");

    if (g_postUDP && logPool) {
        logPool->detachFilter(g_postUDP);
        delete g_postUDP;
        g_postUDP = 0;
    }
    if (g_postFTP && logPool) {
        logPool->detachFilter(g_postFTP);
        delete g_postFTP;
        g_postFTP = 0;
    }
	
    //printf("server = %s  port = %d\n", g_UDPServer, g_UDPServerPort);
    switch (g_output) {
        case 1: // only send ftp log
            if (g_FTPServer[0]) {
                g_postFTP = new android::MonitorLogPostFTP(g_FTPServer, "", g_type, g_level);
                if (g_postFTP)
                    logPool->attachFilter(g_postFTP, 0);
            }
            break;
        case 2: // only send realtime log
            if (g_UDPServer[0]) {
                g_postUDP = new android::MonitorLogPostUDP(g_UDPServer, g_UDPServerPort, g_type, g_level);
                if (g_postUDP)
                    logPool->attachFilter(g_postUDP, 0);
            }
            break;
        case 3: // send both ftp and realtime log
            if (g_UDPServer[0]) {
                g_postUDP = new android::MonitorLogPostUDP(g_UDPServer, g_UDPServerPort, g_type, g_level);
                if (g_postUDP)
                    logPool->attachFilter(g_postUDP, 0);
            }

            if (g_FTPServer[0]) {
                g_postFTP = new android::MonitorLogPostFTP(g_FTPServer, "", g_type, g_level);
                if (g_postFTP)
                    logPool->attachFilter(g_postFTP, 0);
            }
            break;
        default: { //0 is close
                g_UDPServer[0] = 0;
                g_FTPServer[0] = 0;
                g_level  = 3;
                g_type   = 0;
                g_output = 1;
            }
    }
}


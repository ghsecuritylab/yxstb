#include <stdio.h>
#include <string.h>

#include "mid_stream.h"
#include "mid/mid_http.h"
#include "mid/mid_timer.h"
#include "app_c20_init.h"
//#include "KeyTableParser.h"
#include "app_heartbit.h"
#include "SysSetting.h"
#include "curl/curl.h"
#include "curl/easy.h"
#include "TAKIN_browser.h"
#include "TAKIN_setting_type.h"
#include "preconfig.h"
#include "Business.h"

#include "BrowserBridge/Huawei/BrowserEventQueue.h"


static int iptv_status_probe(void);
static int iptv_getHttps_CAaddress(void);

void app_stream_init_private_standard(void)
{
    mid_stream_standard(RTSP_STANDARD_HUAWEI);
    mid_stream_heartbit_period(60);
    mid_stream_nat(1);
    return ;
}

/*when enter dvb mode disable heartbit
* start to check the given address is reacheable*/
int app_enterDvbMode(void)
{
    httpHeartClr();
    iptv_status_probe();
    return 0;
}

/*when enter hybird mode enable heartbit*/
int app_enterHybirdMode(void)
{
    extern int KeyTableUrlGet();
    KeyTableUrlGet();  /*get key table*/
    BusinessSetEDSJoinFlag(1);
    iptv_getHttps_CAaddress();
    epgBrowserAgentSetTakinSettings(TAKIN_CA_PATH, "/root/ca.crt", strlen("/root/ca.crt"));
    return 0;
}

#define BUFFER_EVENT_MAX_COUNT EVENT_NUM_MAX
static char eventBuf[BUFFER_EVENT_MAX_COUNT][A2_INFO_LEN_MAX];
static int eventCount = 0;
/*when epg not ready buffer event ,return  0 not buffer, 1 buffer*/
int app_bufferEvent_C20(const char *pEventInfo)
{
    if (NULL == pEventInfo) {
        printf("ERROR: NULL == pEventInfo\n");
        return 0;//no deal with
    }

    if (BusinessGetEPGReadyFlag() == 0) {//epg not ready to get event
        printf("buffer event[%s]\n", pEventInfo);
        if (eventCount >=BUFFER_EVENT_MAX_COUNT) {
            printf("ERROR:eventCount >=BUFFER_EVENT_MAX_COUNT [%d]\n", BUFFER_EVENT_MAX_COUNT);
            return 0;
        }

        int len = strlen(pEventInfo);
        if (len<A2_INFO_LEN_MAX) {
            strncpy(eventBuf[eventCount], pEventInfo, len);
            eventCount++;
        }
        return 1;//buffered
    } else //epg ready
        return 0;
}

int app_epgReadyCallback_C20(void)
{
    int i = 0;
    while(eventCount > i) {
        browserEventSend(eventBuf[i], NULL);
        i++;
    }
    return 0;
}

int app_netConnectCallback_C20(void)
{
    //upgrade check

    //ntp sync
    //mid_ntp_time_sync();

    return 0;
}

/*check service and create check timer*/
static void checkServiceInDvb(int arg)
{
    char eds[512] = { 0 };
    char url[512] = { 0 };
    char *ip_port = NULL;
    CURL *m_curl;
    int i;
    CURLcode res;

    for (i = 0; i < 2; i++){
        memset( eds, 0, 512 );
        memset( url, 0, 512 );
		if( 0 == i)
			sysSettingGetString("eds", eds, 512, 0);
		else if(1 == i)
			sysSettingGetString("eds1", eds, 512, 0);
		else
			continue;

        printf("eds%d url : %s\n", i, eds );
        ip_port = strtok( eds+7, "/" );
        sprintf( url, "http://%s%s", ip_port, NETWORK_BREAK_IN);
        printf("detect url1 : %s\n", url );
        m_curl = curl_easy_init();

        if (!m_curl) {
            printf("curl init failed\n");
            return ;
        }
	#ifndef _lint
        curl_easy_setopt(m_curl, CURLOPT_URL, url);
        /* Do not do the transfer - only connect to host */
        curl_easy_setopt(m_curl, CURLOPT_CONNECT_ONLY, 1L);
	#endif
        res = curl_easy_perform(m_curl);

        if (CURLE_OK == res) {
            curl_easy_cleanup(m_curl);
            printf("Hybrid service status is success!\n");
            char a2_event[] = "{\"type\":\"EVENT_IPTVSYSTEM_STATUS_RESUME\",\" message\":\"IPTV System Resumed\"}";
            browserEventSend(a2_event, NULL);
            return ;
        }
        curl_easy_cleanup(m_curl);
    }
    printf("Hybrid service status is failed!\n");
    iptv_status_probe();
    return ;
}

/*when check time changed check service*/
void CheckServiceNow(void);
void CheckServiceNow(void)
{
    mid_timer_delete(checkServiceInDvb, 0);/*delete check timer*/
    checkServiceInDvb(0);/*check service and create check timer*/
}

static int iptv_status_probe(void)
{
#ifdef INCLUDE_DVBS
    int checkTime = 0;
	sysSettingGetInt("DvbServiceCheck", &checkTime, 0);

    mid_timer_create(checkTime, 1, checkServiceInDvb, 0);  /*create check timer*/
    return 0;
#else
    printf("not INCLUDE_DVBS  iptv_status_probe not start\n");
    return 0;
#endif
}


#define caAdressLen 256
static char caAddress[caAdressLen]  = {0};
char * getCadownloadAddress(void)
{
    return caAddress;
}

static void getHttpsCAaddress(int result, char* buf, int len, int arg)
{
    printf("getHttpsCAaddress buf[%s]\n", buf);
    if (len > caAdressLen) {
        printf("getHttpsCAaddress err \n");
    } else {
        memset(caAddress, 0, sizeof(caAddress));
        strncpy(caAddress, buf, len);
    }
}

static int iptv_getHttps_CAaddress(void)
{
    char url[256] = {0};
    char *urlport  = app_epgUrl_ip_port_get();
    sprintf(url, "%s/EPG/server/getRootCert.jsp",   urlport);
    printf("iptv_getHttps_CAaddress url[%s]\n", url );
    mid_http_simplecall( url, getHttpsCAaddress, 0);
    return 0;
}


#ifdef HWVERSION
char * HwSoftwareVersion(int type)
{
    if (type)
        return HWVERSION;
    else
        return "IPTV STB "HWVERSION;
}
#endif



#ifndef __APP_C20_INIT_H__
#define __APP_C20_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void app_stream_init_private_standard(void);
int app_enterDvbMode(void);
int app_enterHybirdMode(void);
int app_bufferEvent_C20(const char *pEventInfo);
void CheckServiceNow(void);
int app_epgReadyCallback_C20(void);
int app_netConnectCallback_C20(void);
char * getCadownloadAddress(void);


#define NETWORK_BREAK_IN "/EDS/jsp/headEndStatusCheck.jsp"

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

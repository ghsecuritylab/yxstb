#ifndef MonitorPing_h
#define MonitorPing_h

typedef enum {
    PING_RUN = 0,
    PING_STOP
} PING_STATUS;

typedef struct moni_buf* moni_buf_t;

void monitorPingFunc(moni_buf_t buf, char* string);
int monitorPingConnect(moni_buf_t buf, int len);
PING_STATUS monitorGetPingFlag(void);
void monitorSetPingRun(void);
void monitorSetPingStop(void);
int monitorGetPingMagic(void);
void monitorSetPingStopMagic(void);
int monitorStbMonitorPing(char* url, moni_buf_t buf, void* stb_func);
unsigned short monitorIpChecksum(unsigned short* pcheck, int check_len);

#endif//MonitorPing_h


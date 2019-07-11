#ifndef Tr069X_CTC_IPTV_Monitor_h
#define Tr069X_CTC_IPTV_Monitor_h

#include "Tr069GroupCall.h"

#ifdef __cplusplus

class Tr069X_CTC_IPTV_Monitor : public Tr069GroupCall {
public:
    Tr069X_CTC_IPTV_Monitor();
    ~Tr069X_CTC_IPTV_Monitor();
};

#ifdef TR069_MONITOR

#define MONITOR_POST_MAX 32
#define MONITOR_STATISTIC_FILE_SIZE 1024 * 64
#define MIN_PLAY_TIME 10

struct MonitorPost {
    int mdiMLR;
    int mdiDF;
    int jitter;

    int authNumbers;
    int authFailNumbers;
    int multiReqNumbers;
    int vodReqNumbers;
    int multiFailNumbers;
    int vodFailNumbers;
    int bufUnderFlowNumbers;
    int bufOverFlowNumbers;
};
struct IPTVMonitor {
    unsigned int enable;
    char timeList[16];
    char logUploadInterval[16];
    unsigned int isFileorRealTime;
    char parameterList[256];
};
struct MonitorList {
    int num;
    int list[MONITOR_POST_MAX];
};
struct PlayMonitor {
	int responseDelay;
	int channelSwitchDelay;
	char channelName[128];
	char channelAddress[1024];
	char transmission[16];
	unsigned int programStartTime;
	unsigned int programEndTime;
	int bitRate;
	char VideoQuality[1024];
	int ChannelRequestFrequency;
	int AccessSuccessNumber;
	char AverageAccessTime[16];
	int WatchLong;
	int MediaStreamBandwidth;

};

enum {
	FLAG_MULTIFAILNUM,
	FLAG_VODFAILNUM,
	FLAG_BUFINCNUM,
	FLAG_BUFDECNUM
};

enum {

    DEVICE_X_CTC_IPTV_Monitor_MdiMLR_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_MdiDF_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_Jitter_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_CPURate_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_MemRate_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_AuthNumbers_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_AuthFailNumbers_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_MultiReqNumbers_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_VodReqNumbers_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_MultiFailNumbers_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_VodFailNumbers_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_BufUnderFlowNumbers_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_BufOverFlowNumbers_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_ResponseDelay_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_ChannelSwitchDelay_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_ChannelName_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_ChannelAddress_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_Transmission_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_ProgramStartTime_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_ProgramEndTime_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_BitRate_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_VideoQuality_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_ChannelRequestFrequency_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_AccessSuccessNumber_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_AverageAccessTime_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_WatchLong_INDEX,
	DEVICE_X_CTC_IPTV_Monitor_MediaStreamBandwidth_INDEX,
};

enum {
	TYPE_VOD,
	TYPE_CHANNEL
};
#endif //TR069_MONITOR

#endif // __cplusplus

#ifdef TR069_MONITOR

#ifdef __cplusplus
extern "C" 
{
#endif
int app_monitor_statistic_config_init(void);
void monitor_cycle_statistics(int flag);
int monitor_post_play(int type, int delay, const char *channelName, const char *channelAddress);

int monitor_play_bitrate(int bitrate);

int app_tr069_port_Monitor_get_MdiMLR(void);
int app_tr069_port_Monitor_get_MdiDF(void);
int app_tr069_port_Monitor_get_Jitter(void);
int app_tr069_port_Monitor_get_BufUnderFlowNumbers(void);
int app_tr069_port_Monitor_get_BufOverFlowNumbers(void);
int app_tr069_port_Monitor_get_ResponseDelay(void);
int app_tr069_port_Monitor_get_ChannelSwitchDelay(void);
void app_tr069_port_Monitor_get_ChannelName(char *value, int size);
void app_tr069_port_Monitor_get_ChannelAddress(char *value, int size);
void app_tr069_port_Monitor_get_Transmission(char *value, int size);
void app_tr069_port_Monitor_get_ProgramStartTime(char *value, int size);
void app_tr069_port_Monitor_get_ProgramEndTime(char *value, int size);
int app_tr069_port_Monitor_get_BitRate(void);


void app_tr069_port_Monitor_get_VideoQuality(char *value, int size);
int app_tr069_port_Monitor_get_ChannelRequestFrequency(void);
int app_tr069_port_Monitor_get_AccessSuccessNumber(void);
void app_tr069_port_Monitor_get_AverageAccessTime(char *value, int size);
int app_tr069_port_Monitor_get_WatchLong(void);
int app_tr069_port_Monitor_get_MediaStreamBandwidth(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //#ifdef TR069_MONITOR

#endif // Tr069X_CTC_IPTV_Monitor_h


#ifndef app_tr069_alarm_h
#define app_tr069_alarm_h

#define 	SERIAL_SAMPLING_CYC				3

#ifdef TR069_MONITOR
#define   ALARM_TIME_LAPSE_CODE              "2070"
#define 	ALARM_FILE_ACCESS_CODE		      "3002"
#endif
#ifdef Sichuan
#define 	ALARM_CPU_CODE					  "2000"
#define	    ALARM_MEM_CODE					  "2001"
#define 	ALARM_DISK_FULL_CODE			  	"2002"
#define 	ALARM_DECODE_CODE				  "2003"
#define 	ALARM_DECRYPT_CODE			  	  "2004"
#define     ALARM_CUSHION_CODE                "2005"
#define 	ALARM_DROP_PACKAGE_CODE		      "2050"
#define   ALARM_DROP_FRAME_CODE            "2060"
#define   ALARM_TIME_LAPSE_CODE              "2070"
#define 	ALARM_EPG_ACCESS_CODE		 "3000"
#define 	ALARM_MEDIA_ACCESS_CODE		      "3001"
#define 	ALARM_FILE_ACCESS_CODE		      "3002"
#define 	ALARM_MEDIA_FORMAT_CODE		      "3003"
#else
#define 	ALARM_CPU_CODE					  "100103"
#define	    ALARM_MEM_CODE					  "100105"
#define 	ALARM_DISK_FULL_CODE			  "100104"
#define 	ALARM_DISK_WR_CODE			      "100102"
#define 	ALARM_DROP_PACKAGE_CODE		      "100106"
#define 	ALARM_DECODE_CODE				  "100108"
#define 	ALARM_CHANNEL_CODE			      "300104"
#define 	ALARM_UPGRAGE_CODE			      "300101"
#define 	ALARM_AUTHORIZE_CODE			  "300103"
#endif

enum{
	ALARM_RELIEVE = 0,
	ALARM_RINGING
};

#ifdef __cplusplus
extern "C" {
#endif

void app_setAlarmSwitch(char *str);
void app_setFramesLostAlarmValue(char *str);
void app_setPacketsLostAlarmValue(char *str);

int app_create_search_status(void);
int app_delete_search_status(void);
int app_report_upgrade_alarm(int discrcode);
int app_report_channel_alarm(int discrcode);
int app_report_decode_alarm(void);
int app_report_decrypt_alarm(void);
int app_report_authorize_alarm(int value);
int app_report_cushion_alarm(void);
int app_report_drop_frame_alarm(void);
int app_report_time_lapse_alarm(void);
int app_report_epg_access_alarm(void);
int app_report_media_access_alarm(void);
int app_report_media_format_alarm(void);
int app_report_file_access_alarm(void);
int app_clear_authorize_alarm(void);
int app_clear_decode_alarm(void);
int app_clear_decrypt_alarm(void);
int app_clear_cushion_alarm(void);
int app_clear_drop_frame_alarm(void);
int app_clear_time_lapse_alarm(void);
int app_clear_epg_access_alarm(void);
int app_clear_media_access_alarm(void);
int app_clear_file_access_alarm(void);
int app_clear_media_format_alarm(void);
void app_reportPacketsLost(int permille);
int app_get_framelos_vaue(int * lower_limit, int * upper_limit);
int app_get_packet_alarm_enable(void);
int app_report_disk_offline_alarm(void);

int tr069_port_get_CPUrate(void);
int tr069_port_get_memValue(void);

#ifdef __cplusplus
}
#endif

#endif//app_tr069_alarm_h


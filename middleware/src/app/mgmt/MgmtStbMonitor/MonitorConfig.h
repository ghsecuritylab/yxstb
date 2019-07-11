#ifndef MonitorConfig_h
#define MonitorConfig_h

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


int checkMonitorInfo(const char*, const char*);
void saveMonitorInfo(const char*, const char*);
void resetMonitorInfo(void);
void getMonitorInfo(char*, int, char*, int);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MonitorConfig_h


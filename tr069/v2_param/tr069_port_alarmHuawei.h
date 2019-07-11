
#ifndef __tr069_port_alarmHuawei_h__
#define __tr069_port_alarmHuawei_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagAlarmBody AlarmBody;

struct tagAlarmBody {
    unsigned int AlarmSN;
    char *AlarmObjectInstance;
    char *AlarmLocation;
    unsigned int AlarmCode;
    unsigned int AlarmRaisedTime;
    unsigned int AlarmClearedTime;
    int PerceivedSeverity;
    char *AdditionalInformation;
};

int alarmBodyGet(AlarmBody *alarm, char *name, char *str, unsigned int size);
void alarmBodyClear(AlarmBody *alarm);
void alarmBodyFree(AlarmBody *alarm);
void alarmBodyPost(AlarmBody *alarm, int type, int code, int level, char *location);

#ifdef __cplusplus
}
#endif

#endif //__tr069_port_alarmHuawei_h__

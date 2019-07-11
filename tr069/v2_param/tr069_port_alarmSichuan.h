
#ifndef __tr069_port_alarmSichuan_h__
#define __tr069_port_alarmSichuan_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagAlarmBody AlarmBody;

struct tagAlarmBody {
    unsigned int AlarmID;
    unsigned int AlarmType;
    unsigned int AlarmLevel;
    unsigned int AlarmReason;
    unsigned int AlarmTime;
    char  *AlarmDetail;
};

int alarmBodyGet(AlarmBody *alarm, char *name, char *str, unsigned int size);
void alarmBodyClear(AlarmBody *alarm);
void alarmBodyFree(AlarmBody *alarm);
void alarmBodyPost(AlarmBody *alarm, int type, int code, int level, char *location);

#ifdef __cplusplus
}
#endif

#endif //__tr069_port_alarmSichuan_h__

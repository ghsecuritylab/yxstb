#ifndef StatisticRoot_h
#define StatisticRoot_h

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void tr069StatisticStart(void);
void tr069StaticticPeriodRestart();
void tr069StatisticPeriodStart(int arg);

void tr069StatisticStore(void);
int tr069StatisticConfigInit(void);
void tr069StatisticConfigReset(void);
void tr069StatisticConfigSave(void);

void tr069StatisticPost(int arg);
void tr069StaticticStandy();

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // StatisticRoot_h

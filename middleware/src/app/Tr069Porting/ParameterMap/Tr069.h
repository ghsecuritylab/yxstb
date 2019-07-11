#ifndef Tr069_h
#define Tr069_h

#ifdef INCLUDE_TR069

#ifdef __cplusplus
extern "C" {
#endif

int tr069_get_upgradeUrl(char *url, int len);
int tr069_set_UpgradeUrl(char *url);

#ifdef TR069_ZERO_CONFIG
void tr069_set_bootStrap(int bootStrap);
int tr069_get_bootStrap(void);
#endif //TR069_ZERO_CONFIG


void tr069_port_reset(void);
int tr069_port_setValue(char *name, char *str, unsigned int val);
int tr069_port_getValue(char *name, char *str, unsigned int val);

#ifdef __cplusplus
}
#endif

#include "tr069Itms.h"
#include "Tr069Root.h"
#include "TR069Assertions.h"
#include "tr069_port.h"
#include "tr069_port1.h"
#include "Tr069Setting.h"
#include "tr069_api.h"
#include "tr069_define.h"
#include "app_tr069_alarm.h"
#include "StatisticRoot.h"
#include "StatisticLog.h"
#include "StatisticBase.h"
#include "Tr069PlayDiagnostics.h"
#include "Tr069PacketCapture.h"

void Tr069ApiInit();


#define TR069_PORT_SETVALUE(name, value, flag)             tr069_port_setValue(name, value, flag)
#define TR069_PORT_GETVALUE(name, value, flag)             tr069_port_getValue(name, value, flag)

#define TR069_API_INIT( )                                  Tr069ApiInit( )
#define TR069_API_SETVALUE(name, value, flag)              tr069_api_setValue(name, value, flag)
#define TR069_API_GETVALUE(name, value, flag)              tr069_api_getValue(name, value, flag)

#define TR069_STATISTIC_START()                            tr069StatisticStart()
#define TR069_STATISTIC_CONFIG_RESET()                     tr069StatisticConfigReset()
#define TR069_STATISTIC_CONFIG_SAVE()                      tr069StatisticConfigSave()
#define TR069_STATISTIC_PERIOD_RESTART()                   tr069StaticticPeriodRestart()

#define TR069_STATISTIC_GET_CONFIGURATION(name, str, size) tr069_statistic_getConfiguration(name, str, size)
#define TR069_STATISTIC_SET_CONFIGURATION(name, str, size) tr069_statistic_setConfiguration(name, str, size)
#define TR069_STATISTIC_GET_LOGSERVERURL(value, size)      tr069StatisticGetLogServerUrl(value, size)
#define TR069_GET_UPGRADE_URL(url, size)                   tr069_get_upgradeUrl(url, size)
#define TR069_DIAGNOSTICS_SET_STATE(d_state)               tr069_diagnostics_set_state_by_enum(d_state)

#define TR069_LOG_POST(tm, type, info)                     tr069LogMsgPost(tm, type, info)
#define TR069_ROOT_INIT()                                  Tr069RootInit()

#define TR069_REPORT_CHANNEL_ALARM(msg)                    app_report_channel_alarm(msg)
#define TR069_REPORT_AUTHORIZE_ALARM(msg)                  app_report_authorize_alarm(msg)


#else // INCLUDE_TR069

#define TR069_PORT_SETVALUE(name, value, flag)
#define TR069_PORT_GETVALUE(name, value, flag)

#define TR069_API_INIT()
#define TR069_API_SETVALUE(name, value, flag)
#define TR069_API_GETVALUE(name, value, flag)

#define TR069_DIAGNOSTICS_SET_STATE()
#define TR069_LOG_POST(tm, type, info)
#define TR069_ROOT_INIT()
#define TR069_GET_UPGRADE_URL(url, size)

#define TR069_STATISTIC_START()
#define TR069_STATISTIC_CONFIG_RESET()
#define TR069_STATISTIC_CONFIG_SAVE()
#define TR069_STATISTIC_GET_CONFIGURATION(name, str, size)
#define TR069_STATISTIC_SET_CONFIGURATION(name, str, size)
#define TR069_STATISTIC_GET_LOGSERVERURL(value, size)
#define TR069_STATISTIC_PERIOD_RESTART()

#define TR069_REPORT_CHANNEL_ALARM(msg)
#define TR069_REPORT_AUTHORIZE_ALARM(msg)


#endif // INCLUDE_TR069

#endif // Tr069_h



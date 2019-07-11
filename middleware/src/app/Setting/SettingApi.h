#ifndef SettingApi_h
#define SettingApi_h

#include "ind_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

int settingConfigRead(CfgTree_t tree, char *rootname);
int settingConfigWrite(CfgTree_t tree, char *rootname);

#ifdef __cplusplus
}
#endif
#endif //SettingApi_h
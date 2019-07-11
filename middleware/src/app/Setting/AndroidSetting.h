#ifndef AndroidSetting_h
#define AndroidSetting_h

#include "SettingListener.h"
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

int androidSettingGetString(const char* name, char* value, int valueLen);
int androidSettingGetInt(const char* name, int* value);
int androidSettingSetString(const char* name, const char* value);
int androidSettingSetInt(const char* name, const int value);

#ifdef __cplusplus
}
#endif //__cplusplus

//defined in AndroidSettingJNI.cpp
int setAndroidSetting(const char* name, const char* vlaue);
int getAndroidSetting(const char* name, char* value, int len);

#endif //AndroidSetting_h

#ifndef _AppSetting_H_
#define _AppSetting_H_

#ifdef __cplusplus

#include "Setting.h"

namespace Hippo {

class AppSetting : public Setting {
public:
	AppSetting(std::string fileName);
	virtual ~AppSetting();

    int recoverBakSeting();
};

AppSetting& appSetting();

} // namespace Hippo
#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif
// searchFlag : if need to search from system setting
int appSettingGetString(const char* name, char* value, int valueLen, int searchFlag);
int appSettingSetString(const char* name, const char* value);

int appSettingGetInt(const char* name, int* value, int searchFlag);
int appSettingSetInt(const char* name, const int value);
#if defined(CACHE_CONFIG)
int appSettingSetStringForce(const char* name, const char* value, const int force);
int appSettingSetIntForce(const char* name, const int value, const int force);
#endif

#ifdef __cplusplus
}
#endif

#endif // _AppSetting_H_

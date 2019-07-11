#ifndef _SysSetting_H_
#define _SysSetting_H_

#ifdef __cplusplus

#include "Setting.h"

namespace Hippo {

class SysSetting : public Setting {
public:
	SysSetting(std::string fileName);
	virtual ~SysSetting();

    int recoverBakSeting();
};

SysSetting& sysSetting();

} // namespace Hippo
#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif

// searchFlag : if need to search from app setting
int sysSettingGetString(const char* name, char* value, int valueLen, int searchFlag);
int sysSettingSetString(const char* name, const char* value);

int sysSettingGetInt(const char* name, int* value, int searchFlag);
int sysSettingSetInt(const char* name, const int value);

#if defined(CACHE_CONFIG)
int sysSettingSetStringForce(const char* name, const char* value, const int force);
int sysSettingSetIntForce(const char* name, const int value, const int force);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SysSetting_H_

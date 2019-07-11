#ifndef _CacheSetting_H_
#define _CacheSetting_H_

#ifdef __cplusplus

#include "Setting.h"

namespace Hippo {

class CacheSetting : public Setting {
public:
    CacheSetting(std::string fileName);
    virtual ~CacheSetting();
};

CacheSetting& cacheSetting();

} // namespace Hippo
#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif

// searchFlag : if need to search from CacheConfigFile setting  
int cacheSettingGetString(const char* name, char* value, int valueLen, int searchFlag);
int cacheSettingSetString(const char* name, const char* value);

int cacheSettingGetInt(const char* name, int* value, int searchFlag);
int cacheSettingSetInt(const char* name, const int value);
int cacheSettingLoad();

#ifdef __cplusplus
}
#endif

#endif // _CacheSetting_H_


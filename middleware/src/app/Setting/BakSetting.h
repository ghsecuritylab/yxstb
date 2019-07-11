#ifndef BakSetting_h
#define BakSetting_h

#ifdef __cplusplus

#include "Setting.h"

namespace Hippo {

class BakSetting : public Setting {
public:
	BakSetting(std::string fileName);
	virtual ~BakSetting();

    int load();
    int backup();
    int remove();
    int recoverBakSeting();
};

BakSetting& bakSetting();

} // namespace Hippo

int bakSettingGetString(const char* name, char* value, int valueLen, int searchFlag);
int bakSettingSetString(const char* name, const char* value);

void bakListenRegist();

#endif //__cplusplus
#endif // _BakSetting_H_

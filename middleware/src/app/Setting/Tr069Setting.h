#ifndef _Tr069Setting_H_
#define _Tr069Setting_H_

#ifdef __cplusplus

#include "Setting.h"

namespace Hippo {

class Tr069Setting : public Setting {
public:
	Tr069Setting(std::string fileName);
	virtual ~Tr069Setting();
	
	virtual int load();
	virtual int restore(int flag);


};

Tr069Setting& tr069Setting();

} // namespace Hippo
#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif

// searchFlag : if need to search from tr069ConfigFile setting  
int tr069SettingGetString(const char* name, char* value, int valueLen, int searchFlag);
int tr069SettingSetString(const char* name, const char* value);

int tr069SettingGetInt(const char* name, int* value, int searchFlag);
int tr069SettingSetInt(const char* name, const int value);
int tr069SettingSave();

#ifdef __cplusplus
}
#endif

#endif // _Tr069Setting_H_

#ifndef _SettingModuleTr069_H_
#define _SettingModuleTr069_H_

#ifdef __cplusplus

#include "SettingModule.h"

namespace Hippo {

class SettingModuleTr069 : public SettingModule {
public:
	SettingModuleTr069();
	virtual ~SettingModuleTr069();
	
	virtual int settingModuleRegister();

};

} // namespace Hippo
#endif //__cplusplus

extern "C" int settingTr069();

#endif // _SettingModuleTr069_H_


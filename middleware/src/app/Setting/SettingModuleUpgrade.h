#ifndef _SettingModuleUpgrade_H_
#define _SettingModuleUpgrade_H_

#ifdef __cplusplus

#include "SettingModule.h"

namespace Hippo {

class SettingModuleUpgrade : public SettingModule {
public:
	SettingModuleUpgrade();
	virtual ~SettingModuleUpgrade();
	
	virtual int settingModuleRegister();

};

} // namespace Hippo
#endif //__cplusplus

extern "C" int settingUpgrade();

#endif // _SettingModuleUpgrade_H_


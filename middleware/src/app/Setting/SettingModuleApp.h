#ifndef _SettingModuleApp_H_
#define _SettingModuleApp_H_

#ifdef __cplusplus

#include "SettingModule.h"

namespace Hippo {

class SettingModuleApp : public SettingModule {
public:
	SettingModuleApp();
	virtual ~SettingModuleApp();
	
	virtual int settingModuleRegister();

};

} // namespace Hippo
#endif //__cplusplus

extern "C" int settingApp();

#endif // _SettingModuleApp_H_


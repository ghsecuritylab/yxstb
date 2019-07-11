#ifndef _SettingModuleDvb_H_
#define _SettingModuleDvb_H_

#ifdef __cplusplus

#include "SettingModule.h"

namespace Hippo {

class SettingModuleDvb : public SettingModule {
public:
	SettingModuleDvb();
	~SettingModuleDvb();
	
	virtual int settingModuleRegister();

};

} // namespace Hippo
#endif //__cplusplus

extern "C" int settingDvb();

#endif // _SettingModuleDvb_H_

#ifndef _SettingModulePlayer_H_
#define _SettingModulePlayer_H_

#ifdef __cplusplus

#include "SettingModule.h"

namespace Hippo {

class SettingModulePlayer : public SettingModule {
public:
	SettingModulePlayer();
	virtual ~SettingModulePlayer();
	
	virtual int settingModuleRegister();

};

} // namespace Hippo
#endif //__cplusplus

extern "C" int settingPlayer();

#endif // _SettingModulePlayer_H_


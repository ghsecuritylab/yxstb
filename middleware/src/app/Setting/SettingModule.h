#ifndef _SettingModule_H_
#define _SettingModule_H_

#ifdef __cplusplus

namespace Hippo {

class SettingModule {
public:
	SettingModule();
	virtual ~SettingModule();

    virtual int settingModuleRegister() = 0;

	SettingModule*  m_next;
};

} // namespace Hippo

#endif //__cplusplus

extern "C" void settingModuleResgister();

#endif // _SettingModule_H_


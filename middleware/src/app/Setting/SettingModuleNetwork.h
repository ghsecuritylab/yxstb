#ifndef _SettingModuleNetwork_H_
#define _SettingModuleNetwork_H_

enum {
    NETTYPE_CLOSE = 0, // close ipv4 connect
	NETTYPE_PPPOE = 1, // PPPoE
	NETTYPE_DHCP = 2, // special for DLNA shared ip with IPTV
	NETTYPE_STATIC = 3, // Static
	NETTYPE_DHCP_ENCRYPT = 4, // DHCP with option 60 and 125 check
#ifdef ENABLE_DBLVLAN
    NETTYPE_DHCP_DBLVLAN = 5, // DHCP with double vlan
#endif
    NETTYPE_DBL_STACK = 6 // PPPoE and DHCP one-by-one, write here for definition only
};

#ifdef __cplusplus

#include "SettingModule.h"

namespace Hippo {

class SettingModuleNetwork : public SettingModule {
public:
	SettingModuleNetwork();
	virtual ~SettingModuleNetwork();
	
	virtual int settingModuleRegister();

};

} // namespace Hippo
#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif

int settingNetwork();

#ifdef __cplusplus
}
#endif

#endif // _SettingModuleNetwork_H_


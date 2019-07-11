#include <stdio.h>
#include <string.h>

#include "tr069_interface.h"
#include "nm_common.h"
#include "nm_dbg.h"

#include "aesCrypto.h"

tr069_platform_get p_get = NULL;
tr069_platform_set p_set = NULL;

static int gIsEncryptMark = 2;

void tr069_platform_callback_set(tr069_platform_get platform_get, tr069_platform_set platform_set)
{
	p_get = platform_get;
	p_set = platform_set;
	return;
}

int tr069_port_getValue(char *name, char *str, int str_len)
{
	str[0] = 0;

	if (name == NULL) {
		nm_msg("tr069_port_getValue() name is NULL! error\n");
		return -1;
	}
    //X_00E0FC
	if(!strncmp(name, "Device.X_00E0FC.", 16)) {
	    char *xname = name + 16;
	    if (!strcmp(xname, "IsEncryptMark")) {
	        sprintf(str, "%d", gIsEncryptMark);
	        return 0;
	    }
	    if (!strcmp(xname, "EncryptParameters")) {
	        sprintf(str, "%s",
	            "Device.X_00E0FC.ServiceInfo.DHCPPassword,"
	            "Device.X_00E0FC.ServiceInfo.PPPoEPassword,"
	            "Device.X_00E0FC.ServiceInfo.UserIDPassword,"
	            "Device.X_00E0FC.StatisticConfiguration.LogServerUrl,"
	            "Device.X_00E0FC.PacketCapture.Password,"
	            "Device.X_00E0FC.BandwidthDiagnostics.Password,"
	            "Device.X_00E0FC.LogParaConfiguration.LogFtpServer,"
                "Device.X_CU_STB.STBInfo.AdministratorPassword,"
                "Device.X_CU_STB.STBInfo.UserPassword,"
                "Device.X_CU_STB.AuthServiceInfo.PPPOEPassword,"
                "Device.X_CU_STB.AuthServiceInfo.UserIDPassword,"  
                "Device.X_CU_STB.AuthServiceInfo.PPPOEPassword2," 
                "Device.X_CU_STB.AuthServiceInfo.UserIDPassword2"  
	        );
	        return 0;
	    }
	    if (gIsEncryptMark) {
	        if(!strncmp(xname, "ServiceInfo.", 12)) {
	            xname += 12;
    	        if (!strcmp(xname, "DHCPPassword")
    	            || !strcmp(xname, "PPPoEPassword")
    	            || !strcmp(xname, "UserIDPassword"))
    	            return 0;
    	    }
    	    if (!strcmp(xname, "PacketCapture.Password")
    	        || !strcmp(xname, "BandwidthDiagnostics.Password"))
    	        return 0;
	    }
	}
    //X_CU_STB
	if(!strncmp(name, "Device.X_CU_STB.", 16)) {
	    char *xname = name + 16;
	    if (gIsEncryptMark) {
            if (!strncmp(xname, "STBInfo.", 8)) {
                xname += 8;
                if (!strcmp(xname, "AdministratorPassword") 
                    || !strcmp(xname, "UserPassword"))
                    return 0;
            } else if (!strncmp(xname, "AuthServiceInfo.", 16)) {
                xname += 16;
                if (!strcmp(xname, "PPPOEPassword") 
                    || !strcmp(xname, "UserIDPassword")
                    || !strcmp(xname, "PPPOEPassword2")
                    || !strcmp(xname, "UserIDPassword2"))
                    return 0;
            }
        }
    }

	//nm_msg_level(LOG_DEBUG, "tr069_port_getValue() name = %s\n", name);
	if(p_get) {
		return p_get(name, str, str_len);
	} else
		nm_msg("platform getvalue callback is NULL\n");				
	return -1;
	
}

static int inline in_set_value(char *name, char *str, int pval)
{
	if(p_set){
		return p_set(name, str, pval);
	} else
		nm_msg("platform setvalue callback is NULL\n");			
	return -1;
}

int tr069_port_setValue(char *name, char *str, int pval)
{
	if (name == NULL) {
		nm_msg("tr069_port_setValue() name is NULL! error\n");
		return -1;
	}
	//nm_msg_level(LOG_DEBUG, "tr069_port_setValue() name = %s\n", name);
	if(str){
		//nm_msg_level(LOG_DEBUG, "tr069_port_setValue() value = %s\n", str);
	}
    if(gIsEncryptMark) {
        char tmp_str[1024];
        char *xname;

        xname = name + 7;

        if (!strncmp(xname, "ManagementServer.", 17)) {
            xname += 17;

            if (2 == gIsEncryptMark) {
                if (!strcmp(xname, "Password") || !strcmp(xname, "ConnectionRequestPassword") || !strcmp(xname, "STUNPassword")) {
                    decryptACSCiphertext(str, tmp_str);
                    strcpy(str, tmp_str);
                }
            }
            return in_set_value(name, str, pval);
        }

        if(!strncmp(xname, "X_00E0FC.", 9)) {
            xname += 9;

            if((2 == gIsEncryptMark && !strcmp(xname, "ServiceInfo.DHCPPassword"))
                || !strcmp(xname, "ServiceInfo.PPPoEPassword")
                || !strcmp(xname, "ServiceInfo.UserIDPassword")) {
                decryptACSCiphertext(str, tmp_str);
                return in_set_value(name, tmp_str, pval);
            }
		} else if (!strncmp(name, "X_CU_STB.", 9)) {
	        xname += 9;

            if (!strncmp(xname, "STBInfo.", 8)) {
                xname += 8;
                if (!strcmp(xname, "AdministratorPassword") 
                    || !strcmp(xname, "UserPassword")) {
                    decryptACSCiphertext(str, tmp_str);
                    return in_set_value(name, tmp_str, pval);
                }
            } else if (!strncmp(xname, "AuthServiceInfo.", 16)) {
                xname += 16;
                if (!strcmp(xname, "PPPOEPassword") 
                    || !strcmp(xname, "UserIDPassword")
                    || !strcmp(xname, "PPPOEPassword2")
                    || !strcmp(xname, "UserIDPassword2"))
                    decryptACSCiphertext(str, tmp_str);
                    return in_set_value(name, tmp_str, pval);
            }
        }
	}
	return in_set_value(name, str, pval);
}

int tr069_start(acs_info_t *info)
{
	if(!info){
		nm_msg("acs info is invalid\n");
		return -1;
	}
	tr069_platform_callback_set(android_platform_getValue, android_platform_setValue);
	tr069_api_setValue("Config.ParamPath", PARAM_FILE_PATH"tr069_param.xml", 0);
	tr069_api_setValue("Config.ConfigPath", "/data/yx_config_tr069.cfg", 0);	

	tr069_api_init();
	tr069_acs_set(info->url, info->user_name, info->user_pass, info->peridic_enable, info->peridic_interval, info->connect_user_name, info->connect_user_pass);

    if (info->UpgradeFailFlag >= 0) {
        if (info->UpgradeFailFlag) {
            tr069_api_setValue("AlarmPost.0.300101.2", "Firmware Signature Check Failed", 0);
            tr069_api_setValue("Message.Download.1 Firmware Upgrade Image", "", 9010);
        } else {
            tr069_api_setValue("Message.Download.1 Firmware Upgrade Image", "", 0);
        }
    }

    tr069_api_setValue("Task.Active", NULL, 1);
	return 0;
}

int tr069_resume()
{
	tr069_api_setValue("Task.Active", NULL, 1);
	return 0;
}

int tr069_suspend()
{
	tr069_api_setValue((char*)"Task.Suspend", NULL, 1);
	return 0;
}

int tr069_acs_set(char *url, char *user_name, char *user_pass, char *periodic_enable, char * peridic_interval, char* cpe_name, char *cpe_pass)
{
	nm_msg_level(LOG_DEBUG, "tr069_acs_set() \n");

	tr069_api_setValue("Device.ManagementServer.URL", url, 0); 
	tr069_api_setValue("Device.ManagementServer.Username", user_name, 0); 
	tr069_api_setValue("Device.ManagementServer.Password", user_pass, 0); 
	tr069_api_setValue("Device.ManagementServer.PeriodicInformEnable", periodic_enable, 0); 
	tr069_api_setValue("Device.ManagementServer.PeriodicInformInterval", peridic_interval, 0); 
	tr069_api_setValue("Device.ManagementServer.ConnectionRequestUsername", cpe_name, 0); 
	tr069_api_setValue("Device.ManagementServer.ConnectionRequestPassword", cpe_pass, 0); 

	return 0;
}

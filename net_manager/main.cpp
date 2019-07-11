#include <stdio.h>
#include <string>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include "tr069_interface.h"
#include "HybroadService.h"
#include "json_public.h"
#include "json_object.h"
#include "common/mid_timer.h"
#include "nm_common.h"
#include "param_map.h"

#include "nm_dbg.h"

using namespace android;
char acsPassword[128] = {0};
extern HybroadService *pHybroad;
extern void service_handle_set(HybroadService *pservice);
extern int stbMonitorInit(char *XMLFilePath);
extern int initLog();
void* tr069_func(void* parm);
void *stbMonitor_func(void *parm);
void tr069_log_param_init();

int getAcsPassword(char * value)
{
    strcpy(value, acsPassword);
    return 0;
}
int setAcsPassword(const char *value)
{
    strcpy(acsPassword, value);
    return 0;
}
const char *get_json_value(struct json_object *object, const char *param_name)
{
	struct json_object *in_obj = NULL;
	
	in_obj = json_object_object_get(object, param_name);
	if (!in_obj){
		return NULL;
	}
	return json_object_get_string(in_obj);
}

void* tr069_func(void* parm)
{	
	char server_info[1024 + 1] = {0};
	
	char hw_version[128 + 1] = {0};
	char hw_additional_version[128 + 1] = {0};
	char hw_mac[128 + 1] = {0};
	char ip_addr[64 + 1] = {0};
	char tr069_enable[8 + 1] = {0};
	struct acs_info acs_config_info;
	
	nm_msg_level(LOG_DEBUG, "run tr069...\n");
	nm_msg_level(LOG_DEBUG, "start tr069\n");
	while(1){
		if(pHybroad->module_is_registered("setting")){
			nm_msg_level(LOG_DEBUG, "setting is registered,start tr069\n");
			break;
		}
		usleep(1000 * 500);
	}
	pHybroad->GetValue((char*)"Tr069.Config.Switch", tr069_enable, 8);
	if(tr069_enable[0] == '0'){ //disable
		nm_msg("tr069 is disabled\n");
		return NULL;
	} else {
		nm_msg_level(LOG_DEBUG, "tr069 is enabled: %s\n", tr069_enable);
	}

    {
        char upgradeVersion[128];
        upgradeVersion[0] = 0;
        pHybroad->GetValue((char*)"failtoupgrade_netmanage", upgradeVersion, 128);
        pHybroad->SetValue((char*)"failtoupgrade_netmanage", (char*)"", 0);
        nm_msg_level(LOG_DEBUG, "upgradeVersion = %s\n", upgradeVersion);
        acs_config_info.UpgradeFailFlag = -1;
        if (upgradeVersion[0]) {
            char softwareVersion[128];
            softwareVersion[0] = 0;
            pHybroad->GetValue((char*)"Device.DeviceInfo.SoftwareVersion", softwareVersion, 128);
            nm_msg_level(LOG_DEBUG, "softwareVersion = %s\n", softwareVersion);
            if (strcmp(upgradeVersion, softwareVersion))
                acs_config_info.UpgradeFailFlag = 1;
            else
                acs_config_info.UpgradeFailFlag = 0;
        }
    }

	pHybroad->GetValue("Device.DeviceInfo.HardwareVersion", hw_version, 128);
	pHybroad->GetValue("Device.DeviceInfo.AdditionalSoftwareVersion", hw_additional_version, 128);
	char * p = strrchr(hw_additional_version, ' ');
	if (p)
		snprintf(hw_additional_version, 128, "%s", p + 1);
	pHybroad->GetValue("Device.LAN.MACAddress", hw_mac, 128);
	nm_dbg_sysinfo_set(hw_version, hw_additional_version, hw_mac);

	pHybroad->GetValue("NetworkManagerInfo", server_info, 1024);
	if(strlen(server_info) > 4) {
		struct json_object *parse_obj = NULL, *in_obj = NULL;
		const char *str_value = NULL;

		parse_obj = json_tokener_parse(server_info);
		if (!parse_obj){
			nm_msg("error!, fail to get acs default configuration\n");
			return NULL;
		}
		str_value = get_json_value(parse_obj, "tmsurl");
		if(str_value){
			nm_msg_level(LOG_DEBUG, "Device.ManagementServer.URL is %s\n", str_value);
			strncpy(acs_config_info.url, str_value, 512);
		} else {
			json_object_put(parse_obj);
			goto error;
		}
		str_value = get_json_value(parse_obj, "tmsusername");
		if(str_value) {
			nm_msg_level(LOG_DEBUG, "Device.ManagementServer.Username is %s\n", str_value);
			strncpy(acs_config_info.user_name, str_value, 64);			
		} else {
			json_object_put(parse_obj);
			goto error;
		}
		str_value = get_json_value(parse_obj, "tmspassword");
		if(str_value){
			//nm_msg_level(LOG_DEBUG, "Device.ManagementServer.Password is %s\n", str_value);
			strncpy(acs_config_info.user_pass, str_value, 64);
			setAcsPassword(acs_config_info.user_pass);
		} else {
			json_object_put(parse_obj);
			goto error;
		}
		str_value = get_json_value(parse_obj, "startHeartbeat");
		if(str_value) {
			nm_msg_level(LOG_DEBUG, "Device.ManagementServer.PeriodicInformEnable is %s\n", str_value);
			strncpy(acs_config_info.peridic_enable, str_value, 4);			
		} else {
			json_object_put(parse_obj);
			goto error;
		}
		str_value = get_json_value(parse_obj, "tmsinterval");
		if(str_value) {
			nm_msg_level(LOG_DEBUG, "Device.ManagementServer.PeriodicInformInterval is %s\n", str_value);
			strncpy(acs_config_info.peridic_interval, str_value, 8);
		} else {
			json_object_put(parse_obj);
			goto error;
		}
		str_value = get_json_value(parse_obj, "cpeusername");
		if(str_value) {
			nm_msg_level(LOG_DEBUG, "Device.ManagementServer.ConnectionRequestUsername is %s\n", str_value);
			strncpy(acs_config_info.connect_user_name, str_value, 64);			
		} else {
			json_object_put(parse_obj);
			goto error;
		}
		str_value = get_json_value(parse_obj, "cpepassword");
		if(str_value) {
			nm_msg_level(LOG_DEBUG, "Device.ManagementServer.ConnectionRequestPassword is %s\n", str_value);
			strncpy(acs_config_info.connect_user_pass, str_value, 64);			
		} else {
			json_object_put(parse_obj);
			goto error;
		}
		
		json_object_put(parse_obj);
	}
	while(1){
		pHybroad->GetValue("Device.LAN.IPAddress", ip_addr, 64);
		if(strlen(ip_addr) >= 7)
			break;
		nm_msg("ip_addr is %s\n", ip_addr);
		sleep(2);

	}	
	tr069_start(&acs_config_info);

    tr069_log_param_init();
	return NULL;
error:
	nm_msg("faile to get acs server info\n");
	return NULL;
}

void *stbMonitor_func(void *parm)
{
	while(1){
		if(pHybroad->module_is_registered("setting")){
			nm_msg_level(LOG_DEBUG, "setting is registered,start monitor\n");
			break;
		}
		usleep(1000 * 500);
	}
	
	//启动stbMonitor
	nm_msg_level(LOG_DEBUG, "stbMonitor init ....\n");
	if (stbMonitorInit(PARAM_FILE_PATH"dataModel.xml") < 0)
		nm_msg("stbMonitor init error\n");
	
	return NULL;
}

void *log_func(void *parm)
{
	//启动log
	nm_msg_level(LOG_DEBUG, "log init ....\n");
	initLog();
	
	return NULL;
}

int main()
{
	pthread_t tr069_id = 0;
	pthread_t stbMonitor_id = 0;
	pthread_t log_id = 0;

	//启动service，等待注册listener
	nm_track();
	nm_local_param_init();
	pHybroad = new HybroadService();
	pthread_create(&log_id, NULL, log_func, NULL);
	pthread_create(&stbMonitor_id, NULL, stbMonitor_func, NULL);
	pthread_create(&tr069_id, NULL, tr069_func, NULL);
	mid_timer_init();
	nm_track();
	//service_handle_set(pHybroad);
	sp<IServiceManager> sm = defaultServiceManager();
	nm_track();
	sm->addService(String16("com.hybroad.stb"), pHybroad);
	nm_track();
	sp<ProcessState>proc(ProcessState::self());
	nm_track();
	ProcessState::self()->startThreadPool();
	nm_track();
	IPCThreadState::self()->joinThreadPool();
	nm_track();
	while(1)	{
		usleep(10000);
	}
	return 0;
}

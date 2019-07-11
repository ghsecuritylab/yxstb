#include "app_jse.h"
#include "jse.h"
#include "app_heartbit.h"
#include "app_sys.h"
#include "sys_basic_macro.h"
#include "VersionSetting.h"
#include "stream_port.h"
#include "app_pressuretest.h"
#include "mid_fpanel.h"
#include "mid/mid_time.h"
#include "NativeHandler.h"
#include "sys_key_deal.h"
#include "json/json_object.h"
#include "ind_mem.h"
#include "BrowserEventQueue.h"
#include "Hippo_api.h"
#include "NetworkFunctions.h"
#include "Assertions.h"
#include "Business.h"
#include "Jse/Hybroad/Tools/JseTools.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "MessageValueNetwork.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern const char* json_object_get_string(struct json_object *jso);
extern struct json_object* json_tokener_parse_string(const char *str);
extern struct json_object* json_object_get_object_bykey(struct json_object* obj, const char *key);

int jse_sys_sys_control(char* str)
{
    int i1 = 0;

    str += 4;
    if(!strcmp(str, "stblogin")) { //?��???????
        static int init = 0;

        if(init)
            return 0;
        init = 1;
        app_Init();
    } else if(!strcmp(str, "heartbit")) {
        httpHeartBit(0);
    } else if(jse_parse_param(str, "bootflag", ':', "d", &i1) == 1) {
        mid_sys_boot_set(i1);
    } else if(!strcmp(str, "config")) {
        // char *buf = NULL;
        // int len = 0;

        // JseCfgUpdate(buf, len); // delete at 20130926
        settingManagerSave();
    } else if(!strcmp(str, "iptv_leave")) {
        stream_multicast_leave(); // app_multicast_leave();
    } else if(jse_parse_param(str, "pressure", ':', "d", &i1) == 1) {
        app_pressureTest_set_flag(i1);
    } else if(jse_parse_param(str, "factoryset", ':', "d", &i1) == 1) {
        appFactorySet(i1);
    } else if(!strcmp(str, "upgrade")) {
#ifdef HUAWEI_C10
    	if(mid_fpanel_standby_get() == 1){
    		return 0;
    	}
#endif
	    sendMessageToNativeHandler(MessageType_System, MV_System_OpenUpgradePage, 0, 0);
    } else if(!strcmp(str, "burn")) {
        LogUserOperError("Can't support this function(%s) !\n", str);
    } else if(jse_parse_param(str, "joinflag", ':', "d", &i1) == 1) {
        BusinessSetEDSJoinFlag(i1);
    } else {
        LogUserOperError("I don't know this parms(%s)\n", str);
        return -1;
    }
    return 0;
}

int jse_sys_sys_read(char* str, char* buf, int length)
{
    str += 4;
    if(!strcmp(str, "linkstate")) {
        char devname[USER_LEN] = { 0 };
        network_default_devname(devname, USER_LEN);
        int tLinkState = network_device_link_state(devname);
        sprintf(buf, "%d", tLinkState);
        if(0 == tLinkState)
#if defined(Jiangsu)
            // Bootpage booting, cable is not connected, send 1901 to open errorpage.
            sendMessageToNativeHandler(MessageType_Network, MV_Network_PhysicalDown, 1901, 0);
#else
            sendMessageToNativeHandler(MessageType_Network, MV_Network_PhysicalDown, 0, 0);
#endif
        else
            sendMessageToNativeHandler(MessageType_Network, MV_Network_PhysicalUp, 0, 0);
    } else if(!strcmp(str, "bootflag")) {
        sprintf(buf, "%d", mid_sys_boot_get());
        LogUserOperDebug("bootflag=%s\n", buf);
    } else if(!strcmp(str, "normal")) {
        get_upgrade_version(buf);
    } else if(!strcmp(str, "pressure")) {
        sprintf(buf, "%d", app_pressureTest_get_flag());
    } else if(!strcmp(str, "errorurl")) {
        LogUserOperError("Can't support this key errorurl\n");
    } else if(!strcmp(str, "appmode")) {
        sprintf(buf, "%d", sys_appmode_get());
    } else if(!strcmp(str, "pppoeerrstate")) {
        buf[0] = '0';
        buf[1] = '\0';
    } else if(!strcmp(str, "include_dlna")) {
#ifdef INCLUDE_DLNA
        IND_STRCPY(buf, "1");
#else
        IND_STRCPY(buf, "0");
#endif
    } else {
        LogUserOperDebug("I don't know this parms(%s)\n", str);
        return -1;
    }
    return 0;
}

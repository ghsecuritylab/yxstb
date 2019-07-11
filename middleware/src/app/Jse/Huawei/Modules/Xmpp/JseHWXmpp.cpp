
#include "JseHWXmpp.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"
#include "XmppService.h"

#include "AppSetting.h"

#include "json/json_tokener.h"

#include <string.h>
#include <stdlib.h>


using namespace gloox;

static int JseXmppServerUriWrite( const char* param, char* value, int len )
{
    if (!value || !strlen(value)) {
        LogJseError("Invalid value.\n");
        return -1;
    }
    LogJseDebug("%s\n", value);

    struct json_object *obj = NULL;
    obj = json_tokener_parse(value);
    if (!obj) {
        LogJseError("Json parse error.\n");
        return -1;
    }
    const char *serverUri = json_object_get_string(json_object_object_get(obj, "pushServerURL"));
    if (!serverUri) {
        LogJseError("Get pushServerURL error!\n");
        json_object_put(obj);
        return -1;
    }

    const char *port = strchr(serverUri, ':');
    if (port) {
        xmppService()->setServerUrl(serverUri, port - serverUri);
        xmppService()->setPort(atoi(port + 1));
    } else
        xmppService()->setServerUrl(serverUri, strlen(serverUri));
    xmppService()->m_didGetServerUrl = true;
    LogJseDebug("Get xmpp servrURL: %s, port: %d\n", xmppService()->getServerUrl().c_str(), xmppService()->getPort());
    json_object_put(obj);
    return 0;
}

static int JseXmppServiceEnableRead( const char* param, char* value, int len )
{
    //undefined presently
    return 0;
}

static int JseXmppServiceEnableWrite( const char* param, char* value, int len )
{
    if (!value || !strlen(value)) {
        LogJseError("Invalid value.\n");
        return -1;
    }
    LogJseDebug("%s\n", value);

    struct json_object *obj = NULL;
    obj = json_tokener_parse(value);
    if (!obj) {
        LogJseError("Json parse error.\n");
        return -1;
    }
    const char *serviceEnable = json_object_get_string(json_object_object_get(obj, "serviceEnable"));
    if (!serviceEnable) {
        LogJseError("Get string error!\n");
        json_object_put(obj);
        return -1;
    }
    if (xmppService()) {
        xmppService()->setXmppServiceEnableByEPG(atoi(serviceEnable));
    } else {
        LogJseError("Get xmpp service failed.\n");
        json_object_put(obj);
        return -1;
    }
    json_object_put(obj);
    return 0;
}

static int JseXmppJIDWrite( const char* param, char* value, int len )
{
    if (!value || !strlen(value)) {
        LogJseError("Invalid value.\n");
        return -1;
    }
    LogJseDebug("%s\n", value);

    if (xmppService()) {
        xmppService()->setXmppJID(std::string(value));
        xmppService()->m_didGetJID = true;

        char netUser[34] = {0};
        appSettingGetString("ntvuser", netUser, 32, 0);
        std::string phoneName = xmppAccountEscape(std::string(netUser));
        xmppService()->m_myPhone = xmppService()->getXmppJID();
        xmppService()->m_myPhone.setUsername(phoneName);

        xmppService()->prepareConnect();
        LogJseDebug("Get Jid: %s/%s\n", xmppService()->getXmppJID().bare().c_str(), xmppService()->getXmppJID().resource().c_str());
    } else {
        LogJseError("Get xmpp service failed.\n");
    }
    return 0;
}

static int JseXmppCapabiltiyRead( const char* param, char* value, int len )
{
    // If defined XMPP, the value is always "1".
    int xmppCapability = 1;
    sprintf(value, "%d", xmppCapability);
    return 0;
}

static int JseXmppAutoBindingRead( const char* param, char* value, int len )
{
    if (xmppService())
        sprintf(value, "%d", xmppService()->m_xmppAutoBinding);
    else {
        LogJseError("Get xmpp service failed.\n");
    }
    return 0;
}

static int JseXmppAutoBindingWrite( const char* param, char* value, int len )
{
    if (!value || !strlen(value)) {
        LogJseError("Invalid value.\n");
        return -1;
    }
    LogJseDebug("%s\n", value);

    if (xmppService())
        xmppService()->m_xmppAutoBinding = atoi(value);
    else {
        LogJseError("Get xmpp service failed.\n");
        return -1;
    }
    return 0;
}

static int JseXmppRosterWrite( const char* param, char* value, int len )
{
    if (!value || !strlen(value)) {
        LogJseError("Invalid value.\n");
        return -1;
    }
    LogJseDebug("%s\n", value);
    struct json_object *obj = NULL;
    obj = json_tokener_parse(value);
    if (!obj) {
        LogJseError("Json parse error.\n");
        return -1;
    }
    const char *jid = json_object_get_string(json_object_object_get(obj, "jid"));
    const char *action = json_object_get_string(json_object_object_get(obj, "action"));
    if (!jid || !action) {
        LogJseError("Json get string error.\n");
        json_object_put(obj);
        return -1;
    }
    if (!xmppService()) {
        LogJseError("Get xmppService failed.\n");
        json_object_put(obj);
		return -1;
	}
    JID friendJID;
    friendJID.setJID(std::string(jid));
    if (!strncmp(action, "add", 3)) {
        xmppService()->addFriendToRosterByEPG(friendJID);
    } else if (!strncmp(action, "remove", 6)) {
        xmppService()->removeFriendFromRosterByEPG(friendJID);
    }

    json_object_put(obj);
    return 0;
}

static int JseXmppRosterItemCountRead( const char* param, char* value, int len )
{
    int result = 0;
    int count = 0;
	if (xmppService())
        result = xmppService()->getRosterItemCountByEPG(count);
    if (-1 == result || 1 == result) {
        sprintf(value, "{\"result\":\"%d\"}", result);
        return -1;
    } else if (0 == result)
        sprintf(value, "{\"result\":\"%d\",\"count\":\"%d\"}", result, count);
    return 0;

}

static int JseXmppRosterItemListRead( const char* param, char* value, int len )
{
    std::string rosterIetemList;
    if (!param || !strlen(param)) {
        LogJseError("Invalid value.\n");
        return 0;
    }
    LogJseDebug("%s\n", param);
    struct json_object *obj = NULL;
    obj = json_tokener_parse(param);
    if (!obj) {
        LogJseError("Json parse error.\n");
        return -1;
    }
    if(xmppService())
        rosterIetemList = xmppService()->getXmppRosterItemListByEPG(obj);
    snprintf(value, len, "%s", rosterIetemList.c_str());
    return 0;
}

static int JseXmppScreenPlayStartModeWrite( const char* param, char* value, int len )
{
    if (!value || !strlen(value)) {
        LogJseError("Invalid value.\n");
        return -1;
    }
    LogJseDebug("%s\n", value);

    if (xmppService()) {
        if (!strcmp(value, "byEvent"))
            xmppService()->setScreenPlayStartMode(XmppService::XMPP_PLAY_BY_EVENT);
        else if (!strcmp(value, "byURL"))
            xmppService()->setScreenPlayStartMode(XmppService::XMPP_PLAY_BY_URL);
        else
            LogJseWarning("Undefined parameter!\n");
    } else {
        LogJseError("Get xmpp service failed.\n");
        return -1;
    }
    return 0;
}



/*************************************************
Description: 初始化华为Xmpp模块配置定义的接口，由JseHWModules.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWXmppInit()
{
    JseCall* call;

    call = new JseFunctionCall("XMPPpushServerUri", 0, JseXmppServerUriWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("XMPPService", JseXmppServiceEnableRead, JseXmppServiceEnableWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("XMPPJID", 0, JseXmppJIDWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("XMPPCapabiltiy", JseXmppCapabiltiyRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("XMPPAutoBinding", JseXmppAutoBindingRead, JseXmppAutoBindingWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("XMPPRosterSet", 0, JseXmppRosterWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("getXMPPRosterItemCount", JseXmppRosterItemCountRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("getXMPPRosterItemList", JseXmppRosterItemListRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("MultiScreenPlayStartMode", 0, JseXmppScreenPlayStartModeWrite);
    JseRootRegist(call->name(), call);



    return 0;
}


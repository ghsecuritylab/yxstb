
#include "JseHWResource.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

#include "json/json_public.h"
#include "Concurrent.h"
#include "NetworkDevice.h"
#include "ResourceManager.h"

#include "AppSetting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sstream>

#ifdef HUAWEI_C20
//下面的两个变量都是在DeviceConfig.cpp定义的
extern Hippo::NetworkDevice *gNetworkDeviceConfig;
extern Hippo::Concurrent  *gConcurrentConfig;

static int JseMaxChannelBandwidthRead(const char *param, char *value, int len)
{
    int maxChannelBandwidth = 0;
    appSettingGetInt("maxChannelBandwidth", &maxChannelBandwidth, 0);
    sprintf(value, "%d", maxChannelBandwidth);
    return 0;
}

static int JseMaxChannelBandwidthWrite(const char *param, char *value, int len)
{
    appSettingSetInt("maxChannelBandwidth", atoi(value));
    return 0;
}

static int JseNumberOfConcurrentWrite(const char *param, char *value, int len)
{
   	int ret = 0;

   	if(!value)
    	ret = -1;
   	else
    	ret = gConcurrentConfig->setConcurrentNumber(atoi(value));
   	return ret;
}

static int JseAppPriorityModifyWrite(const char *param, char *value, int len)
{
   int parseloop =0;
   std::string str;
   std::string substr;
   std::string jsonstr;

   int firstpos,lastpos;

   struct json_object *obj = NULL;
   struct json_object *sub_obj = NULL;
   str = value;

   firstpos =0;
   lastpos =0;

   firstpos = str.find('{', 0 );
   if(firstpos==-1){
        return -1;
   }

   lastpos = str.find( '}', 0 );
   if(lastpos==-1){
        return -1;
   }

   substr = str.substr(firstpos,lastpos-firstpos+1);

   obj = json_tokener_parse_string( substr.c_str() );
    if (!obj ) {
        return -1;
    }
    sub_obj = json_object_get_object_bykey( obj, "sort" );
    if (!sub_obj ) {
        json_object_delete(obj);
        return -1;
    }

    jsonstr = json_object_get_string(sub_obj);

    std::stringstream ss(jsonstr);

    while (getline(ss,substr,'>'))
    {
      if (parseloop >= 4)
         break;

      if (strncasecmp(substr.c_str(),"PVR",substr.length())==0)
      {
        Hippo::resourceManager().m_priorityMap[Hippo::ResourceUser::PVR] = Hippo::resourceManager().m_hightPriority-Hippo::resourceManager().m_priorityStep*parseloop;
      }
      else if(strncasecmp(substr.c_str(),"PLAYING",substr.length())==0)
      {
       Hippo::resourceManager().m_priorityMap[Hippo::ResourceUser::SimplePlay] = Hippo::resourceManager().m_hightPriority-Hippo::resourceManager().m_priorityStep*parseloop;
      }
      else if(strncasecmp(substr.c_str(),"PIP",substr.length())==0)
      {
        Hippo::resourceManager().m_priorityMap[Hippo::ResourceUser::PIP] = Hippo::resourceManager().m_hightPriority-Hippo::resourceManager().m_priorityStep*parseloop;
      }
      else if(strncasecmp(substr.c_str(),"DOWNLOAD",substr.length())==0)
      {
        Hippo::resourceManager().m_priorityMap[Hippo::ResourceUser::Download] = Hippo::resourceManager().m_hightPriority-Hippo::resourceManager().m_priorityStep*parseloop;
      }

      parseloop++;
    }

    json_object_delete(obj);
    if (parseloop !=4)
        return -1;
   return 0;
}

static int JseAllowBandwidthRead(const char *param, char *value, int len)
{
    int ret = 0;
    if(!gNetworkDeviceConfig || !value){
      ret =-1;
    }else{
        sprintf(value, "%d", gNetworkDeviceConfig->getNetworkBandwidth());
    }
    return ret;
}

static int JseAllowBandwidthWrite(const char *param, char *value, int len)
{
    int ret = 0;

    float bandwidth = 10.0;

    if(!gNetworkDeviceConfig|| !value){
      ret =-1;
    }else{
       bandwidth= atof(value);
       ret= gNetworkDeviceConfig->setNetworkBandwidth(bandwidth);
    }
    return ret;
}

static int JseHWresourceStatusRead(const char *param, char *value, int len)
{
	int result = -1;
	struct json_object *jsonObj = NULL;
	json_object *jsonObjResult = NULL;

	if (value == NULL || value[0] != '{' || value[strlen(value)-1] != '}')
		return result;

	jsonObj = json_tokener_parse_string(value);
	if (jsonObj) {
		std::string jsonStr;
		struct json_object *subJsonObj = NULL;
		struct json_object *resultJsonObj = NULL;

		subJsonObj = json_object_get_object_bykey(jsonObj, "resourceType");
		if (subJsonObj) {
			jsonStr = json_object_get_string(subJsonObj);
		}
		json_object_delete(jsonObj);

		jsonObjResult = json_object_new_object();
		if (jsonObjResult == NULL)
			return result;

		if (jsonStr.compare("tuner") == 0) {
			Hippo::resourceManager().getTunerResourceDetail(jsonObjResult);
			result = 0;
		} else if (jsonStr.compare("concurrent") == 0) {
			Hippo::resourceManager().getConcurrentResourceDetail(jsonObjResult);
			result = 0;
		} else if (jsonStr.compare("bandwidth") == 0) {
			;
		}

		if (result == 0) {
			jsonStr = json_object_get_string(jsonObjResult);
			if (jsonStr.length() < 4096)
				strncpy(value, jsonStr.c_str(), jsonStr.length());
		}

		json_object_delete(jsonObjResult);
	}
	return result;
}


JseHWResource::JseHWResource()
	: JseGroupCall("resourceStatus")
{
    JseCall* call;

    call = new JseFunctionCall("get", JseHWresourceStatusRead,0);
    regist(call->name(), call);
}

JseHWResource::~JseHWResource()
{
}
#endif

/*************************************************
Description: 初始化华为业务资源定义的接口，由JseHWBusiness.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWResourceInit()
{
    JseCall* call;
#ifdef HUAWEI_C20
    //以下全为C20 regist
    call = new JseFunctionCall("MaxChannelBandwidth", JseMaxChannelBandwidthRead, JseMaxChannelBandwidthWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("numberOfConcurrent", 0, JseNumberOfConcurrentWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("appPriorityModify", 0, JseAppPriorityModifyWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("AllowBandwidth", JseAllowBandwidthRead, JseAllowBandwidthWrite);
    JseRootRegist(call->name(), call);

    call = new JseHWResource();
    JseRootRegist(call->name(), call);
#endif
    return 0;
}


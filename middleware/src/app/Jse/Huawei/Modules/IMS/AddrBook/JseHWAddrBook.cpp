
#if defined(INCLUDE_IMS)
#include "JseHWAddrBook.h"

#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include <stdio.h>
#include <string.h>

//ÓÉÓÚ²»ÖªµÀÏÂÃæº¯ÊýÖÐµ÷ÓÃµÄ½Ó¿Ú¾ßÌå°üº¬ÄÄÐ©Í·ÎÄ¼þ£¬Òò´Ë½«¿ÉÄÜ°üº¬µÄÍ·ÎÄ¼þÈ«²¿ÁÐ³ö
#include"FriendBasic.h"
#include"Friends.h"
#include"FriendTV.h"
#include"Group.h"
#include"GroupMember.h"
#include"IMSRegister.h"
#include"InfoPush.h"
#include"platform_types.h"
#include"yx_ims_init.h"
#include"dev_flash.h"
#include"YX_IMS_porting.h"
#include"TMW_Camera.h"
#include"TMW_Media.h"


static int JseGetGroupCountRead(const char* param, char* value, int len)
{
    int ret = 0;

    ret = TMW_group_getCount(GROUP_CONTACT);
    if(-1 != ret) {
        sprintf(value, "%d", ret);
        LogJseDebug("################ GROUP_CONTACT:%d\n", ret);
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseGetGroupNameRead(const char* param, char* value, int len)
{
    char* ptr = NULL;
    int gindex = 0;
    GroupBasicInfo groupBasicInfo;
    int ret = 0;

    memset(&groupBasicInfo, 0, sizeof(GroupBasicInfo));
    ptr = strchr(func, ',');
    if(NULL != ptr) {
        gindex = strtol(ptr + 1, NULL, 10);
        ret = TMW_group_getBasicInfo(gindex, GROUP_CONTACT, &groupBasicInfo);
        if(0 == ret) {
            sprintf(value, "%s", groupBasicInfo.szName + 1);
        } else {
            YIMS_ERR();
        }
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseGetCountRead(const char* param, char* value, int len)
{
    char* ptr = NULL;
    int gindex = 0;
    int count = 0;
    ptr = strchr(func, ',');
    if(NULL != ptr) {
        gindex = strtol(ptr + 1, NULL, 10);
        count = TMW_addrBook_getCount(gindex, GROUP_CONTACT);
        if(count >= 0) {
            LogJseDebug("##############AddrBook.GetCount [%d]\n", count);
            sprintf(value, "%d", count);
        } else {
            YIMS_ERR();
        }
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseGetInfoRead(const char* param, char* value, int len)
{
    char* ptr1 = NULL;
    char* ptr2 = NULL;
    int gindex = 0;
    int index = 0;
    const GroupMemberInfo* pMemberInfo = NULL;
    ptr1 = strchr(func, ',');
    ptr2 = strchr(ptr1 + 1, ',');
    if(NULL != ptr1 && NULL != ptr2) {
        gindex = strtol(ptr1 + 1, NULL, 10);
        index = strtol(ptr2 + 1, NULL, 10);

        pMemberInfo = TMW_addrBook_getInfo(gindex, index, GROUP_CONTACT);
        if(NULL != pMemberInfo) {
            sprintf(value,
                    "{\"name\":\"%s\",\"mobile\":\"%s\",\"tel\":\"%s\"}",
                    pMemberInfo->szName,
                    pMemberInfo->szMobile,
                    pMemberInfo->szTel);
        } else {
            YIMS_ERR();
        }

    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseGetIndexByInfoRead(const char* param, char* value, int len)
{
    char* ptr = NULL;
    int ret = 0;
    int i = 0;
    int groupCount = 0;

    ptr = strchr(func, ',');
    if(NULL != (ptr + 1)) {
        groupCount = TMW_group_getCount(GROUP_CONTACT);
        for(i = 0; i < groupCount; i++) {
            ret = TMW_addrBook_getIndexByID(ptr + 1, i, GROUP_CONTACT);
            if(ret >= 0) {
                sprintf(value, "{\"gindex\":\"%d\",\"index\":\"%d\"}", i, ret);
                break;
            }
        }
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseSetGroupNameWrite(const char* param, char* value, int len)
{
    YX_IMS_JSON_PARSE jsonParse;
    YX_IMS_JSON_STR* pJsonNameValue = NULL;
    int gindex = 0;
    char name[24] = { 0 };
    int ret = 0;

    ims_json_parse_string(value, &jsonParse);
    pJsonNameValue = jsonParse.jsonStr;
    if(NULL != pJsonNameValue) {
        gindex = strtol(pJsonNameValue->value, NULL, 10);
        pJsonNameValue = pJsonNameValue->next;
        if(NULL != pJsonNameValue) {
            strncpy(name, pJsonNameValue->value, 24);
        } else {
            YIMS_ERR();
            ret = -1;
        }
    } else {
        YIMS_ERR();
        ret = -1;
    }
    ims_json_parse_free(&jsonParse);

    if(0 == ret) {
        ret = TMW_group_changeByIndex(GROUP_FREIND, gindex, name);
        if(ret) {
            YIMS_ERR();
        }
    }

    return 0;
}

static int JseAddGroupWrite(const char* param, char* value, int len)
{
    int ret = 0;
    GroupInfo tagGroupInfo;

    memset(&tagGroupInfo, 0, sizeof(GroupInfo));
    strncpy(tagGroupInfo.szName, value, GROUP_NAME_LENGTH);
    tagGroupInfo.eAttr = 1; /*friend*/
    ret = TMW_group_add(&tagGroupInfo);
    if(-3 == ret) {
        /*name conflict*/
        YX_IMS_putEventToList(EVENT_IMS_GROUP,
                              "EVENT_GROUP_ALREADY_EXIST",
                              201801,
                              NULL,
                              NULL);
        YIMS_ERR();
    } else if(-1 == ret) {
        /*max count */

        YIMS_ERR();
    }

    return 0;
}

static int JseDelGroupWrite(const char* param, char* value, int len)
{
    int ret = 0;

    ret = TMW_group_delete(value, GROUP_FREIND);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}


static int JseSetInfoWrite(const char* param, char* value, int len)
{
    YX_IMS_JSON_PARSE jsonParse;
    YX_IMS_JSON_STR* pJsonNameValue = NULL;
    int i = 0;
    GroupMemberInfo tagGroupMemberInfo;
    const GroupMemberInfo* tempInfo = NULL;
    int gindex = 0;
    int index = 0;
    char name[64] = { 0 };
    char mobile[32] = { 0 };
    char tel[32] = { 0 };
    int ret = 0;

    memset(&tagGroupMemberInfo, 0, sizeof(GroupMemberInfo));
    ims_json_parse_string(value, &jsonParse);
    pJsonNameValue = jsonParse.jsonStr;
    for(i = 0; i < jsonParse.count; i++) {
        if(NULL == pJsonNameValue) {
            YIMS_ERR();
            ret = -1;
            break;
        }
        if(!strcmp(pJsonNameValue->name, "gindex")) {
            gindex = strtol(pJsonNameValue->value, NULL, 10);
        } else if(!strcmp(pJsonNameValue->name, "index")) {
            index = strtol(pJsonNameValue->value, NULL, 10);
        } else if(!strcmp(pJsonNameValue->name, "name")) {
            strncpy(name, pJsonNameValue->value, 64);
        } else if(!strcmp(pJsonNameValue->name, "mobile")) {
            strncpy(mobile, pJsonNameValue->value, 32);
        } else if(!strcmp(pJsonNameValue->name, "tel")) {
            strncpy(tel, pJsonNameValue->value, 32);
        } else {
            YIMS_ERR();
        }
        pJsonNameValue = pJsonNameValue->next;
    }
    ims_json_parse_free(&jsonParse);

    tempInfo = TMW_addrBook_getInfo(gindex, index, GROUP_CONTACT);
    if(NULL != tempInfo) {
        memcpy(&tagGroupMemberInfo, tempInfo, sizeof(GroupMemberInfo));
        memset(tagGroupMemberInfo.szName, 0, sizeof(tagGroupMemberInfo.szName));
        memset(tagGroupMemberInfo.szMobile,
               0,
               sizeof(tagGroupMemberInfo.szMobile));
        memset(tagGroupMemberInfo.szTel, 0, sizeof(tagGroupMemberInfo.szTel));
        strncpy(tagGroupMemberInfo.szName, name, strlen(name));
        strncpy(tagGroupMemberInfo.szMobile, mobile, strlen(mobile));
        strncpy(tagGroupMemberInfo.szTel, tel, strlen(tel));
        ret = TMW_addrBook_setInfo(gindex, index, GROUP_CONTACT, &tagGroupMemberInfo);
        if(ret) {
            YIMS_ERR();
        }
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseAddWrite(const char* param, char* value, int len)
{
    YX_IMS_JSON_PARSE jsonParse;
    YX_IMS_JSON_STR *pJsonNameValue = NULL;
    int gindex = 0;
    char url[128] = { 0 };
    int ret = 0;

    ims_json_parse_string(value, &jsonParse);
    pJsonNameValue = jsonParse.jsonStr;
    if(NULL != pJsonNameValue) {
        gindex = strtol(pJsonNameValue->value, NULL , 10);
        pJsonNameValue = pJsonNameValue->next;
        if(NULL != pJsonNameValue) {
            strncpy(url, pJsonNameValue->value, 128);
        } else {
            YIMS_ERR();
            ret = -1;
        }
    } else {
        YIMS_ERR();
        ret = -1;
    }
    ims_json_parse_free(&jsonParse);

    if(0 == ret) {
        ret = TMW_friends_enquire(url);
        if(ret) {
            YIMS_ERR();
            YX_IMS_putEventToList(EVENT_IMS_FRIENDS, "EVENT_FRIENDS_NO_ACCOUNT", 201505, (void *)url, NULL);
        } else {
            ret = TMW_friends_add(gindex, url);
            if(ret == -3) { /*T??????*/
                YX_IMS_putEventToList(EVENT_IMS_FRIENDS, "EVENT_FRIENDS_ALREADY_EXIST", 201506, (void *)url, NULL);
            } else if(-1 == ret) { /*?????T????*/
                YX_IMS_putEventToList(EVENT_IMS_FRIENDS, "EVENT_ADDRBOOK_EXCEED_AMOUNT", 201402, (void *)url, NULL);
                YIMS_ERR();
            } else {
                YIMS_ERR();
            }
        }
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseDelWrite(const char* param, char* value, int len)
{
    YX_IMS_JSON_PARSE jsonParse;
    YX_IMS_JSON_STR *pJsonNameValue = NULL;
    int gindex = 0;
    int index = 0;
    int ret = 0;

    ims_json_parse_string(value, &jsonParse);
    pJsonNameValue = jsonParse.jsonStr;
    if(NULL != pJsonNameValue) {
        gindex = strtol(pJsonNameValue->value, NULL , 10);
        pJsonNameValue = pJsonNameValue->next;
        if(NULL != pJsonNameValue) {
            index = strtol(pJsonNameValue->value, NULL , 10);
        } else {
            YIMS_ERR();
            ret = -1;
        }
    } else {
        YIMS_ERR();
        ret = -1;
    }
    ims_json_parse_free(&jsonParse);

    if(0 == ret) {

        if(gindex == -2) {
            ret = TMW_addrBook_delete(0, index, GROUP_BLACK);
            if(ret) {
                YIMS_ERR();
            }
        } else {
            ret = TMW_friends_deleteByIndex(gindex, index);
            if(ret) {
                YIMS_ERR();
            }
        }
    }

    return 0;
}

static int JseMoveWrite(const char* param, char* value, int len)
{
    YX_IMS_JSON_PARSE jsonParse;
    YX_IMS_JSON_STR* pJsonNameValue = NULL;
    int gindex = 0;
    int index = 0;
    int destgindex = 0;
    int ret = 0;
    int i = 0;
    const FriendInfo* F_info = NULL;

    ims_json_parse_string(value, &jsonParse);
    pJsonNameValue = jsonParse.jsonStr;
    for(i = 0; i < jsonParse.count; i++) {
        if(NULL == pJsonNameValue) {
            YIMS_ERR();
            ret = -1;
            break;
        }
        if(!strcmp(pJsonNameValue->name, "gindex")) {
            gindex = strtol(pJsonNameValue->value, NULL, 10);
            //IND_STRCPY(uri, pJsonNameValue->value);
        } else if(!strcmp(pJsonNameValue->name, "index")) {
            index = strtol(pJsonNameValue->value, NULL, 10);
        } else if(!strcmp(pJsonNameValue->name, "destgroup")) {
            destgindex = strtol(pJsonNameValue->value, NULL, 10);
        } else {
            LogJseDebug("err unknow name\n");
        }

        pJsonNameValue = pJsonNameValue->next;
    }
    ims_json_parse_free(&jsonParse);
    F_info = TMW_friends_getInfo(gindex, index);
    if(NULL == F_info) {
        YIMS_ERR();
    }

    ret = TMW_friends_group_switch(destgindex, (void *) F_info->szURI);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

/*************************************************
Description: ³õÊ¼»¯²¢×¢²á»ªÎª¶¨ÒåµÄ½Ó¿Ú <AddrBook.***> Îª²Ù×÷µØÖ·²¾Ìá¹©½Ó¿Ú£¬
             ¸Ã½Ó¿ÚÊÇIMSÏµÍ³µÄ×Ó¹¦ÄÜ½Ó¿Ú¡£
½Ó¿ÚÏà¹ØËµÃ÷¼û ¡¶IPTV STB V100R002C10&C20 IMSÒµÎñSTBÓëEPG½Ó¿ÚËµÃ÷Êé¡·
Input: ÎÞ
Return: ÎÞ
 *************************************************/
JseHWAddrBook::JseHWAddrBook()
	: JseGroupCall("AddrBook")
{
    JseCall* Call;
	
    Call = new JseFunctionCall("GetGroupCount", JseGetGroupCountRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetGroupName", JseGetGroupNameRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetCount", JseGetCountRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetInfo", JseGetInfoRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetIndexByInfo", JseGetIndexByInfoRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("SetGroupName", 0, JseSetGroupNameWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("AddGroup", 0, JseAddGroupWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("DelGroup", 0, JseDelGroupWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("SetInfo", 0, JseSetInfoWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Add", 0, JseAddWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Del", 0, JseDelWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Move", 0, JseMoveWrite);
    regist(Call->name(), Call);	

}

JseHWAddrBook::~JseHWAddrBook()
{
}

#endif

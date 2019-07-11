
#if defined(INCLUDE_IMS)
#include "JseHWFriends.h"
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

static int JseSetPropertyWrite(const char* param, char* value, int len)
{
    YX_IMS_JSON_PARSE jsonParse;
    YX_IMS_JSON_STR* pJsonNameValue = NULL;
    int request = 0;
    int seeAction = 0;
    int ret = 0;
    const FriendInfo* pInfo = NULL;
    FriendInfo tempInfo;

    ims_json_parse_string(value, &jsonParse);
    pJsonNameValue = jsonParse.jsonStr;
    if(NULL != pJsonNameValue) {
        request = strtol(pJsonNameValue->value, NULL, 10);
        pJsonNameValue = pJsonNameValue->next;
        if(NULL != pJsonNameValue) {
            seeAction = strtol(pJsonNameValue->value, NULL, 10);
        } else {
            YIMS_ERR();
        }
    } else {
        YIMS_ERR();
    }
    ims_json_parse_free(&jsonParse);
    if(1 == YX_IMS_getRegisterFlag()) {
        pInfo = TMW_friends_getMyInfo();
        if(NULL != pInfo) {
            memset(&tempInfo, 0, sizeof(FriendInfo));
            memcpy(&tempInfo, pInfo, sizeof(FriendInfo));
            tempInfo.iActionSwitch   = seeAction;
            tempInfo.iAddFriendFlag = request;
            /*  0??????????
                1??????
                2????????
                */
            ret = TMW_friends_setMyInfo(&tempInfo,  1);
            if(ret) {
                YIMS_ERR();
            }
        } else {
            YIMS_ERR();
        }
    }

    return 0;
}

static int JseGetPropertyRead(const char* param, char* value, int len)
{
    const FriendInfo *info = NULL;
    info =  TMW_friends_getMyInfo();
    if(NULL != info) {
        sprintf(value, "{\"request\":\"%d\",\"seeAction\":\"%d\"}", info->iAddFriendFlag, info->iActionSwitch);
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseGetGroupCountRead(const char* param, char* value, int len)
{
    int ret = 0;

    ret = TMW_group_getCount(GROUP_FREIND);
    if(-1 != ret) {
        LogJseDebug("############Friends.GetGroupCount[%d]\n", ret);
        sprintf(value, "%d", ret);
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

static int JseSetMyInfoWrite(const char* param, char* value, int len)
{
    YX_IMS_JSON_PARSE jsonParse;
    YX_IMS_JSON_STR* pJsonNameValue = NULL;
    int i = 0;
    FriendInfo MyInfo;
    const  FriendInfo* pTempMyInfo = NULL;
    int ret = 0;
    char nike[64] = { 0 };
    int head = 0;
    int state = 0;

    memset(&MyInfo, 0, sizeof(FriendInfo));

    ims_json_parse_string(value, &jsonParse);
    pJsonNameValue = jsonParse.jsonStr;
    for(i = 0; i < jsonParse.count; i++) {
        if(NULL == pJsonNameValue) {
            LogJseDebug("NULL == pJsonNameValue\n");
            break;
        }
        if(!strcmp(pJsonNameValue->name, "nick")) {
            strncpy(nike, pJsonNameValue->value, 64);
        } else if(!strcmp(pJsonNameValue->name, "head")) {
            head = strtol(pJsonNameValue->value, NULL, 10);
        } else if(!strcmp(pJsonNameValue->name, "state")) {
            state = strtol(pJsonNameValue->value, NULL, 10);
        } else {
            LogJseDebug("err unknow name\n");
        }
        pJsonNameValue = pJsonNameValue->next;
    }
    ims_json_parse_free(&jsonParse);

    pTempMyInfo = TMW_friends_getMyInfo();
    if(NULL == pTempMyInfo) {
        ret = -1;
    }
    if(0 == ret) {
        memcpy(&MyInfo, pTempMyInfo, sizeof(FriendInfo));
        strncpy(MyInfo.szNickname, nike, 64);
        MyInfo.iHead = head;
        MyInfo.eState = state;
        ret = TMW_friends_setMyInfo(&MyInfo, 1);
        if(ret) {
            YIMS_ERR();
        }
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseAcceptWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_friends_accept(value);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

static int JseRejectWrite(const char* param, char* value, int len)
{
    int ret = 0;
    ret = TMW_friends_reject(value);
    if(ret) {
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
        count = TMW_friends_getCount(gindex);
        if(count >= 0) {
            sprintf(value, "%d", count);
        } else {
            YIMS_ERR();
        }
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseGetIndexByURIRead(const char* param, char* value, int len)
{
    char* urlStr = NULL;
    int gindex = 0;
    int index = 0;

    urlStr = strchr(func, ',');
    if(NULL != urlStr) {
        index = TMW_friends_getIndex(&gindex, urlStr + 1);
        if(index >= 0) {
            sprintf(value, "{\"gindex\":\"%d\",\"index\":\"%d\"}", gindex, index);
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
    char *ptr = NULL;
    int gindex = 0;
    int index = 0;
    const FriendInfo *pInfo = NULL;
    const GroupMemberInfo *pMemberInfo = NULL;
    ptr = strchr(func, ',');
    if(NULL != ptr) {
        gindex = strtol(ptr + 1, NULL, 10);
        ptr = strchr(ptr + 1, ',');
        if(NULL != ptr) {
            index = strtol(ptr + 1, NULL, 10);
            LogJseDebug("\n ############gindex:%d,index:%d \n", gindex, index);

            pInfo = TMW_friends_getInfo(gindex, index);
            if(NULL != pInfo) {
                int tempPropety = 0;
                if(pInfo->iGroupID) {
                    tempPropety = 0;
                } else {
                    tempPropety = 1;
                }
                sprintf(value, "{\"uri\":\"%s\",\"nick\":\"%s\",\"head\":\"%d\",\"state\":\"%d\",\"groupProperty\":\"%d\",\"action\":\"%s\",\"newMsg\":\"%d\"}", \
                        pInfo->szURI, pInfo->szNickname, pInfo->iHead, pInfo->eState, tempPropety, pInfo->szAction, pInfo->iHasNewIM);
            } else {
                if(gindex != -2) {
                    YIMS_ERR();
                    pMemberInfo = TMW_addrBook_getInfo(gindex, index, GROUP_FREIND);
                    if(NULL != pMemberInfo) {
                        sprintf(value, "{\"uri\":\"%s\",\"nick\":\"%s\",\"head\":\"%d\",\"state\":\"%d\",\"groupProperty\":\"%d\",\"action\":\"%s\",\"newMsg\":\"%d\"}", \
                                pMemberInfo->szURI, " ", 0, 0, 0, " ", 0);
                    } else {
                        YIMS_ERR();
                    }
                } else {
                    pMemberInfo = TMW_addrBook_getInfo(0, index, GROUP_BLACK);
                    if(NULL != pMemberInfo) {
                        sprintf(value, "{\"uri\":\"%s\",\"nick\":\"%s\",\"head\":\"%d\",\"state\":\"%d\",\"groupProperty\":\"%d\",\"action\":\"%s\",\"newMsg\":\"%d\"}", \
                                pMemberInfo->szURI, " ", 0, 0, 0, " ", 0);
                    } else {
                        YIMS_ERR();
                    }
                }
            }
        } else {
            YIMS_ERR();
        }
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseGetMyInfoRead(const char* param, char* value, int len)
{
    const FriendInfo* pInfo = NULL;

    pInfo = TMW_friends_getMyInfo();
    if(NULL != pInfo) {
        sprintf(value,
                "{\"uri\":\"%s\",\"nick\":\"%s\",\"head\":\"%d\",\"state\":\"%d\",\"action\":\"%s\"}",
                \
                pInfo->szURI,
                pInfo->szNickname,
                pInfo->iHead,
                pInfo->eState,
                pInfo->szAction)    ;
    } else {
        YIMS_ERR();
    }

    return 0;
}

static int JseDelByUriWrite(const char* param, char* value, int len)
{
    int ret = 0;

    ret = TMW_friends_delete(value);
    if(ret) {
        YIMS_ERR();
    }

    return 0;
}

int JseHWFriends::call(const char *name, const char *param, char *value, int length, int set)
{
    std::map<std::string, JseCall*>::iterator it = m_callMap.find(name);
    LogJseDebug("### find call (it->second)name :%s\n", (it->second)->name());
    if ( it != m_callMap.end()) {
        return (it->second)->call(name, param, value, length, set);
    } else {
        LogJseDebug("### not find call :%s\n", name);
        return JseGroupCall::call(name, param, value, length, set);
    }
}

/*************************************************
Description: ³õÊ¼»¯²¢×¢²á»ªÎª¶¨ÒåµÄ½Ó¿Ú <Friends.***>  ¸Ã½Ó¿ÚÊÇIMS
             ÏµÍ³µÄ×Ó¹¦ÄÜ½Ó¿Ú¡£
½Ó¿ÚÏà¹ØËµÃ÷¼û ¡¶IPTV STB V100R002C10&C20 IMSÒµÎñSTBÓëEPG½Ó¿ÚËµÃ÷Êé¡·
Input: ÎÞ
Return: ÎÞ
 *************************************************/
JseHWFriends::JseHWFriends()
	: JseGroupCall("Friends")
{
    JseCall* Call;
	
    Call = new JseFunctionCall("SetProperty", 0, JseSetPropertyWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetProperty", JseGetPropertyRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetGroupCount", JseGetGroupCountRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("SetGroupName", 0, JseSetGroupNameWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("AddGroup", 0, JseAddGroupWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("DelGroup", 0, JseDelGroupWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Add", 0, JseAddWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Del", 0, JseDelWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("DelByUri", 0, JseDelByUriWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Move", 0, JseMoveWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("SetMyInfo", 0, JseSetMyInfoWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Accept", 0, JseAcceptWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("Reject", 0, JseRejectWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetCount", JseGetCountRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetIndexByURI", JseGetIndexByURIRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetInfo", JseGetInfoRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("GetMyInfo", JseGetMyInfoRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("op.SetParam", 0, JseSetPropertyWrite);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("op.GetParam", JseGetPropertyRead, 0);
    regist(Call->name(), Call);	
    Call = new JseFunctionCall("op.DelByUri", 0, JseDelByUriWrite);
    regist(Call->name(), Call);	

}

JseHWFriends::~JseHWFriends()
{
}

#endif

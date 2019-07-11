#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "app_include.h"
#include "ind_mem.h"
#include "ntp/mid_ntp.h"
#include "mid/mid_http.h"
#include "mid/mid_msgq.h"
#include "mid/mid_timer.h"

#include "sys_msg.h"
#include "Session.h"

#ifdef TVMS_OPEN
#include "tvms.h"
#endif

#ifdef XMPP
#include "XmppService.h"
#endif

#ifdef HUAWEI_C20
#include "PPVListInfo.h"
#include "config/pathConfig.h"
#include <sstream>
#endif

#include "Assertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "NativeHandler.h"
#include "BrowserBridge/Huawei/BrowserEventQueue.h"
#include "customer.h"

static HEARTBIT *pHeartBit = NULL;
static int userstat = 10;
static int uservalid = 1;
static int serviceID = 0;
static int g_heartpause = 0;

extern char* global_cookies;

/*************************************************
Description:创建心跳接收结构体
Input:     无
output:   无
Return:	指向心跳结构体地址
 *************************************************/
static HEARTBIT *heartbit_create(void)
{
    HEARTBIT *h = (HEARTBIT*)IND_MALLOC(sizeof(HEARTBIT));

    if(h == NULL)
        return NULL;

    h->UserValid = 0;
    h->NextCallInterval = 0;
#ifdef HUAWEI_C20
    h->PVRVersion[0] = '\0';
    h->Channelver[0] = '\0';
    h->Bulletinver[0] = '\0';
    h->AdText[0] = '\0';
    h->Vnet = 0;
    h->MsgNum = 0;
    // h->Msg = NULL;
#endif
    return h;
}

/*************************************************
Description:删除心跳结构体
Input:     无
output:   无
Return:	指向心跳结构体地址
 *************************************************/
static int heartbit_delete(HEARTBIT* h)
{
    if(h != NULL) {
        IND_FREE(h);
    }
    return 0;
}

/*************************************************
Description:解析下发的心跳字符串
Input:     str: 指向下发字符串地址size:长度
output:   无
Return:	指向心跳结构体地址
 *************************************************/
static HEARTBIT* heartbit_parse(char* str, int size)
{
    char *lineText = NULL;
    char *tagText = NULL;
    char *valueText = NULL;
    HEARTBIT *HeartBit = NULL;
    lineText = (char*)IND_MALLOC(2048);
    tagText = (char*)IND_MALLOC(1024);
    valueText = (char*)IND_MALLOC(1024);

    while(1) {
        char *ptr = file_get_line(str, size, lineText);

        if(ptr == NULL)
            break;
        size -= (ptr - str);
        str = ptr;
        if(*lineText == '[') {
            if(line_get_bracket(lineText, (int)strlen(lineText), tagText) < 0)
                continue;
            line_to_cap(tagText, (int)strlen(tagText));
            if(!strcmp(tagText, "HEARTBIT")) {
                if(HeartBit != NULL)
                    heartbit_delete(HeartBit);
                HeartBit = heartbit_create();
            }
#ifdef HUAWEI_C20
            else if(!strcmp(tagText, "MESSAGE")) {
            }
#endif
            else if(!strcmp(tagText, "END")) {
                break;
            }
            continue;
        }
        else {
            if(line_get_tag(lineText, (int)strlen(lineText), tagText, valueText) < 0)
                continue;
            line_to_cap(tagText, (int)strlen(tagText));
            if(!strcmp(tagText, "USERVALID")) {
                if(HeartBit == NULL)
                    continue;
                line_to_cap(valueText, (int)strlen(valueText));
                if(!strcmp(valueText, "TRUE"))
                    HeartBit->UserValid = 1;
                else
                    HeartBit->UserValid = 0;
            }
#ifdef HUAWEI_C20
            else if(!strcasecmp(tagText, "PVRVer")) {
                if(HeartBit == NULL)
                    continue;

                PRINTF("PVRVer is %s\n", valueText);
                if(strlen(valueText) > 0 && strcasecmp(valueText, "null") != 0) {
                    IND_STRNCPY(HeartBit->PVRVersion, valueText, 15);
                }
                else {
                    HeartBit->PVRVersion[0] = '\0';
                }
                continue;
            }
            else if(!strcasecmp(tagText, "CHANNELVER")) {
                if(HeartBit == NULL)
                    continue;

                if(strlen(valueText) > 31) {
                    IND_STRNCPY(HeartBit->Channelver, valueText, 31);
                    HeartBit->Channelver[31] = '\0';
                }
                else
                    IND_STRCPY(HeartBit->Channelver, valueText);
                continue;
            }
            else if (!strcasecmp(tagText, "ppvver")) {
                if(HeartBit == NULL)
                    continue;
                if(strlen(valueText) > 31) {
                    IND_STRNCPY(HeartBit->Bulletinver, valueText, 31);
                    HeartBit->PPVversin[31] = '\0';
                }
                else
                    IND_STRCPY(HeartBit->PPVversin, valueText);
                continue;
            }
            else if(!strcmp(tagText, "BULLETINVER")) {
                if(HeartBit == NULL)
                    continue;

                if(strlen(valueText) > 31) {
                    IND_STRNCPY(HeartBit->Bulletinver, valueText, 31);
                    HeartBit->Bulletinver[31] = '\0';
                }
                else
                    IND_STRCPY(HeartBit->Bulletinver, valueText);
                continue;
            }
#endif
            else if(!strcmp(tagText, "NEXTCALLINTERVAL")) {
                if(HeartBit == NULL)
                    continue;

                HeartBit->NextCallInterval = (unsigned int)atoi(valueText);
            }
#ifdef HUAWEI_C20
            else if(!strcmp(tagText, "ADTEXT")) {
                if(HeartBit == NULL)
                    continue;

                if(strlen(valueText) > 1024) {
                    IND_STRNCPY(HeartBit->AdText, valueText, 1024);
                    HeartBit->AdText[1024] = '\0';
                }
                else
                    IND_STRCPY(HeartBit->AdText, valueText);
                continue;

            }
            else if(!strcmp(tagText, "VNET")) {
                if(HeartBit == NULL)
                    continue;

                line_to_cap(valueText, strlen(valueText));

                if(!strncmp(valueText, "YES", 3))
                    HeartBit->Vnet = 1;
                else
                    HeartBit->Vnet = 0;
            }
#endif
        }
    }
    IND_FREE(lineText);
    IND_FREE(tagText);
    IND_FREE(valueText);
    return HeartBit;
}

/*************************************************
Description:发起启动心跳消息命令
Input:     无
output:   无
Return:	无
 *************************************************/
static void heartbit_timeout(int arg)
{
    sendMessageToNativeHandler(MessageType_System, HEART_BIT_RUN, 0, 0);
}

/*************************************************
Description:启动心跳定时器
Input:     无
output:   无
Return:	无
 *************************************************/
static void heartbit_regist(void)//启动HeartBit定时器
{
    unsigned int interval;

    if(g_heartpause == 1)
        return;

    if(pHeartBit == NULL)
        interval = 900;
    else
        interval = pHeartBit->NextCallInterval;
    PRINTF("interval=%d\n", interval);
    if(mid_timer_create(interval, 1,  heartbit_timeout, 0) != 0)
        PRINTF("mid_timer_create ERROR\n");
}

static int heartbit_recv(int type, char* buf, int len, int arg)
{
    HEARTBIT *h = NULL;

    PRINTF("type = %d\n", type);
    if(buf == NULL)
        ERR_OUT("data is null!\n");

    //mid_tool_lines_show(p, 10);
    h = heartbit_parse(buf, strlen(buf));
    if(h == NULL)
        ERR_OUT("data parse error!\n");

    uservalid = h->UserValid;
    if(uservalid == 0) {
        heartbit_delete(h);
        sendMessageToKeyDispatcher(MessageType_Unknow, HEART_USER_INVALID, 0, 0);
        ERR_OUT("user unvalid!\n");
    }

#ifdef HUAWEI_C20
    channel_array_set_version(h->Channelver, 1);
#ifdef	INCLUDE_IPTV_cPVR
    if(strlen(h->PVRVersion) > 0) {
        PRINTF("heartbit_recv ser version, currently not supported\n");
        cpvrListHearbeatCheck(h->PVRVersion);
    }
#endif
    if(userstat == 10) {
        userstat = 2; /* 设置用户状态 */
        session().setUserStatus(userstat);
    }
#endif
    heartbit_delete(pHeartBit);
    pHeartBit = h;

    heartbit_regist();
    return 0;
Err:
    heartbit_regist();
    return -1;
}

/*************************************************
Description:发起请求心跳的命令
Input:     无
output:   无
Return:	0: 成功-1:失败
 **************************************************/
static int httpHeartBit_P(int arg)
{
#ifdef XMPP
    char xmppInfoUrl[URL_MAX_LEN + 4] = {0};
    if (isXmppInfoGet()) {
        PRINTF("Have not get XMPP parameter, prepare to send xmpp parameter request heartbit.\n");
        std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
        if (!url.empty()) {
            if (url.substr(url.length() - 1, 1).compare(std::string("/")) == 0) {
                url.erase(url.length() -1, 1);
        }
            snprintf(xmppInfoUrl, sizeof(xmppInfoUrl), "%s/EPG/jsp/GetXMPPInfo", url.c_str());
            if (mid_http_call(xmppInfoUrl, (mid_http_f)xmppInfoHeartbitRecv, 0, NULL, 0, global_cookies) < 0 )
                PRINTF("Http request error!\n");
        }
    } else
        PRINTF("Already get XMPP parameter, not need xmpp heartbit!\n");
#endif //XMPP

    g_heartpause = 0;
    std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();

#ifdef HUAWEI_C10
    /*http:// <EPG Server IP地址>:<端口>/EPG/jsp/GetHeartBit?UserStatus=<用户状态>
      &ChannelVer=<当前频道列表版本号>&STBID=<机顶盒唯一标识>
      &STBType=<机顶盒类型>&Version=<机顶盒版本号>
      */
    url += "EPG/jsp/GetHeartBit";
    url = Hippo::Customer().AuthInfo().ConcatenateUrl(url);

    PRINTF("EVENT_STB_HEARTBEAT\n");
    browserEventSend("{\"type\":\"EVENT_STB_HEARTBEAT\"}", NULL);
#else
    /*
    http://<EPG Server IP地址 >:端口/EPG/jsp/gethearbit.jsp?usertoken=<IP-STB用户令牌>
    &User=<IP-STB业务帐号>&pwd=<IP-STB帐号密码>&ip=<IP-STB的IP地址>
    &userstat=<用户状态>&serviceID=<当前播放节目ID>&Channelver=<当前频道列表版本号>
    &NTID=<机顶盒MAC地址>&Version=<机顶盒版本号>
    &lang=<显示语种>
    */
    url += "EPG/jsp/gethearbit.jsp";
    url = Hippo::Customer().AuthInfo().ConcatenateUrl(url);
    PRINTF("EVENT_STB_HEARTBEAT\n");
    browserEventSend("{\"type\":\"EVENT_STB_HEARTBEAT\"}", NULL);

#endif
    PRINTF("%s\n", url.c_str());
    if (mid_http_call(url.c_str(), (mid_http_f)heartbit_recv, 0, NULL, 0, global_cookies) < 0) {
        ERR_PRN("ERROR,GET heartbit is error\n");

#ifdef TVMS_OPEN
        if(mid_ntp_status() == 1) {
            init_tvms_msg();
        }
#endif
        return -1;
    }
#ifdef TVMS_OPEN
    if(mid_ntp_status() == 1) {
        init_tvms_msg();
    }
#endif
    return 0;
}

void httpHeartBit(int arg)
{
    httpHeartBit_P(arg);
}

/*************************************************
Description:清空心跳定时器
Input:     无
output:   无
Return:	0:成功
 *************************************************/
int httpHeartClr(void)
{
    g_heartpause = 1;
    mid_timer_delete(heartbit_timeout, 0);
    return 0;
}

static mid_msgq_t g_msgq = NULL;

void httpRequestInputMsg(const char *msg, int type)
{
    HTTPMESSAGE httpMessage;

    memset(&httpMessage, 0, sizeof(httpMessage));
    httpMessage.type = type;
    IND_STRCPY(httpMessage.msg, msg);
    mid_msgq_put(g_msgq, (char *)(&httpMessage), 2);
}

int httpRequestgetMsg(char *msg, int *type)
{
    HTTPMESSAGE httpMessage;
    int rc = 0;

    memset(&httpMessage, 0, sizeof(httpMessage));
    rc = mid_msgq_get(g_msgq, (char *)(&httpMessage), 0, 5000);

    IND_STRCPY(msg, httpMessage.msg);
    *type = httpMessage.type;
    return rc;
}

void httpRequestMsgCreat(void)
{
    g_msgq = mid_msgq_create(10, sizeof(HTTPMESSAGE));
}


#ifdef HUAWEI_C20

static char channelVersion[32] = {"NULL"};

int channel_array_get_version(char *pBuf, int bufLen)
{
    if(!pBuf) {
        LogUserOperError("channel_array_get_version\n");
        return -1;
    }
    int len = strlen(channelVersion);
    if(len >= bufLen) {
        LogUserOperError("len >= bufLen\n");
        return -1;
    }
    IND_MEMCPY(pBuf, channelVersion, len);
    return 0;
}

void channel_array_set_version(char *ver, int update)
{
    if(ver == NULL)
        return;
    LogUserOperDebug("Channel list version: current[%s], new[%s]\n", channelVersion, ver);
    if(strcmp(channelVersion, ver))
        IND_STRCPY(channelVersion, ver);
    else
        update = 0;
    if(update) {
        channel_array_request(0);
    }
}

void channel_array_request(int type)
{
    std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
    url += std::string("EPG/jsp/getchannellist.jsp");
    url = Hippo::Customer().AuthInfo().ConcatenateUrl(url);
    url += std::string("&channellisttype=");

    std::stringstream   oss;
    oss << type;

    url += oss.str();
    httpRequestInputMsg(url.c_str(), 1);

    ppvListInfo()->refreshPPVList();
    return;
}
#endif


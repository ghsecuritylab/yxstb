
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "app_include.h"
#include "app_reminderlist.h"

#include "sys_msg.h"
#include "sys_key_deal.h"

#include "MessageTypes.h"
#include "ind_mem.h"
#include "NativeHandler.h"
#include "BrowserBridge/Huawei/BrowserEventQueue.h"
#include "mid/mid_http.h"
#include "mid/mid_mutex.h"
#include "mid/mid_time.h"
#include "mid/mid_timer.h"
#include "mid/mid_tools.h"

#include "customer.h"

#define A2_INFO_LEN_MAX    1024

static mid_mutex_t		g_mutex	= NULL;

static char linetext[2048 + 4];
static char tagtext[64 + 4];
static char valuetext[2048 - 64 + 4];

static int g_interval = 30; // 分钟
static int g_poll_flag = 0;

static REMINDER_LIST*	refreshReminderlist	= NULL;
static REMINDER_LIST*	g_Reminderlist	= NULL;

extern char* global_cookies;

static REMINDER_LIST* reminder_list_create(void)
{
    REMINDER_LIST* elem = (REMINDER_LIST*)IND_MALLOC(sizeof(REMINDER_LIST));

    if(elem == NULL)
        return (NULL);
    IND_MEMSET(elem, 0, sizeof(REMINDER_LIST));
    elem->count = 0;
    elem->array = elem->_array;
    elem->size = VOD_REMINDER_SIZE;

    return elem;
}


static void reminder_list_add(REMINDER_LIST* list, VOD_REMINDER* elem)
{
    int	i, num;

    VOD_REMINDER			**array;

    if(list == NULL || elem == NULL)
        return;

    array = list->array;
    num = list->count;

    if(num >= VOD_REMINDER_SIZE)
        return;
    //重复丢弃
    for(i = 0; i < num; i ++) {
        if(!strcmp(array[i]->ReminderId, elem->ReminderId))
            ERR_OUT("repeat reminder\n");
    }

    if(list->count >= list->size) {
        array  = (VOD_REMINDER**)IND_MALLOC(sizeof(VOD_REMINDER) * list->size * 2);
        if(array == NULL)
            ERR_OUT("malloc remnider array\n");

        list->size *= 2;
        if(list->array != list->_array)
            IND_FREE(list->array);
        list->array = array;
    }

    list->array[list->count] = elem;
    list->count ++;

    return;

Err:
    IND_FREE(elem);
}

static void reminder_elem_delete(REMINDER_LIST*list, char* reminderid)
{
    int i, num;
    int tIndex = -1;
    VOD_REMINDER* elem;

    num = list->count;
    VOD_REMINDER** old_array;
    old_array = list->array;
    for(i = 0; i < num; i ++) {
        if(!strcmp(old_array[i]->ReminderId, reminderid)) {
            tIndex = i;
            elem = old_array[i];
            IND_FREE(elem);
            break;
        }
    }

    if(tIndex == -1) {
        return ;
    }
    num--;

    for(i = tIndex; i < num; i++) {
        old_array[i] = old_array[i + 1];
    }

    list->count -= 1;
}

static void reminder_list_delete(REMINDER_LIST* list)
{
    int i, num;
    VOD_REMINDER **array;

    if(list == NULL)
        return;

    num = list->count;
    array = list->array;
    for(i = 0; i < num; i ++)
        IND_FREE(array[i]);
    if(array != list->_array)
        IND_FREE(array);

    IND_FREE(list);
}

static void poll_vod_reminder(int)
{
    char curtime_str[16];
    char message[A2_INFO_LEN_MAX];
    unsigned int curtime_int;
    unsigned int starttime, stoptime;
    int i;
    IND_MEMSET(message , 0 , 128);
    mid_tool_time2string(mid_time(), curtime_str, 0);
    curtime_int = mid_tool_string2time(curtime_str);
    if(NULL == g_Reminderlist || g_Reminderlist->count == 0)
        return;
    if(1 == NativeHandlerGetState())
        return;
    PRINTF("\ncurrent time %s\n", curtime_str);
    for(i = 0; i < g_Reminderlist->count; i++) {
        starttime = mid_tool_string2time(g_Reminderlist->array[i]->ReminderTime);
        stoptime = mid_tool_string2time(g_Reminderlist->array[i]->ReminderExpireTime);
        if(!strcmp(g_Reminderlist->array[i]->ReminderTime, curtime_str)) {
            PRINTF("vod reminder ***********\n");
//			len = sprintf(message, "{\"type\":\"EVENT_UNIVERSAL_REMINDER\",");
//			len += snprintf(message+len, A2_INFO_LEN_MAX-len, "\"message\":\"%s,%s,%s,%s,%s,%d\"}", g_Reminderlist->array[i]->ReminderId,
//				g_Reminderlist->array[i]->ContentID, g_Reminderlist->array[i]->ContentType, g_Reminderlist->array[i]->ReminderTime,
//				g_Reminderlist->array[i]->ReminderExpireTime, g_Reminderlist->array[i]->ReminderType);
            //message[len] = '\0';

            /*{"type":"EVENT_UNIVERSAL_REMINDER",
            		"ReminderID":"001",
            		" ContentID":"12345",
            		" ContentType":"300",
            		" ReminderTime":"20100114200000",
            		" ReminderExpireTime":"20100120200000",
            		" ReminderType":"111",
            		" ExtendDescription":
            		" {\"ContentCode\":\"111\"}"}
            		*/
            snprintf(message, A2_INFO_LEN_MAX, "{\"type\":\"EVENT_UNIVERSAL_REMINDER\",\"ReminderID\":\"%s\",\"ContentID\":\"%s\",\"ContentType\":\"%s\",\"ReminderTime\":\"%s\",\"ReminderExpireTime\":\"%s\",\"ReminderType\":\"%d\",\"ExtendDescription\":\"%s\"}",
                     g_Reminderlist->array[i]->ReminderId,	\
                     g_Reminderlist->array[i]->ContentID,	\
                     g_Reminderlist->array[i]->ContentType,	\
                     g_Reminderlist->array[i]->ReminderTime,	\
                     g_Reminderlist->array[i]->ReminderExpireTime, \
                     g_Reminderlist->array[i]->ReminderType, \
                     g_Reminderlist->array[i]->ExtendDescription);
            browserEventSend(message, NULL);
            reminder_elem_delete(g_Reminderlist, g_Reminderlist->array[i]->ReminderId);
            i--;
            continue;
        }

        PRINTF("vod reminder: ReminderTime:%s,ReminderExpireTime:%s,count:%d,starttime:%d,curtime_int:%d,stoptime:%d\n", g_Reminderlist->array[i]->ReminderTime, g_Reminderlist->array[i]->ReminderExpireTime, g_Reminderlist->count, starttime, curtime_int, stoptime);
        if(curtime_int > starttime && curtime_int <= (stoptime + 6)) {
            PRINTF("vod reminder ***********\n");
//	        len = sprintf(message, "{\"type\":\"EVENT_UNIVERSAL_REMINDER\",");
//			len += snprintf(message+len, A2_INFO_LEN_MAX-len, "\"message\":\"%s,%s,%s,%s,%s,%d\"}", g_Reminderlist->array[i]->ReminderId,
//			g_Reminderlist->array[i]->ContentID, g_Reminderlist->array[i]->ContentType, g_Reminderlist->array[i]->ReminderTime,
//			g_Reminderlist->array[i]->ReminderExpireTime, g_Reminderlist->array[i]->ReminderType);
            //message[len] = '\0';

            snprintf(message, A2_INFO_LEN_MAX, "{\"type\":\"EVENT_UNIVERSAL_REMINDER\",\"ReminderID\":\"%s\",\"ContentID\":\"%s\",\"ContentType\":\"%s\",\"ReminderTime\":\"%s\",\"ReminderExpireTime\":\"%s\",\"ReminderType\":\"%d\",\"ExtendDescription\":\"%s\"}",
                     g_Reminderlist->array[i]->ReminderId,	\
                     g_Reminderlist->array[i]->ContentID,	\
                     g_Reminderlist->array[i]->ContentType,	\
                     g_Reminderlist->array[i]->ReminderTime,	\
                     g_Reminderlist->array[i]->ReminderExpireTime, \
                     g_Reminderlist->array[i]->ReminderType, \
                     g_Reminderlist->array[i]->ExtendDescription);
            browserEventSend(message, NULL);

            reminder_elem_delete(g_Reminderlist, g_Reminderlist->array[i]->ReminderId);
            i--;
            continue;

        }
        if(curtime_int > stoptime + 9) {
            PRINTF("vod expired ***********\n");
            reminder_elem_delete(g_Reminderlist, g_Reminderlist->array[i]->ReminderId);
            i--;
            continue;
        }
    }
    return;
}

void httpReminderRequestIntervalChange(int interval);
/******************lh pare vod reminder list
************************/
static REMINDER_LIST* reminder_list_parse(char* str, int len)
{
    char*			p;
    VOD_REMINDER*		elem = NULL;
    REMINDER_LIST*   remderlist = NULL;
    char* g_lineText = linetext;
    char* g_tagText = tagtext;
    char* g_valueText = valuetext;
    PRINTF("*********str:%s\n", str);
    remderlist = reminder_list_create();
    if(remderlist == NULL)
        return NULL;

    while(1) {
        p = file_get_line(str, len, g_lineText);
        len -= (p - str);
        str = p;

        PRINTF("g_lineText=%s\n", g_lineText);
        if(p == NULL) {
            PRINTF("p == NULL\n");
            if(elem) {
                PRINTF("*******here\n");
                reminder_list_add(remderlist, elem);
                elem = NULL;
            }
            break;
        }

        if(*g_lineText == '[') {
            /* reminder列表尾部 */
            PRINTF("****^^^^^^here\n");
            if(!strcmp(g_tagText, "END")) {
                if(elem) {
                    PRINTF("****here\n");
                    reminder_list_add(remderlist, elem);
                    // elem = NULL;
                }
                break;
            }

            if(!strcasecmp(g_tagText, "HeartBit"))
                continue;
            if(!strcasecmp(g_tagText, "Reminder"))
                continue;
        }

        if(line_get_tag(g_lineText, strlen(g_lineText), g_tagText, g_valueText) < 0)
            continue;

        if(!strcmp(g_tagText, "Interval")) {
            httpReminderRequestIntervalChange(atoi(g_valueText));
            PRINTF("g_interval:%d\n", g_interval);
            continue;
        }

        if(!strcmp(g_tagText, "ReminderNum")) {
            PRINTF("ReminderNum:%d\n", atoi(g_valueText));
            if(atoi(g_valueText) == 0)
                break;
            continue;
        }

        if(!strcmp(g_tagText, "ReminderID")) {
            if(elem)
                reminder_list_add(remderlist, elem);

            elem = (VOD_REMINDER*)IND_MALLOC(sizeof(VOD_REMINDER));
            if(elem == NULL) {
                PRINTF("ERROR: not enough memory\n");
                break;
            }
            IND_MEMSET(elem, 0, sizeof(VOD_REMINDER));
            if(strlen(g_valueText) > 63) {
                IND_STRNCPY(elem->ReminderId, g_valueText, 63);
                elem->ReminderId[63] = '\0';
            } else
                IND_STRCPY(elem->ReminderId, g_valueText);
            PRINTF("ReminderId:%s\n", elem->ReminderId);
            continue;
        }
        if(elem == NULL)
            continue;

        if(!strcmp(g_tagText, "ContentID")) {
            if(strlen(g_valueText) > 11) {
                IND_STRNCPY(elem->ContentID, g_valueText, 11);
                elem->ContentID[11] = '\0';
            } else
                IND_STRCPY(elem->ContentID, g_valueText);
            PRINTF("ContentID:%s\n", elem->ContentID);
            continue;
        }

        if(!strcmp(g_tagText, "ContentType")) {
            if(strlen(g_valueText) > 11) {
                IND_STRNCPY(elem->ContentType, g_valueText, 11);
                elem->ContentType[11] = '\0';
            } else
                IND_STRCPY(elem->ContentType, g_valueText);
            PRINTF("ContentType:%s\n", elem->ContentType);
            continue;
        }

        if(!strcmp(g_tagText, "ReminderType")) {
            elem->ReminderType = atoi(g_valueText);
            PRINTF("ReminderType:%d\n", elem->ReminderType);
            continue;
        }

        if(!strcmp(g_tagText, "ExtendDescription")) {
            IND_STRNCPY(elem->ExtendDescription, g_valueText, 2048);
            PRINTF("ExtendDescription:%s\n", elem->ExtendDescription);
            continue;
        }

        if(!strcmp(g_tagText, "ReminderTime")) {
            IND_STRNCPY(elem->ReminderTime, g_valueText, 16);
            PRINTF("ReminderTime:%s\n", elem->ReminderTime);
            continue;
        }

        if(!strcmp(g_tagText, "ReminderExpireTime")) {
            IND_STRNCPY(elem->ReminderExpireTime, g_valueText, 16);
            PRINTF("ReminderExpireTime:%s\n", elem->ReminderExpireTime);
            continue;
        }
    }
    IND_FREE(elem);
    return remderlist;
}
static void reminder_list_recv(int type, char* buf, int len, int arg)
{



    REMINDER_LIST* reminderlist;

    if(buf == NULL || len <= 0)
        return;

    reminderlist = reminder_list_parse(buf, strlen(buf));
    if(reminderlist == NULL)
        ERR_OUT("parse failed\n");

    if(g_mutex == NULL)
        g_mutex = mid_mutex_create();
    if(mid_mutex_lock(g_mutex))
        WARN_PRN("\n");
    if(refreshReminderlist)
        reminder_list_delete(refreshReminderlist);
    refreshReminderlist = reminderlist;
    mid_mutex_unlock(g_mutex);
    sendMessageToNativeHandler(MessageType_System, REMINDER_LIST_OK, 0, 0);
    if(0 == g_poll_flag) {

        if(mid_timer_create(3, 0, poll_vod_reminder, 0) == 0) {
            g_poll_flag = 1;
            PRINTF("set g_poll_flag=1\n");
        }
    }
Err:
    return ;
}
void reminder_list_sync(void)
{

    VOD_REMINDER **array, *elem, *newelem;
    int i, num;

    VOD_REMINDER **old_array, *old_elem;
    int old_i, old_num;

    if(mid_mutex_lock(g_mutex))
        WARN_PRN("\n");
    if(refreshReminderlist == NULL || refreshReminderlist->count == 0) {
        PRINTF("not refresh***********\n");
        if(refreshReminderlist) {
            reminder_list_delete(refreshReminderlist);
        }
        goto End;
    }

    num = refreshReminderlist->count;
    array = refreshReminderlist->array;


    if(g_Reminderlist == NULL) {
        g_Reminderlist = refreshReminderlist;
        PRINTF("***********go to end \n");
        goto End;
    }

    reminder_list_delete(g_Reminderlist);
    g_Reminderlist = refreshReminderlist;
    goto End;
    /**linhui**/
    /*每次刷新reminder的时候将老的reminder删除*/
#if 0

    for(i = 0; i < num ; i++) {
        //PRINTF("%d,%s,%s\n",num,refreshReminderlist->array[i]->ReminderId,refreshReminderlist->array[i]->ReminderTime);
    }
    if(num == 0) {
        reminder_list_delete(g_Reminderlist);
        g_Reminderlist = refreshReminderlist;
        goto End;
    }

    elem = NULL;
    old_num = g_Reminderlist->count;
    old_array = g_Reminderlist->array;
    for(i = 0; i < old_num ; i++) {
        PRINTF("%d,%s,%s\n", old_num, g_Reminderlist->array[i]->ReminderId, g_Reminderlist->array[i]->ReminderTime);
    }

    for(old_i = 0; old_i < old_num; old_i ++) {
        old_elem = old_array[old_i];
        if(old_elem == NULL)
            continue;
        //PRINTF("old:%s,%s\n",old_elem->ReminderId,old_elem->ReminderTime);

        for(i = 0; i < num; i ++) {
            elem = array[i];
            // PRINTF("new:%s,%s\n",elem->ReminderId,elem->ReminderTime);
            if(!strcmp(elem->ReminderId, old_elem->ReminderId)) {
                IND_STRCPY(elem->ReminderId, old_elem->ReminderId);
                IND_STRCPY(elem->ContentID, old_elem->ContentID);
                elem->ReminderType = old_elem->ReminderType;
                IND_STRCPY(elem->ContentType, old_elem->ContentType);
                IND_STRCPY(elem->ReminderTime, old_elem->ReminderTime);
                IND_STRCPY(elem->ReminderExpireTime, old_elem->ReminderExpireTime);
                old_elem = NULL;
                break;
            }

        }
        if(old_elem) {
            PRINTF("add :%s,%s\n", old_elem->ReminderTime, old_elem->ReminderExpireTime);
            newelem = (VOD_REMINDER*)IND_MALLOC(sizeof(VOD_REMINDER));
            if(newelem == NULL) {
                PRINTF("ERROR: not enough memory\n");
                break;
            }
            IND_MEMSET(newelem, 0, sizeof(VOD_REMINDER));
            IND_STRCPY(newelem->ReminderId, old_elem->ReminderId);
            IND_STRCPY(newelem->ContentID, old_elem->ContentID);
            newelem->ReminderType = old_elem->ReminderType;
            IND_STRCPY(newelem->ContentType, old_elem->ContentType);
            IND_STRCPY(newelem->ReminderTime, old_elem->ReminderTime);
            IND_STRCPY(newelem->ReminderExpireTime, old_elem->ReminderExpireTime);
            reminder_list_add(refreshReminderlist, newelem);
        }
    }
    reminder_list_delete(g_Reminderlist);
    g_Reminderlist = refreshReminderlist;
#endif
End:
    refreshReminderlist = NULL;
    mid_mutex_unlock(g_mutex);
}


void httpReminderRequest(int arg)
{
    std::string url = Hippo::Customer().AuthInfo().AvailableEpgUrlWithoutPath();
    if (url.empty())
        return;
    url += "EPG/jsp/getreminderlist.jsp";
    if (mid_http_call(url.c_str(), (mid_http_f)reminder_list_recv, 0, NULL, 0, global_cookies) != 0)
        ERR_OUT("mid_http_call failed\n");
Err:
    mid_timer_create(g_interval * 60, 0, httpReminderRequest, 0);
}

void httpReminderRequestIntervalChange(int interval)
{
    if (interval != g_interval) {
        mid_timer_delete_all(httpReminderRequest);
        g_interval = interval;
        mid_timer_create(g_interval * 60, 0, httpReminderRequest, 0);
    }
}


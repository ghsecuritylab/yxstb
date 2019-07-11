
#include "JseHWArrayTimer.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "app_include.h"
#include "sys_msg.h"

#include "ntp/mid_ntp.h"
#include "mid/mid_time.h"
#include "mid/mid_timer.h"
#include "mid/mid_tools.h"

#include "ind_mem.h"

#include "BrowserBridge/Huawei/BrowserEventQueue.h"

#define APP_TIME_MAX 64
#define APP_TIME_ARRAY_MAX 64

typedef struct _TIMER_ {
    char array_time_id[20];
    char index[20];
    char time[16];
    char description[512 + 4];
} TIMER;

typedef struct _ARRAY_NODE_ {
    char ID[20];
    int start;
    int count;
} ARRAY_NODE;

typedef struct _ARRAY_TIMER_ {
    int count;
    ARRAY_NODE array[APP_TIME_ARRAY_MAX];
} ARRAY_TIMER;

static TIMER *g_timer[APP_TIME_ARRAY_MAX]; // operate array
static int g_timer_count = 0;
static ARRAY_TIMER g_array_timer;	//operate bu array
static int g_array_timer_flag = 0;

static void timer_sync2_array(void);
static void timer_poll_stop(void);

static void poll_array_timer(int flag)
{
    int i = 0, j = 0;
    int update_flag = 0;
    char curtime[16] = {0};
    unsigned int cur_time = mid_time();
    unsigned int timer;

    mid_tool_time2string(mid_time(), curtime, 0);
    //PRINTF("current time %s\n", curtime);

    while(1) {
        if (g_timer_count <= 0 || i >= g_timer_count)
            break;
        timer = mid_tool_string2time(g_timer[i]->time);
        if (cur_time >= (timer - 1) && cur_time <= (timer - 1)) {
            char event[1024] = {0};
            snprintf(event, 1024, "{\"type\":\"EVENT_ARRAY_TIMER\",\"ArrayTimerID\":\"%s\",	\
				\"Index\":\"%s\",\"Time\":\"%s\",\"Description\":\"%s\"}", g_timer[i]->array_time_id,
                     g_timer[i]->index, g_timer[i]->time, g_timer[i]->description);
            browserEventSend(event, NULL);

            update_flag = 1;

            if (g_timer[i])
                IND_FREE(g_timer[i]);
            for (j = i; j < (g_timer_count - 1); j++)
                g_timer[j] = g_timer[j + 1];
            g_timer[j + 1] = 0;
            g_timer_count--;
        } else if (timer < (cur_time - 1)) {	//expired
            update_flag = 1;
            if (g_timer[i])
                IND_FREE(g_timer[i]);
            for(j = i; j < (g_timer_count - 1); j++)
                g_timer[j] = g_timer[j + 1];
            g_timer[j + 1] = 0;
            g_timer_count--;
        } else
            i++;
    }
    if( update_flag)
        timer_sync2_array();
    if (g_timer_count <= 0)
        timer_poll_stop();
    return;
}

static void timer_poll_start(void)
{
    if (mid_ntp_status() <= 0)
        ERR_OUT("ntp not synced!");
    if (0 == g_array_timer_flag) {
        LogJseDebug("timer poll start!\n");
        g_array_timer_flag = 1;
        mid_timer_create(1, 0, poll_array_timer, 0);
    }
Err:
    return;
}

static void timer_poll_stop(void)
{
    if (1 == g_array_timer_flag) {
        LogJseDebug("timer poll stop!\n");
        mid_timer_delete(poll_array_timer, 0);
        g_array_timer_flag = 0;
    }
}

static void timer_print(void)
{
    int i;

    LogJseDebug("TIMER\n");
    for(i = 0; i < g_timer_count; i++) {
        PRINTF("index       : %d\n", i);
        PRINTF("id          : %s\n", g_timer[i]-> array_time_id);
        PRINTF("index       : %s\n", g_timer[i]-> index);
        PRINTF("time        : %s\n", g_timer[i]-> time);
        PRINTF("description : %s\n", g_timer[i]-> description);
    }
    return;
}

static void timer_array_print(void)
{
    int i;
    int count = g_array_timer.count;

    LogJseDebug("TIMER array\n");
    for(i = 0; i < count; i++) {
        PRINTF("index   : %d\n", i);
        PRINTF("id      : %s\n", g_array_timer.array[i].ID);
        PRINTF("start   : %d\n", g_array_timer.array[i].start);
        PRINTF("count   : %d\n", g_array_timer.array[i].count);
    }
    return;
}

static void timer_sync2_array(void)
{
    int i, j;

    memset(&g_array_timer, 0, sizeof(ARRAY_TIMER));
    for (i = 0; i < g_timer_count; i++) {
        TIMER *time_node = g_timer[i];
        ARRAY_NODE *array = 0;
        int array_count = g_array_timer.count;
        for (j = 0; j < array_count; j++) {
            if (!strcmp(g_array_timer.array[j].ID, time_node->array_time_id))
                break;
        }
        array = &(g_array_timer.array[j]);
        if (j >= array_count) {	// add a new array
            IND_STRCPY(array->ID, time_node->array_time_id);
            array->start = i;
            array->count = 1;
            g_array_timer.count += 1;
        } else {	//insert to a exist array
            int k;
            array->count += 1;
            for(k = j + 1; k < array_count; k++) {
                array = &(g_array_timer.array[k]);
                array->start += 1;
            }
        }
    }
    timer_array_print();
    return;
}


static int timer_node_add(TIMER * timer)
{
    int i, j;
    int array_count;
    ARRAY_NODE *array = 0;
    int update_flag = 0;

    if(!timer)
        ERR_OUT("timer is NULL!\n");

    array_count  = g_array_timer.count;
    for (i = 0; i < array_count; i++) {	// array
        array = &(g_array_timer.array[i]);
        if (!strcmp(array->ID, timer->array_time_id)) {
            LogJseDebug("the same id:%s", array->ID);
            break;
        }
    }

    if (i >= array_count) {	//new array
#ifdef HUAWEI_C20
        //STB do nothing if the array does not exist
        ERR_OUT("no this array[%s]\n", timer->array_time_id);
#else
        if (g_timer_count >= APP_TIME_ARRAY_MAX)
            ERR_OUT("timer array is full\n");
        g_timer[g_timer_count] = timer;
        g_timer_count++;
        update_flag = 1;
#endif
    }
    else {
        for (j = array->start; j < (array->start + array->count); j++) {
            if (!strcmp(g_timer[j]->index, timer->index))
                break;
        }

        if (j >= (array->start + array->count)) {//add a new node
            int k;
            if (g_timer_count >= APP_TIME_ARRAY_MAX)
                ERR_OUT("timer array is full\n");
            for (k = g_timer_count; k > j; k--)
                g_timer[k] = g_timer[k - 1];
            g_timer[j] = timer;
            g_timer_count++;
            update_flag = 1;
        }
        else {	//refresh a node
            IND_MEMCPY(g_timer[j], timer, sizeof(TIMER));
            timer_print();
        }
    }

    if (update_flag) {
        timer_print();
        timer_sync2_array();
    }
    if (g_timer_count > 0)
        timer_poll_start();

    return 0;
Err:
    if (timer) {
        IND_FREE(timer);
        timer = 0;
    }
    return -1;
}

static int timer_node_delete(TIMER * timer)
{
    int i;

    if (!timer)
        ERR_OUT("NULL pointer!\n");

    for (i = 0; i < g_timer_count; i++) {
        if (!strcmp(g_timer[i]->index, timer->index))
            break;
    }

    if (i >= g_timer_count)
        ERR_OUT("No this node:%s!\n", timer->index);

    if (g_timer[i])
        IND_FREE(g_timer[i]);

    for (; i < (g_timer_count - 1); i++)
        g_timer[i] = g_timer[i + 1];
    g_timer_count--;
    timer_sync2_array();
    if(g_timer_count <= 0)
        timer_poll_stop();
    timer_print();
    return 0;
Err:
    return -1;
}

static int json_parse_time(const char *buf, TIMER * timer)
{
    char str[IPANEL_BUF_SIZE + 4] = {0};
    char *p, *head, *tag, *value;
    int len = 0;

    if (!buf || !timer)
        ERR_OUT("NULL pointer!\n");

    len = strlen(buf);
    if (len <= 0 || len > IPANEL_BUF_SIZE)
        ERR_OUT("str length is [%d]:%s\n", len, buf);
    IND_STRCPY(str, buf);
    len--;
    if (str[len] == '}')
        str[len] = '\0';	// delete charactor '}' at the end of JSON

    head = str;

    while(head) {
        tag = head;
        p = strchr(head, ',');
        if(p) {
            *p = '\0';
            head = p + 1;
        }
        else
            head = 0;

        while(*tag) {	//filter { and " and space
            if('{' != *tag && '\"' != *tag && ' ' != *tag)
                break;
            tag++;
        }

        p = strchr(tag, ':');
        if(!p) {
            LogJseDebug("invalid str:%s\n", tag);
            continue;
        }
        value = p + 1;
        while(*p) {
            if(':' != *p && '\"' != *p && ' ' != *p)
                break;
            *p = '\0';
            p--;
        }

        while(*value) {	//filter { and " and space
            if('\"' != *value && ' ' != *value)
                break;
            value++;
        }

        if(!strcmp(tag, "Description")) {	//\"Description\":\"{\'id\':1,\'name\':1}\"
            if(head) {
                *(head - 1) = ',';
                head = strchr(head, '\"');
                if(head) {
                    *head = '\0';
                    head++;
                }
            }
        }
        p = value + strlen(value) - 1;
        while(*p) {
            if('\"' != *p && ' ' != *p)	//'}' != *p &&
                break;
            *p = '\0';
            p--;
        }

        if(!strcmp(tag, "ArrayTimerID"))
            IND_STRNCPY(timer->array_time_id, value, 20);
        else if(!strcmp(tag, "Index"))
            IND_STRNCPY(timer->index, value, 20);
        else if(!strcmp(tag, "Time"))
            IND_STRNCPY(timer->time, value, 16);
        else if(!strcmp(tag, "Description"))
            IND_STRNCPY(timer->description, value, 512);
        else
            LogJseDebug("invalid tag[%s] with value[%s]\n", tag, value);
    }
    return 0;
Err:
    return -1;
}

/***************************
*add a timer(or refresh)
*input:timer info, JSON format
*eg.{"ArrayTimerID":"start","Index":"0","Time":"20100726123000","Description":"{'id':1,'name':1}"}
    {"ArrayTimerID":"start","Index":"0","Time":"20100804162501","Description":"{'id':9806,'name':'kkkk37'}"}
    {"ArrayTimerID":"start","Index":"0","Time":"20100804163001","Description":"{'id':9875,'name':'lll38'}"}
*output 0:OK, -1:ERROR
***************************/
static int JseTimerNodeAddWrite(const char *param, char *value, int len)
{
    TIMER *timer = 0;

    if (!value)
        ERR_OUT("NULL pointer!\n");
    LogJseDebug("run here:%s\n", value);

    if (!(timer = (TIMER *)IND_MALLOC(sizeof(TIMER))))
        ERR_OUT("malloc error!\n");

    IND_MEMSET(timer, 0, sizeof(TIMER));
    if (json_parse_time(value, timer)) {
        PRINTF("timer info error:%s!\n", value);
        if (timer)
            IND_FREE(timer);
        return -1;
    }
    return timer_node_add(timer);
Err:
    return -1;
}

/***************************
*delete a timer
*input:timer info, JSON format
*eg.{"ArrayTimerID":" BTVArrayTimer","Index":"aaa"}
*output 0:OK, -1:ERROR
***************************/
static int JseTimerNodeDeleteWrite(const char *param, char *value, int len)
{
    TIMER timer;

    if (!value)
        ERR_OUT("NULL pointer!\n");
    LogJseDebug("run here:%s\n", value);

    if (json_parse_time(value, &timer))
        ERR_OUT("timer info error:%s!\n", value);

    return timer_node_delete(&timer);
Err:
    return -1;
}

/***************************
*delete a timer
*input: timer array id
*output 0:OK, -1:ERROR
***************************/
static int JseTimerArrayAddWrite(const char *param, char *value, int len)
{
    if (!value)
        ERR_OUT("NULL pointer!\n");
    LogJseDebug("run here:%s\n", value);
#ifdef HUAWEI_C20
    int i;

    for (i = 0; i < g_array_timer.count; i++) { //check array
        if (!strcmp(g_array_timer.array[i].ID, value))
            ERR_OUT("there have the same array!\n");//do nothing if the array exist
    }

    if (i >= g_array_timer.count) { //create new array
        if (g_timer_count >= APP_TIME_ARRAY_MAX)
            ERR_OUT("timer array is full\n");
        strcmp(g_array_timer.array[g_array_timer.count].ID, value);
        g_array_timer.array[g_array_timer.count].start = 0;
        g_array_timer.array[g_array_timer.count].count = 0;
        g_array_timer.count++;
    }
#endif
    return 0;
Err:
    return -1;
}

/***************************
*delete a timer
*input: timer array id
*output 0:OK, -1:ERROR
***************************/
static int JseTimerArrayDeleteWrite(const char *param, char *value, int len)
{
    int i = 0, start = 0, count = 0;

    if (!value)
        ERR_OUT("NULL pointer!\n");
    PRINTF("run here:%s\n", value);

    for (i = 0; i < g_array_timer.count; i++) {
        if(!strcmp(g_array_timer.array[i].ID, value))
            break;
    }

    if (i >= g_array_timer.count)
        ERR_OUT("No this array:%s\n", value);

    start = g_array_timer.array[i].start;
    count = g_array_timer.array[i].count;

    for (i = start; i < (start + count); i++) {
        if (g_timer[i])
            IND_FREE(g_timer[i]);
        if ((i + count) <= g_timer_count)
            g_timer[i] = g_timer[i + count];
        else
            g_timer[i] = 0;
    }
    g_timer_count -= count;
    timer_sync2_array();
    if(g_timer_count <= 0)
        timer_poll_stop();
    return 0;
Err:
    return -1;
}

/*************************************************
Description: 初始化并注册华为定义的接口 <Tools.ArrayTimer.***>
接口相关说明见 《IPTV 海外版本STB与EPG接口文档 STB公共能力(Webkit) V1.1》
Input: 无
Return: 无
 *************************************************/
JseHWArrayTimer::JseHWArrayTimer()
	: JseGroupCall("ArrayTimer")
{
    JseCall *call;

    call = new JseFunctionCall("New", 0, JseTimerArrayAddWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("Release", 0, JseTimerArrayDeleteWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("AddNode", 0, JseTimerNodeAddWrite);
    regist(call->name(), call);

    call = new JseFunctionCall("DelNode", 0, JseTimerNodeDeleteWrite);
    regist(call->name(), call);
}

JseHWArrayTimer::~JseHWArrayTimer()
{
}


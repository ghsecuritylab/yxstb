#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "app_include.h"

#include "sys_msg.h"
#include "MessageTypes.h"

#include "browser_event.h"
#include "sys_key_deal.h"

#include "config/pathConfig.h"

#define PRESSURE_KEY_NUM	7
#define PRESSURE_CONFIG_FILE DEFAULT_MODULE_PRESSURE_DATAPATH"/yx_config_pressure.ini"
static int g_pressure_flag	= 0;
static int g_refuse_irkey = 0;

static const int g_pressure_array[PRESSURE_KEY_NUM] = {
    EIS_IRKEY_PAUSE,	//play
    EIS_IRKEY_PLAY,		//play
    EIS_IRKEY_LEFT,		//back
    EIS_IRKEY_RIGHT,	//back
    EIS_IRKEY_LEFT,		//backward
    EIS_IRKEY_LEFT,		//backward
    EIS_IRKEY_CHANNEL_UP,//swtich
};
static int g_pressure_keynum = 0;
static int g_pressure_ary[160];
static int g_pressure_interval = 10;
static int g_pressure_rc_forbid = 0;
static int g_pressure_index = 0;
static int g_pressure_rc_rand = 0;


int app_pressureTest_refuseIrkey()
{
    return g_refuse_irkey;
}

static void app_pressureTest_timeout(int arg)
{
    /**linhui ËÍËæ»úÖµ²âÊÔ**/
    if(g_refuse_irkey == 0 && g_pressure_rc_forbid) {
        g_refuse_irkey = 1;
    }

    if(!g_pressure_rc_rand) {
        int index_max = 0;
        if(g_pressure_keynum != 0) {
            sendMessageToKeyDispatcher(MessageType_Unknow, g_pressure_ary[g_pressure_index], 0, 0);
            index_max = g_pressure_keynum;
        } else {
            sendMessageToKeyDispatcher(MessageType_Unknow, g_pressure_array[g_pressure_index], 0, 0);
            index_max = PRESSURE_KEY_NUM;
        }
        g_pressure_index ++;
        if(g_pressure_index >= index_max)
            g_pressure_index = 0;
    } else {
#include<time.h>
#include<stdlib.h>
        int j;
        double index_max;
        if(g_pressure_keynum != 0) {
            index_max = g_pressure_keynum;
            srand((int)time(0));
            j = (int)(index_max * rand() / (RAND_MAX + 1.0));

            sendMessageToKeyDispatcher(MessageType_Unknow, g_pressure_ary[j], 0, 0);
        } else {
            index_max = PRESSURE_KEY_NUM;
            srand((int)time(0));
            j = (int)(index_max * rand() / (RAND_MAX + 1.0));

            sendMessageToKeyDispatcher(MessageType_Unknow, g_pressure_array[j], 0, 0);
        }

    }
}

void app_pressureTest_timer(void)
{
    g_pressure_index = 0;
    mid_timer_create(g_pressure_interval, 0, app_pressureTest_timeout, 0);
}

void app_pressureTest_clear(void)
{
    mid_timer_delete(app_pressureTest_timeout, 0);
    g_pressure_index = 0;
    g_refuse_irkey = 0;
}

void app_pressureTest_set_para(int interval, int keynum, char *key_str)
{
    int i;
    char *s_str = NULL;

    if(key_str == NULL)
        return;
    s_str = key_str;
    g_pressure_interval 	= interval;
    g_pressure_keynum 	= 0;

    for(i = 0; i < keynum; i++) {
        g_pressure_ary[i] = atoi(s_str);
        g_pressure_keynum++;
        s_str = strchr(s_str, ';');
        if(s_str == NULL)
            break;
        s_str++;
    }

}

static void app_pressureTest_set_key(char *key_str)
{
    char *s_str = NULL;

    if(key_str == NULL)
        return;
    s_str = key_str;
    g_pressure_keynum 	= 0;

    while(*s_str) {
        g_pressure_ary[g_pressure_keynum++] = atoi(s_str);
        s_str = strchr(s_str, ';');
        if(s_str == NULL)
            break;
        s_str++;
    }

}

int app_pressureTest_get_keynum(void)
{
    return g_pressure_keynum;
}

void app_pressureTest_set_flag(int flag)
{
    FILE *fp = NULL;
    char *p = NULL;
    char buf[2048] = {0};

    fp = fopen(PRESSURE_CONFIG_FILE, "rb");
    if(NULL == fp)
        ERR_OUT("open file error %s\n", PRESSURE_CONFIG_FILE);

    while(fgets(buf, 2048, fp)) {
        p = strstr(buf, "\n");
        if(p != NULL)
            *p = '\0';

        if(!strncmp(buf, "rc_forbit=", 10)) {
            g_pressure_rc_forbid = atoi(buf + 10);
            PRINTF("rc_forbit: %s\n", buf);
        } else if(!strncmp(buf, "rc_rand=", 8)) {
            g_pressure_rc_rand = atoi(buf + 8);
            PRINTF("rc_rand: %s\n", buf);
        } else if(!strncmp(buf, "press_interval=", 15)) {
            g_pressure_interval = atoi(buf + 15);
            PRINTF("press_interval: %s\n", buf);
        } else if(!strncmp(buf, "press_key=", 10)) {
            app_pressureTest_set_key(buf + 10);
            PRINTF("press_key: %s\n", buf);
        } else
            PRINTF("undefined cmd: %s\n", buf);

    }
    fclose(fp);

Err:
	PRINTF("flag=%d\n",flag);
    if(flag) {
        g_pressure_flag = 1;
        NativeHandlerSetState(2); // Æô¶¯Ñ¹Á¦²âÊÔ
    } else{
        g_pressure_flag = 0;
    	}
	int pressure_test_flag = app_pressureTest_get_flag();
	if(pressure_test_flag == 1){
		//app_graph_pressuretest_show();
    		sendMessageToEPGBrowser(MessageType_Prompt, 6, 1, 0);
		app_pressureTest_timer();
	}else if(pressure_test_flag == 0){
    		sendMessageToEPGBrowser(MessageType_Prompt, 6, 0, 0);
		//app_pressureTest_clear();
	}
    return;
}

int app_pressureTest_get_flag()
{
    return g_pressure_flag;
}



void app_pressureTest_deal_key(char *cmd_str)
{

    if(strncmp(cmd_str, "Start", 5) == 0) {
        int interval = 15, key_num = 0;

        if(app_pressureTest_get_keynum() > 0)
            return;

        char *s_str = cmd_str + 6;
        key_num = atoi(s_str);

        s_str = strchr(s_str, ':');
        if(s_str == NULL)
            ERR_OUT("%s \n", cmd_str);
        s_str++;
        interval = atoi(s_str);

        s_str = strchr(s_str, ':');
        if(s_str == NULL)
            ERR_OUT("%s \n", cmd_str);
        s_str++;

        app_pressureTest_set_para(interval, key_num, s_str);
        app_pressureTest_timer();
        return;
    } else if(strncmp(cmd_str, "End", 3) == 0) {
        if(app_pressureTest_get_keynum() <= 0)
            return;
        app_pressureTest_clear();
        app_pressureTest_set_para(15, 0, "");
    }
Err:
    return;

}


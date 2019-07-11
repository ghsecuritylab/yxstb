#include "tvms.h"
#include "tvms_define.h"
#include "tvms_setting.h"
#include "TvmsAssertions.h"
#include "mid/mid_tools.h"

#include "UltraPlayer.h"
#include "SystemManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h> //for some marco: FD_ZERO

#define TVMS_DEBUG() TVMS_LOG("##TVMS##############[%s:%s:%d]\n ", __FILE__, __FUNCTION__, __LINE__)
#define RAND_PASSTIME 2000 //����ֵû�����壬ֻ��Ҫ�������
/*************************************************************************
							����tvms_request_loop����
**************************************************************************/
static 	void* 		tvms_msg_queue_mutex = NULL;
static 	tvms_yxmsgq_t			g_tvms_request_queue = NULL;
static	int				g_tvms_request_fd;
static	int  				stanby_tvms = 0;
static 	TVMSLIST 		*tvms_list = NULL;


static 	int 				modify_tvms_delaytime(TVMS *h, int delay_time);
static 	TVMSLIST * 		tvmslist_creat(void);
static 	int 				compare_time(unsigned int passtime, unsigned int currenttime);
static 	int 				modify_tvms_showtime(TVMS *h);
static	int 				get_user_info(char *userid);
static 	void 				tvms_free(TVMS **p);
static 	void 				unlink_tvms_elem(TVMS **head, TVMS *p);
static 	void 				add_scan_to_msglist();
/*****************************************************
*     ����ʱ��ѭ������
******************************************************/
char* tvms_type[4] = {
    "NOTIFY_TVMS_MSG",
    "IPTV_TVMS_MSG",
    "VOD_TVMS_MSG",
    "BEGIN_TVMS_MSG",
};
extern "C" void
printf_tvms_msg(char *str, TVMS *p)
{
    TVMS_LOG("\n");
    TVMS_LOG("%s:msgcode = %s ,msgtype = %s,send_time = %s\n", str, p->msgTaskCode, tvms_type[p->taskType], p->sendTime);
    TVMS_LOG("%s:url = %s,priority = %d,confirmflag =%d,\n", str, p->displayURL, p->priority, p->confirmFlag);
    TVMS_LOG("%s:mediacode  = %s,mode = %d=%d=%d \n", str, p->mediaCode, p->mode, tvms_mid_time(), p->pass_time);
    TVMS_LOG("\n");
}


/*���ڼ���ʱ����������Լٶ�2���ڵ�TVMS��Ϣ���Ǵ��ڳ�ͻ��*/
static int compare_time(unsigned int passtime, unsigned int currenttime)
{
    TVMS_LOG("passtime:%d currenttime:%d\n", passtime, currenttime);
    if((currenttime - 1) <= passtime && (passtime <= (currenttime + 1)))
        return 1;
    return 0;
}

/*����һ��ʱ����ȵģ�1,����0 */
static int compare_heartmsg_time()
{

	if(tvms_list == NULL || tvms_list->heartlist == NULL)
		return -1;
	unsigned int 	current_time = 0;
	current_time 	= tvms_mid_time();
	
	TVMS *p = tvms_list->heartlist;

    for(; NULL != p; p = p->next)
    {
        TVMS_LOG("pass_time:%u current_time:%u\n", p->pass_time, current_time);
        if(compare_time(p->pass_time, current_time) == 1)
        {
            TVMS_LOG("pass_time:%u current_time:%u\n", p->pass_time, current_time);
            return 0;
        }


/*ɾ��������Ϣ�ŵ���Ϣ��ȡ��*/
#if 0        
        if(p->pass_time < (current_time -1)){
		unlink_tvms_elem(&(tvms_list->heartlist), p);
		tvms_free(&p);
		p = NULL;
		PRINTF("TVMS :warning!!! the msg pass_time time is warining !!!!warning!!!!!");
	    }
#endif
    }

#if 0
	if(compare_time(p->pass_time,current_time) == 1)
		return 0;
	/*�߼���Ҫ��ϸ����**/
	if(p->pass_time < (current_time -1)){
		unlink_tvms_elem(&(tvms_list->heartlist), p);
		tvms_free(&p);
		p = NULL;
		PRINTF("TVMS :warning!!! the msg pass_time time is warining !!!!warning!!!!!");
		return 0;
	}
#endif
	return -1;
}




/**�Ƚϵ�ǰ���е�ƫ��ʱ�䣬�������2S���ڣ�����Ϊ�ǵ�����*/
static int compare_vodmsg_time()
{

	if(tvms_list == NULL || tvms_list->vodlist == NULL)
		return -1;


	TVMS *p = NULL;
	int flag = -1;
	unsigned int current_time = 0;
	
	p =	tvms_list->vodlist;
	current_time = tvms_get_vod_time();
       TVMS_LOG("current_time:%d\n", current_time);
	if(current_time == 0)
		return -1;
	
	while(p != NULL){
		if(compare_time(p->pass_time,current_time) == 0){
			//p->showflag = 0;
		}
		else
			flag = 0;		
		p = p->next;
		continue;
	}
	return flag;
}




/****************************************************
*    ���Ϳ�������INI�ļ�����Ϣ
*****************************************************/

/*���Ϳ�ʼ�������tvms_request_loop
*  0 heart  1 vod   2 end
*/
static int control_tvms_request(int msg)
{
	if (tvms_yx_msgq_put(g_tvms_request_queue, (char *)&msg) == -1){
		return -1;
	}
	return 0;
}



/***********************************************************
*            ��Ϣ���е�ͨ�ú���
************************************************************/

/*����ȫ�ֵ���Ϣ����*/
static TVMSLIST * tvmslist_creat(void)
{
    TVMSLIST *p = NULL;
    p = (TVMSLIST *) malloc(sizeof(TVMSLIST));
    if(NULL == p) {
        TVMS_LOG("TVMS :creat tvmslist failed!\n");
        return NULL;
    }
    memset(p, 0, sizeof(TVMSLIST));
    p->showlist 	= NULL;
    p->vodlist 	= NULL;
    p->heartlist 	= NULL;
    return p;
}

/*����һ��TVMS��Ϣ*/
static TVMS *new_tvms_elem()
{
    TVMS *p = NULL;
    p = (TVMS *)malloc(sizeof(TVMS));
    if(NULL == p) {
        TVMS_LOG("TVMS :create the tvms elem failed!\n");
        return (NULL);
    }
    memset(p, 0, sizeof(TVMS));

    p->next = NULL;
    p->prev = NULL;
    p->showstatus = -1;
    p->showflag = 0;
    p->msgTaskCode[0] = '\0';
    p->taskType = -1;
    p->priority = -1;
    p->confirmFlag = 1;
    p->origsendTime[0] = '\0';
    p->sendTime[0] = '\0';
    p->resendTime = 600;
    p->validTime[0] = '\0';
    p->mediaCode[0] = '\0';
    p->mode = -1;
    //p->title[0]='\0';
    p->showDirect = 0;
    p->displayURL = NULL;
    //p->reserve = NULL;
    return p;
}


static void tvms_free(TVMS **p)
{
    if(*p == NULL)
        return;
    //printf_tvms_msg("tvms_free", *p);
    if((*p)->displayURL != NULL)
        free((*p)->displayURL);
#if 0
    if((*p)->reserve != NULL)
        free((*p)->reserve);
#endif
    free(*p);
    *p = NULL;
}

/*����*/
static void unlink_tvms_elem(TVMS **head, TVMS *p)
{

    if(*head != NULL && p != NULL) {
        if(p->prev != NULL)
            p->prev->next = p->next;
        else
            *head = p->next;

        if(p->next != NULL)
            p->next->prev = p->prev;
        p->next = NULL;
        p->prev = NULL;
    }
}

/*�ж����MSGTASKCODE �ǲ����Ѿ����ڣ�������ڣ��������ָ��*/
static TVMS *search_queue_by_msgcode(TVMS *head, char *msgcode)
{
    TVMS *temp = head;
    if(head == NULL || msgcode == NULL)
        return NULL;
    while(temp != NULL) {
        if(temp->msgTaskCode != NULL) {
            if(strcmp(msgcode, temp->msgTaskCode) == 0)
                return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

/*��ʱ�����򣬳���2������ͷ���ı䣬����������ʱ��
 ����ʱ���������ʱ��һ�����������*/
static int add_tvms_elem(TVMS *p)
{
    if(NULL == p || tvms_list == NULL) {
        TVMS_LOG("TVMS :error! add tvms elem is error\n");
        return -1;
    }
    TVMS *templist = NULL;
    int task_type = p->taskType;
    if((task_type < 0) || (task_type > 3)) {
        TVMS_LOG("TVMS :ERROR!UNKOWN task_type = %d to add !\n", task_type);
        return -1;
    }
    if(2 == task_type) {
        templist = tvms_list->vodlist;
    } else {
        templist = tvms_list->heartlist;
    }
    TVMS *temp_msg = NULL;
    if(search_queue_by_msgcode(tvms_list->showlist, p->msgTaskCode) != NULL)
        return -1;
    /*�����ж����MSG�ǲ����ظ���������ڣ�ɾ�������MSG��֮���ڰ���ʱ�����²���*/
    temp_msg = search_queue_by_msgcode(templist, p->msgTaskCode);
    if(temp_msg != NULL) {
        return -1;
    }

    unsigned int pass_time = p->pass_time;

    /*�����һ��*/
    if((templist == NULL) || (templist->pass_time >= pass_time)) {
        p->next = templist;
        templist = p;
        if(p->next != NULL) {
            p->next->prev = p;
        }
    } else {
        TVMS *temp = NULL;
        TVMS *temp1 = NULL;
        temp = templist;
        temp1 = temp;
        while((temp != NULL) && (pass_time > (temp->pass_time))) {
            temp1 = temp;
            temp = temp->next;
        }
        temp1->next = p;
        p->prev = temp1;
        p->next = temp;
        if(temp != NULL)
            temp->prev = p;
    }

    if(2 == task_type) {
        tvms_list->vodlist = templist;
    } else {
        tvms_list->heartlist = templist;
    }

    return 0;

}

/*չʾ������Ϣ�ͷ������showlist��֪��validtime���ڣ���ɾ����
��������ͬһ����Ϣ���չʾ*/
static void add_elem_showlist(TVMS *p)
{
    if(NULL == p || tvms_list == NULL) {
        TVMS_LOG("TVMS :error! add tvms showlist is error\n");
        return ;
    }
    TVMS *templist = tvms_list->showlist;
    //printf_tvms_msg("add_elem_showlist:",p);
    char validtime[15] = {0};
    strcpy(validtime, p->validTime);
    /*�����һ��*/
    if((templist == NULL) || strcmp(templist->validTime, validtime) >= 0) {
        p->next = templist;
        templist = p;
        if(p->next != NULL) {
            p->next->prev = p;
        }
    } else {
        TVMS *temp = NULL;
        TVMS *temp1 = NULL;
        temp = templist;
        temp1 = temp;
        while((temp != NULL) && (strcmp(templist->validTime, validtime) < 0)) {
            temp1 = temp;
            temp = temp->next;
        }
        temp1->next = p;
        p->prev = temp1;
        p->next = temp;
        if(temp != NULL)
            temp->prev = p;
    }
    tvms_list->showlist = templist;

}

/*ɾ��չʾ������Ϣ*/
static void delete_showlist_invalelem()
{
    if(tvms_list == NULL || tvms_list->showlist == NULL)
        return ;
    char value[15] = {0};
    unsigned int current_time = tvms_mid_time();
    tvms_time2string(current_time, value, 0);
    TVMS *temp = tvms_list->showlist;
    while(temp) {
        if(strcmp(temp->validTime, value) <= 0) {
            TVMS_LOG("temp->validTime =%s,value=%s\n", temp->validTime, value);
            printf_tvms_msg("the msg is time out,delete", temp);
            unlink_tvms_elem(&(tvms_list->showlist), temp);
            tvms_free(&temp);
            temp = tvms_list->showlist;
            continue;
        }
        break;
    }
    return;
}


/*���ݷ���ֵ����MSG*/
static void chang_showlist_list(char *code, char * status)
{
    if(tvms_list == NULL)
        return;
    TVMS *p = NULL;
    int delay_time = tvms_delay_time_read();
    int ret = 0;
    TVMS_LOG("--\n");
    p = search_queue_by_msgcode(tvms_list->vodlist,  code);
    if(p != NULL) {
        TVMS_DEBUG();
        if(strcmp(status, "DELAY") == 0) { //delay
            TVMS_DEBUG();
            unlink_tvms_elem(&(tvms_list->vodlist), p);
            ret = modify_tvms_delaytime(p, delay_time);
            p->showflag = 0;
            if(ret == -1) {
                tvms_free(&p);
                p = NULL;
            } else {
                ret = add_tvms_elem(p);
                if(ret == -1) {
                    tvms_free(&p);
                    p = NULL;
                }
            }
        } else if(strcmp(status, "NOSHOW") == 0 || strcmp(status, "NOFILE") == 0) {
            unlink_tvms_elem(&(tvms_list->vodlist), p);
            tvms_free(&p);
            p = NULL;
        }else if(0 == strcmp(status, "SUCCESS")){
            unlink_tvms_elem(&(tvms_list->vodlist), p);
            tvms_free(&p);
            p = NULL;
        }
        return;
    }
    TVMS_LOG("--\n");
    p = search_queue_by_msgcode(tvms_list->showlist,  code);
    if(p == NULL || p->showstatus == 0)
        return;
    TVMS_LOG("--\n");
    p->showstatus = 0;
    if(strcmp(status, "DELAY") == 0) { //delay
        TVMS_DEBUG();
        unlink_tvms_elem(&(tvms_list->showlist), p);
        ret = modify_tvms_delaytime(p, delay_time);
        if(ret == -1) {
            tvms_free(&p);
            p = NULL;
        } else {
            ret = add_tvms_elem(p);
            if(ret == -1) {
                tvms_free(&p);
                p = NULL;
            }
        }
    } else if(strcmp(status, "NOSHOW") == 0 || strcmp(status, "NOFILE") == 0) {
        unlink_tvms_elem(&(tvms_list->showlist), p);
        tvms_free(&p);
        p = NULL;
        return;
    } else if(strcmp(status, "SUCCESS") == 0) {
#if 1
        if(p->displayURL != NULL) {
            free(p->displayURL);
            p->displayURL = NULL;
        }
#endif
#if 0
        if(p->reserve != NULL) {
            free(p->reserve);
            p->reserve = NULL;
        }
#endif
    }
    return ;
}


/*���VOD����*/
static void clear_tvms_vod_request_msg()
{
    TVMSLIST *templist = tvms_list;
    if((templist == NULL) || (templist->vodlist == NULL))
        return ;
    TVMS *vodtvms = templist->vodlist;
    while(vodtvms != NULL) {
        templist->vodlist = vodtvms->next;
        tvms_free(&vodtvms);
        vodtvms = templist->vodlist;
    }
    tvms_list = templist;
}

/*���Heart����*/
static void clear_tvms_heart_request_msg()
{
    TVMSLIST *templist = tvms_list;
    if((templist == NULL) || (templist->heartlist == NULL))
        return ;
    TVMS *temptvms = templist->heartlist;
    while(temptvms != NULL) {
        templist->heartlist = temptvms->next;
        tvms_free(&temptvms);
        temptvms = templist->heartlist;
    }
    tvms_list = templist;
}


static void clear_tvms_showlist()
{
    TVMSLIST *templist = tvms_list;
    if((templist == NULL) || (templist->showlist == NULL))
        return;
    TVMS *temptvms = templist->showlist;
    while(temptvms != NULL) {
        templist->showlist = temptvms->next;
        tvms_free(&temptvms);
        temptvms = templist->showlist;
    }
    tvms_list = templist;
}



/***********************************************************
*             ����INI�ļ���������Ϣ����
************************************************************/
static int modify_tvms_delaytime(TVMS *h, int delay_time)
{
    if(h == NULL)
        return -1;
    TVMS_LOG ("TVMS :#modify_tvms_delaytime\n");
    char value[16];
    unsigned int  result;
    unsigned int cur_time, val_time, total_time;
    unsigned int current_time = tvms_mid_time();
    tvms_time2string(current_time, value, 0);

    if(3 == h->taskType) { /*������Ϣ,���Կ�����ʱ2��չʾ,��������ν��delay����*/
        unsigned int temp_time = 0;
        unsigned int validtime = tvms_string2time(h->validTime);
        temp_time = tvms_string2time(h->sendTime);
        temp_time += delay_time;
        if(temp_time > validtime)
            return -1;
        value[0] = '\0';
        tvms_time2string(temp_time, value, 0);
        strcpy(h->sendTime, value);
        h->pass_time = temp_time;
    } else if((0 == h->taskType) || (1 == h->taskType)) {	//֪ͨ��Ϣ��ֱ����Ϣ
        cur_time = tvms_string2time(h->sendTime) + delay_time;
        val_time = tvms_string2time(h->validTime);
        /*��ʱ���������Ч�ڣ�������*/
        if(val_time < cur_time)
            return -1;
        tvms_time2string(cur_time, value, 0);
        strcpy(h->sendTime, value);
        h->pass_time = cur_time;
    } else if(2 == h->taskType) { //VOD��Ϣ
        //����һ���޷����η�ֹԽ�磬VOD�·��ص��Ǻ���

        total_time = atoi(h->validTime);

        result = atoi(h->sendTime);
        result += delay_time;
        if(result > total_time) {
            //TVMS_LOG("TVMS :the vod tvms show time  is over the program totaltime!\n");
            return -1;
        }
        /*��VOD ��Ҳʹ�ã������������򣬾���Ķ�ʱ����������ڲ��������*/
        h->pass_time = result;
        sprintf(h->sendTime, "%d", result);
    } else {
        TVMS_LOG("TVMS :warnning!The Task_type is error!\n");
        return -1;
    }
    printf_tvms_msg("TVMS :delay_msg_time_success:", h);
    return 0;
}


/*��ʱ��ʱ���heart����
*����һ��TVMS,�������չʾʱ�䣬����delay_time
*�����һ�δ��룬Ĭ��delay_time  = 0;
*����showtime֮�󣬸���pass_time
*ʧ�ܷ��� -1 ����ʾ����ʱ�������Ч��
*������� -1,ɾ��������ڵ�*/
static int modify_tvms_showtime(TVMS *h)
{
    if(h == NULL)
        return -1;
    strcpy(h->origsendTime, h->sendTime);
    //TVMS_LOG ("##########modify_tvms_showtime############\n");
    char value[16] = {0};
    unsigned int  result;
	unsigned int cur_time,val_time,total_time;
    unsigned int current_time = tvms_mid_time();
    unsigned int temp_time = 0;
    tvms_time2string(current_time, value, 0);

    if(h->resendTime == 0)
        h->resendTime = 600;

    if(3 == h->taskType) /*������Ϣ,���Կ�����ʱ2��չʾ,��������ν��delay����*/
        strcpy(h->origsendTime, value);
    else
        strcpy(h->origsendTime, h->sendTime);


    if(3 == h->taskType) { /*������Ϣ,���Կ�����ʱ2��չʾ,��������ν��delay����*/
        strcpy(h->sendTime, value);
        h->pass_time = current_time;
        temp_time = h->resendTime + current_time;
        value[0] = '\0';
        tvms_time2string(temp_time, value, 0);
        strcpy(h->validTime, value);

    } else if((0 == h->taskType) || (1 == h->taskType)) {	//֪ͨ��Ϣ��ֱ����Ϣ
        TVMS_LOG("h->sendtiem =%s==tasktype=%d\n", h->sendTime, h->taskType);
        if(strlen(h->sendTime) == 0)
            return -1;
        TVMS_LOG("h->sendtiem =%s==tasktype=%d\n", h->sendTime, h->taskType);

        temp_time = tvms_string2time(h->sendTime);
        if(current_time > temp_time)
            cur_time = current_time ;
        else
            cur_time = temp_time;
        //���ݰ��ȳ�Ҫ��Ĭ��ʱ��Ϊ2����֮�����
#if 0
        if(strlen(h->validTime) == 0) {
            unsigned int temp_time1 = h->resendTime + temp_time;
            tvms_time2string(temp_time1, value, 0);
            strcpy(h->validTime, value);
        }

        val_time = tvms_string2time(h->validTime);
#else
        if(strlen(h->validTime) == 0) {
            unsigned int temp_time1 = h->resendTime + temp_time;
            tvms_time2string(temp_time1, value, 0);
            strcpy(h->validTime, value);
        }
        //char newtime[15] = {0};
        unsigned int val_time2 = tvms_string2time(h->validTime);
		int addtime = val_time2 - cur_time;
        if(addtime >= 0)
        {
		    if(addtime > 120)
			    addtime = 120;
		    if(cur_time != temp_time)
			    addtime = 30 ; //�ٶ�sendtime С�ڵ�ǰʱ�䣬ֻ�ڵ�ǰʱ����60S���

        }
        val_time = cur_time + addtime;
#endif
        /*��ʱ���������Ч�ڣ�������*/
        if(val_time < current_time)
            return -1;
		PRINTF("h->sendtiem =%s==tasktype=%d\n",h->sendTime,h->taskType);
		int testrand = RAND_PASSTIME;
		result=(testrand % (val_time - cur_time)) + cur_time ;
		if(temp_time >= current_time)
			result = temp_time;
        tvms_time2string(result, value, 0);
        strcpy(h->sendTime, value);
        h->pass_time = result;
    } else if(2 == h->taskType) { //VOD��Ϣ
        if(strlen(h->sendTime) == 0)
            return -1;
        Hippo::SystemManager &sysManager = Hippo::systemManager();
        Hippo::UltraPlayer *player = sysManager.obtainMainPlayer();

        if(player){
            total_time = player->getTotalTime();
        }
        sysManager.releaseMainPlayer(player);
        result = atoi(h->sendTime); // ����һ���޷����η�ֹԽ�磬VOD�·��ص��Ǻ���

        unsigned int temp_time1 = result + h->resendTime;
        sprintf(h->validTime, "%d", temp_time1);
        //TVMS_LOG("TVMS :VOD VALID TIME = %s\n",h->validTime);
        h->pass_time = result;
    } else {
        TVMS_LOG("TVMS :The Task_type is error!\n");
        return -1;
    }
    return 0;
}






/*����INI�ļ���������Ϣ����*/
static int parser_tvms_inifile(int type, char* str, int size, int arg)
{
   
    if(!str || !size) {
        TVMS_LOG("Error , not get any tvms file\n");
        return  -1;
    }
    TVMS_LOG("TVMS :get _tvms_inifile ,size[%d] info{\n%s\n}\n", size, str);
    char*		lineText;
    char*		tagText;
    char*		valueText;
    TVMS*		g_temp = NULL;
    char*		ptr;

    lineText = (char*)malloc(2048);
    tagText = (char*)malloc(1024);
    valueText = (char*)malloc(1024);
    int ret = 0;
    while(size > 0 && str != NULL) {
        ptr = tvms_file_get_line(str, size, lineText);
        TVMS_LOG("tvms file get line [%s]\n", lineText);
        if(NULL == ptr)
            break;
        size -= (ptr - str);
        str = ptr;
        if('[' == *lineText) {
            if(tvms_line_get_bracket(lineText, strlen(lineText), tagText) < 0)
                continue;
            /*���INI�ļ��а��������Ϣ�ṹ��*/
            if(!strcmp(tagText, "msglist")) {
                continue;
            } else if(!strcmp(tagText, "end")) {
                if(g_temp != NULL) {
                    ret = modify_tvms_showtime(g_temp);
                    if(ret == -1) {
                        TVMS_LOG(("error show time\n"));
                        tvms_free(&g_temp);
                        g_temp = NULL;
                    } else {
                        TVMS_DEBUG();
                        tvms_mid_mutex_lock(tvms_msg_queue_mutex);
                        int rt = add_tvms_elem(g_temp);
                        if(rt != -1)
                            printf_tvms_msg("add_tvms_elem", g_temp);
                        tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
                        if(rt == -1) {
                            if(g_temp != NULL)
                                tvms_free(&g_temp);
                            g_temp = NULL;
                        }

                    }
                    g_temp = NULL;
                }
                break;
            } else {
                if(g_temp != NULL) {
                    tvms_free(&g_temp);
                }
                g_temp = NULL;
                break;
            }
        } else {
            if(tvms_line_get_tag(lineText, strlen(lineText), tagText, valueText) < 0)
                continue;
            if(!strcmp(tagText, "msgTaskCode")) {
                if(g_temp != NULL) {
                    ret = modify_tvms_showtime(g_temp);
                    if(ret == -1) {
                        TVMS_LOG("TVMS :modify_tvms_showtime::error show time\n");
                        tvms_free(&g_temp);
                        g_temp = NULL;
                    } else {
                        TVMS_DEBUG();
                        tvms_mid_mutex_lock(tvms_msg_queue_mutex);
                        int rt = add_tvms_elem(g_temp);
                        if(rt != -1)
                            printf_tvms_msg("add_tvms_elem", g_temp);
                        tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
                        if(rt == -1) {
                            if(g_temp != NULL)
                                tvms_free(&g_temp);
                        }

                    }
                    g_temp = NULL;
                }
                g_temp = new_tvms_elem();
                if(NULL == g_temp)
                    break;
                if(strlen(valueText) > 64) {
                    strncpy(g_temp->msgTaskCode, valueText, 63);
                    g_temp->msgTaskCode[64] = '\0';
                } else
                    strcpy(g_temp->msgTaskCode, valueText);
                continue;
            } else if(!strcmp(tagText, "taskType")) {
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) != 0)
                    g_temp->taskType = atoi(valueText);
                continue;
            } else if(!strcmp(tagText, "priority")) {
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) != 0)
                    g_temp->priority = atoi(valueText);
                continue;
            } else if(!strcmp(tagText, "confirmFlag")) {
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) != 0)
                    g_temp->confirmFlag = atoi(valueText);
                continue;
            } else if(!strcmp(tagText, "sendTime")) {
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) > 14) {
                    strncpy(g_temp->sendTime, valueText, 14);
                    g_temp->sendTime[14] = '\0';
                } else
                    strcpy(g_temp->sendTime, valueText);
                continue;
            } else if(!strcmp(tagText, "reSendTime")) {
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) != 0)
                    g_temp->resendTime = atoi(valueText);
                continue;
            } else if(!strcmp(tagText, "validTime")) {
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) > 14) {
                    strncpy(g_temp->validTime, valueText, 14);
                    g_temp->validTime[14] = '\0';
                } else
                    strcpy(g_temp->validTime, valueText);
                continue;
            } else if(!strcmp(tagText, "programCode")) {
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) > 128) {
                    strncpy(g_temp->mediaCode, valueText, 128);
                    g_temp->mediaCode[128] = '\0';
                } else
                    strcpy(g_temp->mediaCode, valueText);
                continue;
            } else if(!strcmp(tagText, "mode")) {
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) != 0)
                    g_temp->mode = atoi(valueText);
                continue;
            }

            else if(!strcmp(tagText, "title")) {
#if 1
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) > 50) {
                    strncpy(g_temp->title, valueText, 50);
                    g_temp->title[50] = '\0';
                } else
                    strcpy(g_temp->title, valueText);
#endif
                continue;
            }

            else if(!strcmp(tagText, "displayURL")) {
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) > 1024) {
                    g_temp->displayURL = (char *)malloc(1025 * sizeof(char));
                    strncpy(g_temp->displayURL, valueText, 1024);
                    g_temp->displayURL[1024] = '\0';
                } else {
                    g_temp->displayURL = (char *)malloc((strlen(valueText) + 1) * sizeof(char));
                    strcpy(g_temp->displayURL, valueText);
                }
                continue;
            } else if(!strcmp(tagText, "showDirect")) {
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) != 0)
                    g_temp->showDirect = atoi(valueText);
                continue;
            }

            else if(!strcmp(tagText, "reserve")) {
#if 0
                if(g_temp == NULL)
                    continue;
                if(strlen(valueText) > 1024) {
                    g_temp->reserve = (char *)os_malloc(1025 * sizeof(char));
                    strncpy(g_temp->reserve, valueText, 1024);
                    g_temp->reserve[1024] = '\0';
                } else {
                    g_temp->reserve = (char *)os_malloc((strlen(valueText) + 1) * sizeof(char));
                    strcpy(g_temp->reserve, valueText);
                }
#endif
                continue;
            }

            else {
                if(g_temp != NULL) {
                    tvms_free(&g_temp);
                }
                g_temp = NULL;
                break;
            }
        }
    }

    if(g_temp != NULL)
        tvms_free(&g_temp);
    free(lineText);
    lineText = NULL;
    free(tagText);
    tagText = NULL;
    free(valueText);
    valueText = NULL;
    return 0;
}

/******************************************************************
*								��ʱ��ɨ�赽��ʱ��㣬����Ϣ����
*******************************************************************/

typedef struct scan_list_t {
    TVMS *confirmlist;
    TVMS *noconfirmlist;
} tvms_scan_list;

static tvms_scan_list *scan_list = NULL;

static int  init_tvms_scan_list()
{
    scan_list = (tvms_scan_list *)malloc(sizeof(tvms_scan_list));
    if(scan_list == NULL)
        return -1;
    memset(scan_list, 0, sizeof(tvms_scan_list));
    return 0;
}
static void add_scan_temp_elem(TVMS *p)
{
    if(p == NULL || scan_list == NULL)
        return;
    int confirm = p->confirmFlag;
    TVMS *templist = NULL;
    if(confirm == 1)
        templist = scan_list->confirmlist;
    else
        templist = scan_list->noconfirmlist;

    if(templist == NULL) {
        templist = p;
        if(confirm == 1)
            scan_list->confirmlist = templist;
        else
            scan_list->noconfirmlist = templist;
        return;
    }
    unsigned int priority = p->priority;

    /*�����һ��*/
    if((templist == NULL) || (templist->priority <= priority)) {
        p->next = templist;
        templist = p;
        if(p->next != NULL) {
            p->next->prev = p;
        }
    } else {
        TVMS *temp = NULL;
        TVMS *temp1 = NULL;
        temp = templist;
        temp1 = temp;
        while((temp != NULL) && (priority < (temp->priority))) {
            temp1 = temp;
            temp = temp->next;
        }
        temp1->next = p;
        p->prev = temp1;
        p->next = temp;
        if(temp != NULL)
            temp->prev = p;
    }
    if(confirm == 1)
        scan_list->confirmlist = templist;
    else
        scan_list->noconfirmlist = templist;
    return;
}



/*ֻ���ǵ�һ����ȷ���յ������*/
static void delete_noconfirm_elem()
{
    if(scan_list == NULL)
        return;
    TVMS *temp = scan_list->noconfirmlist;
    while(temp != NULL) {
        scan_list->noconfirmlist = temp->next;
        tvms_free(&temp);
        temp = scan_list->noconfirmlist;
    }
    scan_list->noconfirmlist = NULL;
    return ;

}

/*�õ����ȼ���ȷ���յ�һ���ĸ���*/
static int get_same_time_num(TVMS *p)
{
    if(p == NULL)
        return -1;
    int prio = p->priority;
    int confirm = p->confirmFlag;
    int temp_num = 0;
    TVMS *temp = p;
    while(temp != NULL) {
        if((prio == temp->priority) && (confirm == temp->confirmFlag)) {
            temp_num ++;
            temp = temp->next;
        } else
            break;
    }
    return temp_num;
}

/*�����������ѡ����ʾMSG*/
static TVMS * search_scan_list(int num, TVMS *p)
{
    if(p == NULL)
        return NULL;
    TVMS *temp = p;
    int i = 0;
    while(temp != NULL) {
        if(i == num)
            return temp;
        temp = temp->next;
        i++;
    }
    return NULL;
}


/*ȥ��ʾҳ? ʧ�ܻ����ӳ�*/
static int begin_show_msg(TVMS *p)
{
    if(p == NULL)
        return -1;
    printf_tvms_msg("begin_show_msg:::begin=1054", p);
    if(p->taskType == 2 && 0 != tvms_get_vod_playstate()) {
        TVMS_LOG("TVMS :warning ,the vod status is not playing!!!\n");
        //TVMS_DEBUG();
        //tvms_mid_mutex_lock(tvms_msg_queue_mutex);
        add_tvms_elem(p);
        //tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
        return 0;
    }
    if(p->taskType == 2 && p->showflag == 1) {
        TVMS_LOG("TVMS :warning ,the same vod msg send to browser!!!\n");
        //TVMS_DEBUG();
        //tvms_mid_mutex_lock(tvms_msg_queue_mutex);
        add_tvms_elem(p);
        //tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
        return 0;
    }

    if(p->taskType == 2) {
        char mcode[128 + 1] = {0};
        tvms_mediacode_read(mcode);
        if(strlen(mcode) == 0 || strcmp(mcode, p->mediaCode) != 0) {
            TVMS_LOG("TVMS :warning,the mediacode is error,the msg mediacode =%s,the current mediacode = %s\n", p->mediaCode, mcode);
            //	TVMS_DEBUG();
            //	tvms_mid_mutex_lock(tvms_msg_queue_mutex);
            /*�����־λ�ŵ��ɹ�sucess ֮��*/
            add_tvms_elem(p);
            //	tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
            return 0;
        }
    }
    if(p->taskType == 1) {
        char mcode[128 + 1] = {0};
        tvms_mediacode_read(mcode);
        if(strlen(mcode) == 0 || strcmp(mcode, p->mediaCode) != 0) {
            TVMS_LOG("TVMS :warning,the mediacode is not match,the msg mediacode =%s,the current mediacode = %s\n", p->mediaCode, mcode);
            /*�����־λ�ŵ��ɹ�sucess ֮��*/
            tvms_free(&p);
            p = NULL;
            return 0;
        }
    }
    tvms_send_event(p);

    if(p->taskType != 2) {
        //	TVMS_DEBUG();
        //	tvms_mid_mutex_lock(tvms_msg_queue_mutex);
        add_elem_showlist(p);
        //	tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
    } else {
        //	TVMS_DEBUG();
        //	tvms_mid_mutex_lock(tvms_msg_queue_mutex);
        /*�����־λ�ŵ��ɹ�sucess ֮��*/
        p->showflag = 1;
        add_tvms_elem(p);
        //	tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
    }

    printf_tvms_msg("$$$begin_show_msg:::end$==1114==", p);
    return 0;
}


static void add_temp_to_msglist(TVMS *head)
{
    if(head == NULL)
        return;
    unsigned int delay_time = tvms_delay_time_read();
    int ret = 0;
    TVMS *temp = head;
    TVMS *temp2 = NULL;
    while(temp != NULL) {
        temp2 = temp->next;
        unlink_tvms_elem(&(head), temp);
        TVMS_DEBUG();
        ret = modify_tvms_delaytime(temp, delay_time);
        if(ret == -1) {
            if(temp != NULL)
                tvms_free(&temp);
            temp = NULL;
        } else {
            ret = add_tvms_elem(temp);
            if(ret == -1) {
                if(temp != NULL)
                    tvms_free(&temp);
            }
        }
        temp = temp2;
    }

}

static void add_scan_to_msglist()
{
	if(scan_list == NULL)
		return;

    modify_delay_after_show();

	TVMS *p = scan_list->confirmlist;
	add_temp_to_msglist(p);
	p = scan_list->noconfirmlist;
	add_temp_to_msglist(p);
	scan_list->noconfirmlist = NULL;
	scan_list->confirmlist = NULL;
}

/*��ͻ��������A,B���ֲ�ͬ��ͻ����*/
static void deal_conflict_msg()
{
    TVMS_DEBUG();

    if(scan_list == NULL)
        return;
    TVMS *temp_confirm = scan_list->confirmlist;
    TVMS *temp_noconfirm = scan_list->noconfirmlist;
    TVMS *temp = NULL;

    if(temp_confirm == NULL  && temp_noconfirm == NULL)
        return;
    /* ���е���Ϣ���ǲ�ȷ���յ�*/

    if(temp_confirm == NULL) {
        /*�õ�����������*/
        int num = get_same_time_num(temp_noconfirm);
        if(num == -1)
            return;
        int rand_num = 0;
        if(num != 1)
            rand_num = rand() % num;
        else
            rand_num = 0;

        temp = search_scan_list(rand_num, temp_noconfirm);
        if(temp == NULL) {
            add_scan_to_msglist();
            return;
        }
        /*չʾҳ��*/
        unlink_tvms_elem(&(scan_list->noconfirmlist), temp);
        begin_show_msg(temp);
        temp = NULL;
        delete_noconfirm_elem();
    }
    /*����ȷ���յ�����Ϣ����Ĭ��ֱ����ʾ��һ�����ȼ�*/
    else {
        /*��ɾ����ȷ������*/
        delete_noconfirm_elem();
        int num = get_same_time_num(temp_confirm);
        if(num == -1)
            return;
        int rand_num = 0;
        int testnum = rand();
        if(num != 1)
            rand_num = testnum % num;
        else
            rand_num = 0;
        temp = search_scan_list(rand_num, temp_confirm);
        if(temp == NULL) {
            add_scan_to_msglist();
            return;
        }
        unlink_tvms_elem(&(scan_list->confirmlist), temp);
        begin_show_msg(temp);
        temp = NULL;
        add_scan_to_msglist();
    }

}

/***********************************************************
*              ����INI�ļ���
************************************************************/
static int g_beattimes				= 0;
static char tvms_heart_url[512] 	= {0};
static char tvms_vod_url[512] 		= {0};

static int get_beattimes()
{
    if(g_beattimes == 65000)
        g_beattimes = 1;
    return g_beattimes++;
}

static char * get_heart_url()
{
#if 0
    static char test[1024] = {0};
    static int testflag = 1;
    sprintf(test, "http://10.10.11.80/tvms/tvms%d.ini", testflag++);
    return test;
#endif
    char url[512] = {0};
    char areaid[128 + 1] = {0};
    int ret = -1;
    ret  = get_areaID(areaid);
    if(ret == -1)
        return NULL;
    tvms_heart_url_read(url, 512);
    if(strlen(url) == 0)
        return NULL;
    tvms_heart_url[0] = '\0';
    char userid[32] = {0};
    get_user_info(userid);

    snprintf(tvms_heart_url, 512, "%s?method=heartbeat&user=%s&areaid=%s&beattimes=%d&UserGroup=%s",
             url, userid, areaid, get_beattimes(), tvms_app_UserGroupNMB_get());
    TVMS_LOG("TVMS :tvms_heart_url:%s \n", tvms_heart_url);
    return tvms_heart_url;
}

static char* get_vod_url()
{
    char url[512] = {0};
    int ret = -1;
    TVMS_DEBUG();
    tvms_heartvod_url_read(url, 512);
    TVMS_DEBUG();
    if(strlen(url) == 0)
        return NULL;
    char userid[32] = {0};
    get_user_info(userid);
    char areaid[128 + 1] = {0};
    TVMS_DEBUG();
    ret = get_areaID(areaid);
    TVMS_DEBUG();
    if(ret == -1)
        return NULL;
    TVMS_DEBUG();
    tvms_vod_url[0] = '\0';
    char mcode[128 + 1] = {0};
    tvms_mediacode_read(mcode);
    if(strlen(mcode) == 0)
        return NULL;
    TVMS_DEBUG();
    snprintf(tvms_vod_url, 512, "%s?method=VOD&user=%s&areaid=%s&mediacode=%s&UserGroup=%s",
             url, userid, areaid, mcode, tvms_app_UserGroupNMB_get());
    TVMS_LOG("TVMS :tvms_vod_url -= %s \n", tvms_vod_url);
    return tvms_vod_url;
}


/*������������Ҫ��INI�ļ�*/
static int tvms_heart_request_msg(void)
{
    char *url = NULL;
    url = get_heart_url();
    if(url == NULL)
        return -1;

    TVMS_LOG("hsyslog: request tvms heart message,url =%s\n", url);

    if(tvms_get_inifile(url, (void *)parser_tvms_inifile) < 0)
        return -1;
    return 0;
}

static int tvms_vod_request_msg(void)
{
    char *url = NULL;
    TVMS_DEBUG();
    url = get_vod_url();
    TVMS_DEBUG();
    if(url == NULL)
        return -1;

    TVMS_LOG("hsyslog: request tvms vod message,url =%s\n", url);

    if(tvms_get_inifile(url, (void *)parser_tvms_inifile) < 0)
        return -1;
    return 0;
}





/********************************************************
*                        �ⲿ�ӿ�
*
*********************************************************/
static int tvms_request_is_init = 2;
extern "C" int
init_tvms_msg()
{
    //����stanby֮�󣬲��·�tvms��ַ����
    if(stanby_tvms == 2)
        stanby_tvms = 0;

    int 	ret = 0;
    int 	tvms_request_task = 0;
    int 	tvms_time_scan_task = 0;
    char url[512] = {0};
    int 	len = 512;

    LogRunOperDebug("hsyslog: now init tvms\n");

    TVMS_LOG("--now-init-tvms---\n");
    tvms_heart_url_read(url, len);
    if(strlen(url) < 7) {
        stanby_tvms = 1;
        return -1;
    }
    TVMS_LOG("init_tvms_msg heart url=%s\n", url);

    memset(url, 0, 512);
    tvms_heartvod_url_read(url, len);
    if(strlen(url) < 7) {
        stanby_tvms = 1;
        return -1;
    }
    TVMS_LOG("init_tvms_msg vod url=%s\n", url);

    if(tvms_request_is_init != 2)
        return 0;
    tvms_request_is_init = 0;

    if(0 == tvms_request_is_init) {
        TVMS_DEBUG();
        tvms_list = tvmslist_creat();
        if(tvms_list == NULL) {
            TVMS_LOG("TVMS :ERROR,tvmslist_create is error\n");
            return -1;
        }
        TVMS_DEBUG();
        ret = init_tvms_scan_list();
        if(ret != 0) {
            if(!tvms_list)
                free(tvms_list);
            return -1;
        }
        TVMS_DEBUG();
        g_tvms_request_queue = tvms_yx_msgq_create(10, sizeof(int));
        if(!g_tvms_request_queue) {
            if(!tvms_list)
                free(tvms_list);
            if(!scan_list)
                free(scan_list);
            TVMS_LOG("create tvms queue for control failed\n");
            return -1;
        }

        TVMS_DEBUG();
        g_tvms_request_fd 	= tvms_yx_msgq_readfd_get(g_tvms_request_queue);

        /*�����ź���*/
        tvms_msg_queue_mutex 		= 	tvms_mid_mutex_create();
        /*���������ջ��С����*/
        tvms_request_task  = tvms_request_task_create();
        tvms_time_scan_task = tvms_time_scan_task_create();
        TVMS_DEBUG();
        if(tvms_request_task == -1 || tvms_time_scan_task == -1) {
            TVMS_LOG("ERROR,Create_tvms_task is error\n");
            if(!tvms_list)
                free(tvms_list);
            if(!scan_list)
                free(scan_list);
            tvms_yx_msgq_delete(g_tvms_request_queue);
            return -1;
        }
        TVMS_DEBUG();
        TVMS_LOG("TVMS :********init_tvms_msg***20-05-2009******************\n");
        tvms_request_is_init = 1;
    }
    return 0;
}


/*************************************************************
*��ֲ��Ҫ���õĺ���
***************************************************************/
extern "C" void
livetv_tvmsmsg_request()
{
    TVMS_LOG("TVMS :join LiveTV\n");
    char areaid[128 + 1] = {0};

    if(-1 == get_areaID(areaid))
        return;
    char mediacode[128 + 1] = {0};
    if(-1 == tvms_mediacode_read(mediacode))
        return;
    control_tvms_request(2);
}

extern "C" void
livetv_tvmsmsg_clear_request()
{
    TVMS_LOG("TVMS :quit LiveTv\n");
    char areaid[128 + 1] = {0};
    if(-1 == get_areaID(areaid))
        return;
    control_tvms_request(4);

}
/*����һ���µ�VODʱ�����*/
extern "C" void
vod_tvmsmsg_request()
{
    char areaid[128 + 1] = {0};
    if(get_areaID(areaid) == -1)
        return;
    char mediacode[128 + 1] = {0};
    if(-1 == tvms_mediacode_read(mediacode))
        return;

    control_tvms_request(1);
}

/*�˳�VODʱ�����*/
extern "C" void
vod_tvmsmsg_clear_request()
{
    char areaid[128 + 1] = {0};
    if(get_areaID(areaid) == -1)
        return;

    control_tvms_request(3);
}

extern "C" void
tvms_msg_status_deal(char * code, char * status)
{
    if(code == NULL || status == NULL)
        return;
    TVMS_LOG("TVMS :return msg code =%s,status = %s\n", code, status);
    if((strlen(code) > 64) || (strlen(status) > 9))
        return;
    TVMS_DEBUG();
    tvms_mid_mutex_lock(tvms_msg_queue_mutex);
    TVMS_LOG("TVMS :return msg code =%s,status = %s\n", code, status);
    chang_showlist_list(code, status);
    TVMS_LOG("TVMS :return msg code =%s,status = %s\n", code, status);
    tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
    TVMS_LOG("TVMS :return msg code =%s,status = %s\n", code, status);
}
/*************************************************************
*��ֲ��Ҫʵ�ֵĺ���
***************************************************************/


/*����stb�˺�*/
static int get_user_info(char *userid)
{
    return tvms_userid_read(userid);
}


/*stanby*/
extern "C" void
clear_tvms_msg()
{
    if(tvms_request_is_init != 1)
        return;
    TVMS_DEBUG();
    tvms_mid_mutex_lock(tvms_msg_queue_mutex);

    stanby_tvms = 2;
    add_scan_to_msglist();
    clear_tvms_vod_request_msg();
    clear_tvms_heart_request_msg();
    clear_tvms_showlist();
    tvms_list->heartlist = NULL;
    tvms_list->vodlist = NULL;
    tvms_list->showlist = NULL;
    g_beattimes = 0;
    tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
}

#define NOSTATUSTIME 8
#define NOSTATUSDELAYTIME 20
#if 1
//������������������⣬���µ���Ϣ�޷���ȡ����ֵ�����
static void showlist_scan_deal(void)
{
    if(tvms_list == NULL || tvms_list->showlist == NULL)
        return;
    TVMS *temp = tvms_list->showlist;
    TVMS *temp1 = NULL;
    unsigned int current_time = 0;
    current_time = tvms_mid_time();
    int delay_time = NOSTATUSDELAYTIME;  //��˵Ĭ��10S
    int ret = 0;
    while(temp != NULL) {
        temp1 = temp->next;

        if((temp->showstatus == -1) && (((int)(current_time - temp->pass_time)) >= NOSTATUSTIME)) {
            //TVMS_LOG("TVMS :static void showlist_scan_deal(void) = time = %d,cur =%d---%d\n",temp->pass_time,current_time,((int)(temp->pass_time - current_time)));
            if(temp->confirmFlag == 1) { //delay
                unlink_tvms_elem(&(tvms_list->showlist), temp);
                TVMS_DEBUG();
                ret = modify_tvms_delaytime(temp, delay_time);
                if(ret == -1) {
                    tvms_free(&temp);
                    temp = NULL;
                } else {
                    ret = add_tvms_elem(temp);
                    if(ret == -1) {
                        tvms_free(&temp);
                        temp = NULL;
                    }
                }
            } else { //noshow/nofile
                unlink_tvms_elem(&(tvms_list->showlist), temp);
                tvms_free(&temp);
                temp = NULL;
            }
        }
        temp = temp1;
    }
    return ;
}
#endif

/*�������߳�һֱ���ڣ����˳�*/
extern "C" void
tvms_request_loop()
{
    fd_set rset;
    struct timeval tv = {0, 0};
    int msg = -1;
    int ret = 0;
    int hearttime = 0;
    hearttime = tvms_heart_time_read();
    tvms_loop_init();
    int flag = 0;
    char areaid[128 + 1] = {0};
    tvms_mid_task_delay(8000);
    while(1) {
        /*area_id ����EPG����·������������ж��Ƿ��½�ɹ��ı�־*/
        if(get_areaID(areaid) == -1) {
            //TVMS_LOG("TVMS :**********stanby1****************\n");
            tvms_mid_task_delay(3000);
            flag = 0;
            continue;
        }
        tv.tv_sec = hearttime;
        if(flag == 0) {
            tv.tv_sec = 3;
            flag = 1;
        }
        tv.tv_usec = 0;
        FD_ZERO(&rset);
        FD_SET(g_tvms_request_fd, &rset);
        ret = -1;
        msg = -1;
        ret = select(g_tvms_request_fd + 1, &rset, NULL, NULL, &tv);
        if(ret > 0 && FD_ISSET(g_tvms_request_fd, &rset)) {
            if(tvms_yx_msgq_get(g_tvms_request_queue, (char *)&msg) < 0)
                continue;
        }

        /*����VOD ini*/
        if(ret > 0) {
            if(msg == 1) {
                TVMS_LOG("TVMS :tvms_request_loop::begin to get vod ini \n");
                /*��յ�ǰVOD-TVMS��Ϣ*/
                TVMS_DEBUG();
                tvms_mid_mutex_lock(tvms_msg_queue_mutex);
                TVMS_DEBUG();
                clear_tvms_vod_request_msg();
                TVMS_DEBUG();
                tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
                TVMS_DEBUG();
                /*��ȡVOD��Ϣ����*/
                /*ʹ��һ��ȫ�ֱ��������뻥��*/
                tvms_mid_task_delay(2500); //����mediacode�·�������֮��
                TVMS_DEBUG();
                tvms_vod_request_msg();
                TVMS_DEBUG();
                continue;
            } else if(msg == 2) {
                TVMS_LOG("TVMS print:tvms_request_loop:request livetv msg\n");
                TVMS_DEBUG();
                tvms_mid_mutex_lock(tvms_msg_queue_mutex);
                tvms_heart_request_msg();
                tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
                continue;
            }

            else if(msg == 3) {
                TVMS_LOG("TVMS :tvms_request_loop:delete vod msg \n");
                TVMS_DEBUG();
                tvms_mid_mutex_lock(tvms_msg_queue_mutex);
                clear_tvms_vod_request_msg();
                tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
                continue;
            } else if(msg == 4) {
                TVMS_LOG("TVMS print:tvms_request_loop:delete livetv msg\n");
                TVMS_DEBUG();
                tvms_mid_mutex_lock(tvms_msg_queue_mutex);
                clear_tvms_heart_request_msg();
                tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
                continue;
            }

            else {
                TVMS_LOG("TVMS :tvms_request_loop:error_msg\n");
                continue;
            }
        } else {
            /*����heart ini*/
            TVMS_LOG("TVMS :tvms_request_loop::begin to get heart ini \n");
            tvms_heart_request_msg();
            continue;
        }
    }
}

#define TVMS_SCAN_TIME 800
#define DELETE_NUM 150*5 // 5MINTINE
/*����̲߳�����ɨ��ʱ���ö�ʱ������*/
extern "C" void
tvms_time_scan_loop()
{
    int ret = 0;
    unsigned int vod_time = 0;
    TVMS *vod_tvms = NULL;
    TVMS *heart_tvms = NULL;
    TVMS *temp_tvms = NULL;
    TVMS *temp_tvms2 = NULL;
    char value[16] = {0};
    unsigned int current_time = 0;
    char areaid[128 + 1] = {0};
    int delete_flag = 0;
    tvms_loop_init();
    tvms_mid_task_delay(8000);
    while(1) {
        /*area_id ����EPG����·������������ж��Ƿ��½�ɹ��ı�־*/
        if(get_areaID(areaid) == -1) {
            //TVMS_LOG("TVMS :**********stanby2****************\n");
            tvms_mid_task_delay(3000);
            continue;
        }
        tvms_mid_task_delay(TVMS_SCAN_TIME);
        //TVMS_DEBUG();
        tvms_mid_mutex_lock(tvms_msg_queue_mutex);
        showlist_scan_deal();
        ret = compare_heartmsg_time();
        if(ret == 0) {
            tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
            goto DealMsg;
        }
        ret = compare_vodmsg_time();
        if(ret == 0) {
            tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
            goto DealMsg;
        }
        delete_flag ++;
        if(delete_flag == DELETE_NUM) {
            //TVMS_LOG("TVMS :delete_flag == DELETE_NUM\n");
            delete_showlist_invalelem();
            delete_flag = 0;
        }
        tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
        continue;
DealMsg:
        //TVMS_LOG("********tvms_do_loop*********************\n");
        /*�˳��ͱ�ʾ����Ϣ������ʱ��OK��*/
        TVMS_DEBUG();
        tvms_mid_mutex_lock(tvms_msg_queue_mutex);

        TVMSLIST *templist = tvms_list;
        if(templist == NULL) {
            tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
            continue;
        }
        vod_tvms = tvms_list->vodlist;
        heart_tvms = tvms_list->heartlist;
        temp_tvms = NULL;
        value[0] = '\0';
        current_time = tvms_mid_time();
        tvms_time2string(current_time, value, 0);

        /*ȡ����ʱ����Ҫ������VOD��Ϣ*/
        vod_time = tvms_get_vod_time();
        if(vod_time != 0 && vod_tvms != NULL) {
            temp_tvms = vod_tvms;
            while(temp_tvms != NULL) {
                //DEBUG_TVMS(("temp_tvms->pass_time=%d: vod_pass =%d\n", temp_tvms->pass_time, vod_time));
                temp_tvms2 = temp_tvms->next;
                if(compare_time(temp_tvms->pass_time , vod_time)) {
                    unlink_tvms_elem(&(tvms_list->vodlist), temp_tvms);
                    add_scan_temp_elem(temp_tvms);
                }
                temp_tvms = temp_tvms2;
            }
        }

        /*ȡ����ʱ����Ҫ������IPTV��Ϣ*/
        if(heart_tvms != NULL) {
            temp_tvms = heart_tvms;
            while(temp_tvms != NULL) {
                //DEBUG_TVMS(("temp_tvms->pass_time=%d: current_time =%d\n", temp_tvms->pass_time, current_time));
                if(compare_time(temp_tvms->pass_time , current_time)) {// && strcmp(get_current_mediacode(),temp_tvms->Program_Code) == 0){
                    temp_tvms2 = temp_tvms->next;
                    unlink_tvms_elem(&(tvms_list->heartlist), temp_tvms);
                    add_scan_temp_elem(temp_tvms);
                    /*ȡ������ڵ�*/
                    temp_tvms = temp_tvms2;
                    continue;
                }
                break;
            }
        }
        //���ڶ���¼������ó�ͻ����
        /*��ʣ�µ���Ϣ�����²��뵽LIST*/
        deal_conflict_msg();
        add_scan_to_msglist();
        delete_showlist_invalelem();
        tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
    }
}


/*�õ�����ID��EPG�·������ֵ(TVMS��������)*/
int get_areaID(char *areaid)
{
    if(areaid == NULL)
        return -1;
    int ret = -1;
    if(stanby_tvms == 0) {
        ret = tvms_tvms_areaid_get(areaid);
        if(strlen(areaid) == 0)
            ret = -1;
        return ret;
    } else
        return -1;

}

//����ӿ�������װ����taskcode ����ȡ�����Ϣ
//ֻ��ѯ��ʾ����showlist.
//B300�����ӿ�
#if  1
static int search_elem_by_msgcode(TVMS *temp, char *msgcode, char *msgtitle);
static int search_elem_by_msgcode(TVMS *temp, char *msgcode, char *msgtitle)
{
    if(temp == NULL)
        return -1;
    while(temp != NULL) {
        TVMS_LOG("msgcode =%s=%s=\n", msgcode, temp->msgTaskCode);
        if(strcmp(msgcode, temp->msgTaskCode) == 0) {
            TVMS_LOG("OK find this msg \n");
            snprintf(msgtitle, 51, "%s", temp->title);
            return 0;
        }
        temp = temp->next;
    }
    return -1;
}
extern "C" int
search_showlist_by_msgcode(char *msgcode, char *msgtitle)
{
    int ret = -1;
    tvms_mid_mutex_lock(tvms_msg_queue_mutex);
    if(tvms_list == NULL) {
        tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
        return -1;
    }
    TVMS *temp = tvms_list->showlist;
    ret = search_elem_by_msgcode(temp, msgcode, msgtitle);
    if(ret == 0) {
        tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
        return 0;
    }

    temp = tvms_list->heartlist;
    ret = search_elem_by_msgcode(temp, msgcode, msgtitle);
    if(ret == 0) {
        tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
        return 0;
    }

    temp = tvms_list->vodlist;
    ret = search_elem_by_msgcode(temp, msgcode, msgtitle);
    if(ret == 0) {
        tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
        return 0;
    }

    tvms_mid_mutex_unlock(tvms_msg_queue_mutex);
    TVMS_LOG("---\n");
    return -1;
}
#endif

void modify_delay_after_show(void)
{
    TVMS *pTemp = NULL;
    int delay_time = 0;

    delay_time = tvms_delay_time_read();

    pTemp = tvms_list->heartlist;
    for(; NULL != pTemp; pTemp = pTemp->next)
    {
        pTemp->pass_time += delay_time;
    }

    pTemp = tvms_list->vodlist;
    for(; NULL != pTemp; pTemp = pTemp->next)
    {
        pTemp->pass_time += delay_time;
    }

    return ;
}

/*<ShowMsgNotify><MsgCode>1234</MsgCode><STATUS> SUCCESS|DELAY|NOSHOW|NOFILE <STATUS></ShowMsgNotify>*/
void tvms_show_status(const char *buf)
{
    char temp[256] = {0};
    char msgcode[128 + 1] = {0};
    char satus[10] = {0};
    char *p = NULL;
    int tag_len = 0;

    LogSafeOperDebug("TVMS_SHOW_STATUS =%s\n", buf);
    p = distillValuebyTag(buf, "ShowMsgNotify", &tag_len, 0);
    if(p == NULL)
        return;
    strncpy(temp, p, tag_len);
    LogSafeOperDebug("temp1 = %s \n", temp);
    p = distillValuebyTag(temp, "MsgCode", &tag_len, 0);
    if(p == NULL)
        return;
    memset(msgcode, '\0', 129);
    if(tag_len > 128)
        tag_len = 128;
    strncpy(msgcode, p, tag_len);
    msgcode[tag_len] = '\0';
    LogSafeOperDebug("Browser msgcode =%s \n", msgcode);

    p = distillValuebyTag(temp, "STATUS", &tag_len, 0);
    if(p == NULL)
        return;
    memset(satus, '\0', 10);
    if(tag_len > 9)
        tag_len = 9;
    strncpy(satus, p, tag_len);
    LogSafeOperDebug("Browser satus =%s, msg code = %s\n", satus, msgcode);
    tvms_msg_status_deal(msgcode, satus);
}

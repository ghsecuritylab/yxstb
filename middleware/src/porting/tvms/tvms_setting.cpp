#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mid/mid_mutex.h"
#include "AppSetting.h"
#include "SysSetting.h"
#include "SettingApi.h"
#include "ind_cfg.h"

#include "tvms.h"
#include "tvms_define.h"

#include "tvms_setting.h"
#include "TvmsAssertions.h"

typedef struct tvms_conf_t_{
	char 	tvmsgwip[22];  // 标准是16，考虑到端口号，最大为22
	char 	tvmsheartbiturl[256];
	char 	tvmsvodheartbiturl[256];
	int   tvmsheartbitinterval;
	int   tvmsdelaylength;
	//int   exist_flag;
	int 	dirty;
}tvms_conf_t;

static tvms_conf_t tvms_conf_info;
static mid_mutex_t		g_tvms_mutex = NULL;
static CfgTree_t g_cfgTree = NULL;

extern "C" void
tvms_config_init(void)
{

    if(g_cfgTree)
        return;

    g_cfgTree = ind_cfg_create();
    if(g_cfgTree == NULL) {
        LogSafeOperError("tree_cfg_create\n");
        return;
    }

	ind_cfg_inset_object(g_cfgTree, "tvms");
	ind_cfg_inset_string(g_cfgTree, "tvms.tvmsgwip",			          tvms_conf_info.tvmsgwip,22);
	ind_cfg_inset_string(g_cfgTree, "tvms.tvmsheartbiturl",			    tvms_conf_info.tvmsheartbiturl,256);
	ind_cfg_inset_string(g_cfgTree, "tvms.tvmsvodheartbiturl",			tvms_conf_info.tvmsvodheartbiturl,256);
	ind_cfg_inset_int(g_cfgTree, "tvms.tvmsheartbitinterval",		&tvms_conf_info.tvmsheartbitinterval);
	ind_cfg_inset_int(g_cfgTree, "tvms.tvmsdelaylength",		      &tvms_conf_info.tvmsdelaylength);
	memset(&tvms_conf_info, 0, sizeof(tvms_conf_t));
}
/*************************************************
  Description: 加载用户配置信息
  Input:reset:是否重置标志位
  Return:	    无
 *************************************************/
extern "C" void
tvms_config_load(int reset)
{
	TVMS_LOG("tvms_config_load reset =%d\n",reset);
	if(g_tvms_mutex == NULL)
		g_tvms_mutex = mid_mutex_create( );
	mid_mutex_lock(g_tvms_mutex);

	memset(&tvms_conf_info, 0, sizeof(tvms_conf_t));
	tvms_conf_info.tvmsgwip[0] = '\0';
	tvms_conf_info.tvmsheartbitinterval = MIN_HEART_TIME;
#if defined(STBTYPE_ROUMANIA)
	strcpy(tvms_conf_info.tvmsheartbiturl, "http://10.1.12.70:35220/gateway/query.do");
	strcpy(tvms_conf_info.tvmsvodheartbiturl, "http://10.1.12.70:35220/gateway/query.do");
	tvms_conf_info.tvmsdelaylength	= 10;
	tvms_conf_info.tvmsheartbitinterval = 60;
#elif (defined(STBTYPE_QTEL))
	strcpy(tvms_conf_info.tvmsheartbiturl, "http://tvms.qteliptv.net.qa:35220/gateway/query.do");
	strcpy(tvms_conf_info.tvmsvodheartbiturl, "http://tvms.qteliptv.net.qa:35220/gateway/query.do");
	tvms_conf_info.tvmsdelaylength	= 60;
	tvms_conf_info.tvmsheartbitinterval = 60;
#elif (defined(Huawei_v5))
#if (defined(Cameroon_v5))
    strcpy(tvms_conf_info.tvmsheartbiturl, "http://10.10.252.18:35220/gateway/query.do？");
    strcpy(tvms_conf_info.tvmsvodheartbiturl, "http://10.10.252.18:35220/gateway/query.do？");
    tvms_conf_info.tvmsdelaylength  = 60;
    tvms_conf_info.tvmsheartbitinterval = 300;
#endif
#elif (defined(HAERBIN_CUC_HD))
	strcpy(tvms_conf_info.tvmsheartbiturl, "");
	strcpy(tvms_conf_info.tvmsvodheartbiturl, "");
	tvms_conf_info.tvmsdelaylength	= 0;
	tvms_conf_info.tvmsheartbitinterval = 0;
#elif defined(Jiangsu)
	strcpy(tvms_conf_info.tvmsheartbiturl, "");
	strcpy(tvms_conf_info.tvmsvodheartbiturl, "");
	tvms_conf_info.tvmsdelaylength = 0;
	tvms_conf_info.tvmsheartbitinterval = 0;
#elif (defined(VIETTEL_HD))
	strcpy(tvms_conf_info.tvmsheartbiturl, "");
	strcpy(tvms_conf_info.tvmsvodheartbiturl, "");
	tvms_conf_info.tvmsdelaylength = 30;
	tvms_conf_info.tvmsheartbitinterval = MIN_HEART_TIME;
#else
	tvms_conf_info.tvmsdelaylength = 60;
	tvms_conf_info.tvmsheartbitinterval = 60;
#endif
	//tvms_conf_info.exist_flag = 0;

	mid_mutex_unlock(g_tvms_mutex);

	if (reset == 0) {
		if(0 == settingConfigRead(g_cfgTree, "tvms")) {
			TVMS_LOG("Read [yx_config_tvms.ini]  OK!\n");
			return;
		}
		WARN_PRN("Read [yx_config_tvms.ini] failed.\n");
	} else {
	    tvms_conf_info.dirty = 1;
		tvms_config_save();
	}
}

/*************************************************
  Description:保存用户配置信息
  Input:reset:无
  Return:	    无
 *************************************************/
extern "C" void
tvms_config_save(void)/* cusSysConfigSave */
{
	TVMS_LOG("tvms_config_save =%d\n",tvms_conf_info.dirty);
	if(tvms_conf_info.dirty == 0)
		return;
	tvms_conf_info.dirty = 0;
	settingConfigWrite(g_cfgTree, "tvms");
}

extern "C" int
tvms_conf_tvmsgwip_set( const char *buff)
{
	if(strlen(buff) > 21){
		TVMS_LOG("warnning, tvms_gwip is too long or have noting\n");
		return -1;
	}
	mid_mutex_lock(g_tvms_mutex);
	if(strcmp(tvms_conf_info.tvmsgwip,buff)!= 0){
		strcpy(tvms_conf_info.tvmsgwip,buff);
		tvms_conf_info.dirty = 1;
	}
	mid_mutex_unlock(g_tvms_mutex);
	return 0;
}

extern "C" int
tvms_conf_tvmsgwip_get(char *buff)
{
	if(buff == NULL)
		return -1;
//	mid_mutex_lock(g_tvms_mutex);
	strcpy(buff,tvms_conf_info.tvmsgwip);
//	mid_mutex_unlock(g_tvms_mutex);
	return 0;
}

extern "C" int
tvms_conf_tvmsheartbiturl_set( const char *buff)
{
	if(strlen(buff) > 255 ){
		TVMS_LOG("warnning, tvms_tvmsheartbiturl is too long\n");
		return -1;
	}
	mid_mutex_lock(g_tvms_mutex);
	if(strcmp(buff,tvms_conf_info.tvmsheartbiturl) != 0){
		if(strlen(buff) == 0 || strncmp(buff,"http://",7) != 0){
			tvms_conf_info.tvmsheartbiturl[0] = '\0';
		}
		else
			strcpy(tvms_conf_info.tvmsheartbiturl,buff);
		tvms_conf_info.dirty = 1;
	}
	mid_mutex_unlock(g_tvms_mutex);
	return 0;
}

extern "C" int
tvms_conf_tvmsheartbiturl_get(char *buff)
{
	if(buff == NULL)
		return -1;

	mid_mutex_lock(g_tvms_mutex);

	if(strlen(tvms_conf_info.tvmsheartbiturl) == 0){
		char tembuff[22] = {0};
		tvms_conf_tvmsgwip_get(tembuff);
		if(strlen(tembuff) == 0)
				buff[0] = '\0';
		else{
			snprintf(buff,256,"http://%s/gateway/query.do?",tembuff);
		}
	}
	else
		strcpy(buff,tvms_conf_info.tvmsheartbiturl);
	mid_mutex_unlock(g_tvms_mutex);
	return 0;
}

extern "C" int
tvms_conf_tvmsvodheartbiturl_set( const char *buff)
{
	if(strlen(buff) > 255){
		TVMS_LOG("warnning, tvms_tvmsvodheartbiturl is too long\n");
		return -1;
	}
	mid_mutex_lock(g_tvms_mutex);
	if(strcmp(buff,tvms_conf_info.tvmsvodheartbiturl) != 0){
		if(strlen(buff) == 0 || strncmp(buff,"http://",7) != 0){
			tvms_conf_info.tvmsvodheartbiturl[0] = '\0';
		}
		else
			strcpy(tvms_conf_info.tvmsvodheartbiturl,buff);
		tvms_conf_info.dirty = 1;
	}
	mid_mutex_unlock(g_tvms_mutex);
	return 0;
}

extern "C" int
tvms_conf_tvmsvodheartbiturl_get(char *buff)
{
	if(buff == NULL)
		return -1;
	mid_mutex_lock(g_tvms_mutex);
	if(strlen(tvms_conf_info.tvmsvodheartbiturl) == 0){
		char tembuff[22] = {0};
		tvms_conf_tvmsgwip_get(tembuff);
		if(strlen(tembuff) == 0)
			buff[0] = '\0';
		else{
			snprintf(buff,256,"http://%s/gateway/query.do?",tembuff);
		}
	}
	else
		strcpy(buff,tvms_conf_info.tvmsvodheartbiturl);
	mid_mutex_unlock(g_tvms_mutex);
	return 0;
}

extern "C" int
tvms_conf_tvmsheartbitinterval_set(int tvmsheartbitinterval)
{
	if(tvms_conf_info.tvmsheartbitinterval != tvmsheartbitinterval){
		tvms_conf_info.tvmsheartbitinterval = tvmsheartbitinterval;
		tvms_conf_info.dirty = 1;
	}
	return 0;
}

extern "C" int
tvms_conf_tvmsheartbitinterval_get(int *tvmsheartbitinterval)
{
	*tvmsheartbitinterval = tvms_conf_info.tvmsheartbitinterval;
	return 0;
}

extern "C" int
tvms_conf_tvmsdelaylength_set(int delaylength)
{
	if(tvms_conf_info.tvmsdelaylength != delaylength){
		tvms_conf_info.dirty = 1;
		tvms_conf_info.tvmsdelaylength = delaylength;
	}
	return 0;
}

extern "C" int
tvms_conf_tvmsdelaylength_get(int *delaylength)
{
	*delaylength = 	tvms_conf_info.tvmsdelaylength;
	return 0;
}

extern "C" void
tvms_mediaurl_mediacode(char *mediacode, const char * mediaurl)
{
    char *pbuf = NULL;
    char *pch = NULL;
    char *pchEnd = NULL;
    int i = 0;

    if(!mediacode || !mediaurl){
        TVMS_LOG("mediacode or mediaurl is null!\n");
         return;
    }

    pbuf = (char *)malloc(strlen(mediaurl)+1);
    if(!pbuf){
        TVMS_LOG("malloc failed!\n");
        return;
    }
    memset(pbuf, 0, strlen(mediaurl)+1);
    strcpy(pbuf, mediaurl);

    pch = pbuf;

    for(i = 0; i < 5; i++){
        pch = strchr(pch, ',');
        if(!pch){
            TVMS_LOG("url not error!\n");
            return;
        }
        pch += 1;
    }

    pchEnd = strchr(pch, ',');
    if(!pchEnd){
        TVMS_LOG("URL not error!\n");
        return;
    }
    *pchEnd = 0;
    strcpy(mediacode, pch);

    return;
}



//int tvms_conf_tvmsopenflag_set(int flag)
//{
//	if(tvms_conf_info.tvmsopenflag == flag)
//		return -1;
//	tvms_conf_info.tvmsopenflag = flag;
//
//	return 0;
//}
//int tvms_conf_tvmsopenflag_get(int *flag)
//{
//	if(tvms_conf_info.exist_flag == 0)
//		return -1;
//	*flag = tvms_conf_info.tvmsopenflag;
//	return 0;
//}

//void default_tvms_conf_set()
//{
//	init_tvms_conf();
//	tvms_conf_tvmsheartbiturl_set("http://192.168.0.109:8080/gateway/query.do?");
//	tvms_conf_tvmsvodheartbiturl_set("http://192.168.0.109:8080/gateway/query.do?");
//	tvms_conf_tvmsheartbitinterval_set(30);
//	tvms_conf_tvmsdelaylength_set(20);
//	tvms_conf_tvmsopenflag_set(1);
//	tvms_conf_save();
//	char buff[520] = {0};
//	tvms_conf_tvmsheartbiturl_get(buff);
//	TVMS_LOG\("default_tvms_conf_set = %s\n",buff);
//	tvms_conf_load();
//	memset(buff,0,520);
//	tvms_conf_tvmsheartbiturl_get(buff);
//	TVMS_LOG\("default_tvms_conf_set2 = %s\n",buff);
//	memset(buff,0,520);
//	tvms_conf_tvmsvodheartbiturl_get(buff);
//	TVMS_LOG\("default_tvms_conf_set 3= %s\n",buff);
//	TVMS_LOG\("=========%d=%d=%d\n",tvms_conf_info.exist_flag,tvms_conf_info.tvmsheartbitinterval,tvms_conf_info.tvmsdelaylength);
//	return;
//}

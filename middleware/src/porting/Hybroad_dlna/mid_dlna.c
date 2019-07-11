#ifdef INCLUDE_DLNA
#include <stdio.h>
#include <unistd.h>
#include "semaphore.h"

#include "mid_dlna_ex.h"

#include "hitTime.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "dlna_api.h"
#include "Hippo_api.h"
//#include "log.h"
#include "libzebra.h"
#include "json/json.h"
#include "dms.h"

#include "BrowserBridge/Huawei/BrowserEventQueue.h"

extern int StartscanDmsList( const char* aFieldName,char *deviceID,char *out,int len);
extern void DmsList_jseAPI_Register(ioctl_context_type_e type);
extern int DeletescanDmsList( const char* aFieldName,const char *deviceID,char *out,int len);

#define HUAWEI_V1R5
//#define INCLUDE_DMS
#if 0	//if dlna crashs in customer's home after release, please enable this semphore
#define SEM_WAIT()			mid_dlna_sem_wait()
#define SEM_POST()			mid_dlna_sem_post()
#else
#define SEM_WAIT()			{}
#define SEM_POST()			{}
#endif

int g_dlna_homenas_found = 0; /* only for debug js interface by c*/
int g_dlna_running_flag = 0;
static sem_t s_dlna_sem;
static int dlna_dmp = 1;/* only for debug */

static char s_dlna_interface[128] = {0};
static char s_dlna_ip[32] = {0};
static char s_dlna_mask[32] = {0};

void mid_dlna_sem_wait(void)
{
	sem_wait(&s_dlna_sem);
}
void mid_dlna_sem_post(void)
{
	sem_post(&s_dlna_sem);
}

/*-----------------------------------------------------------------------------------------*/
/* js-related as below */
/*-----------------------------------------------------------------------------------------*/
/* var sValue =Utility.setValueByName('dlna.setFriendlyName','{"friendlyName":"friendlyName"}') */
static int s_Jse_Dlna_setFriendlyName(const char *func, const char *para, char *value, int len)
{
    char *p1, *p2;
    json_object *json, *j1;

	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, func);
    p1 = strchr(value, '{');
    if(!p1)
        return -1;

    json = dlna_json_tokener_parse(p1);
    if(json)
    {
	    j1 = json_object_object_get(json, "friendlyName");
	    if(j1 && (p2 = (char*)json_object_get_string(j1)) )

	    json_object_put(json);
    }

	HT_DBG_FUNC_END(0 , para);
	return 0;
}

static int s_Jse_Dlna_getFriendlyName(const char *func, const char *para, char *value, int len)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, func);
	strncpy(value, "device", len - 1);
	HT_DBG_FUNC_END(0 , value);
	return 0;
}

#ifdef HUAWEI_V1R5
int Dmc_HuaweiJse_V1R5_Read(const char *func, const char *para, char *value, int len)
{
    SEM_WAIT();
	int ret = Dmc_HuaweiJse_V1R5(func, para, value, len, 1);
    SEM_POST();
    return ret;
}
int Dmc_HuaweiJse_V1R5_Write(const char *func, const char *para, char *value, int len)
{
    SEM_WAIT();
    int ret = Dmc_HuaweiJse_V1R5(func, para, value, len, 0);
    SEM_POST();
    return ret;
}
static void s_HuaweiJS_JseMap(int type)
{
    a_Hippo_API_JseRegister( "dlna.getDmsCount", 		Dmc_HuaweiJse_V1R5_Read, NULL, type );
    a_Hippo_API_JseRegister( "dlna.getDmsList", 		Dmc_HuaweiJse_V1R5_Read, NULL, type );
    a_Hippo_API_JseRegister( "dlna.openFileList", 		Dmc_HuaweiJse_V1R5_Read, NULL, type );
    a_Hippo_API_JseRegister( "dlna.getCount", 			Dmc_HuaweiJse_V1R5_Read, NULL, type );
    a_Hippo_API_JseRegister( "dlna.getList", 			Dmc_HuaweiJse_V1R5_Read, NULL, type );
    a_Hippo_API_JseRegister( "dlna.getAllContainerList",    Dmc_HuaweiJse_V1R5_Read, NULL, type );

    a_Hippo_API_JseRegister( "dlna.setFriendlyName",    NULL, s_Jse_Dlna_setFriendlyName, type );
}

#else
static int s_Jse_SubtitleEnableFlag(const char *func, const char *para, char *value, int len)
{
	if(atoi(value))
		ymm_decoder_showSubtitle( );
	else
		ymm_decoder_hideSubtitle( );
	return 0;
}
static int s_Jse_Local_Play_setPostion(const char *func, const char *para, char *value, int len)
{
	int x,y,w,h;
	if(sscanf(value, "%d, %d, %d, %d", &x, &y, &w, &h) == 4 )
	{
		//app_stream_rect( x, y, w, h);NODEFINE_DINGLEI
	}
	return 0;
}
int Dmc_HuaweiJse_Read(const char *func, const char *para, char *value, int len)
{
    SEM_WAIT();
    int ret = Dmc_HuaweiJse(func, para, value, len, 1);
    SEM_POST();
    return ret;
}
int Dmc_HuaweiJse_Write(const char *func, const char *para, char *value, int len)
{
    SEM_WAIT();
    int ret = Dmc_HuaweiJse(func, para, value, len, 0);
    SEM_POST();
    return ret;
}
int Dmp_HuaweiJse_Read(const char *func, const char *para, char *value, int len)
{
    SEM_WAIT();
    int ret = Dmp_HuaweiJse(func, para, value, len, 1);
    SEM_POST();
    return ret;
}
int Dmp_HuaweiJse_Write(const char *func, const char *para, char *value, int len)
{
    SEM_WAIT();
    int ret = Dmp_HuaweiJse(func, para, value, len, 0);
    SEM_POST();
}
static void s_HuaweiJS_JseMap(int type)
{
	//browsing interface
	a_Hippo_API_JseRegister( "DeviceNumber_Get", 		Dmc_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "DeviceList_Get", 			Dmc_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "ItemNumber_Get", 			Dmc_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "ItemList_Get", 			Dmc_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "Item_Delte", 				NULL, Dmc_HuaweiJse_Write, type );
#ifdef STBTYPE_MAROC
	a_Hippo_API_JseRegister( "ContainerNumber_Get", 	Dmc_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "ContainerList_Get", 		Dmc_HuaweiJse_Read, NULL, type );
#endif
	a_Hippo_API_JseRegister( "ContItemNumber_Get", 		Dmc_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "ContItemList_Get", 		Dmc_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "dlna.statutsTest.triggered",NULL, Dmc_HuaweiJse_Write, type );

	// playback interface
	a_Hippo_API_JseRegister( "PlayerInstance_Creat",	Dmp_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "PlayerInstance_Release",	NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "PlayerInstance_Switch",	NULL, Dmp_HuaweiJse_Write, type );

    a_Hippo_API_JseRegister( "MuteState_Get",			Dmp_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "MuteState_Set",			NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "Volume_Get",				Dmp_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "Volume_Set", 				NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "Audio_Channel_Set", 		NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "Audio_Channel_Get", 		Dmp_HuaweiJse_Read, NULL, type );

	a_Hippo_API_JseRegister( "dlna.AllSubtitleInfo",		Dmp_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "dlna.AllAudioTrackInfo", 		Dmp_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "dlna.CurrentSubtitleInfo", 	Dmp_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "dlna.CurrentAudioTrackInfo", 	Dmp_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "dlna.hw_op_stb", 				NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "dlna.hw_op_subtitle_select", 	NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "dlna.hw_op_audiotrack_select",NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "dlna.SubtitleEnableFlag",		NULL, s_Jse_SubtitleEnableFlag, type );

	a_Hippo_API_JseRegister( "Local_Play_setPostion", 	NULL, s_Jse_Local_Play_setPostion, type );
	a_Hippo_API_JseRegister( "Local_Play", 				NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "Local_Stop", 				NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "Local_Pause", 			NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "Local_Resume", 			NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "Local_Seek", 				NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "Duration_Get",			Dmp_HuaweiJse_Read, NULL, type );
	a_Hippo_API_JseRegister( "Postion_Get", 			Dmp_HuaweiJse_Read, NULL, type );

	a_Hippo_API_JseRegister( "setTrickPlayModel", 		NULL, Dmp_HuaweiJse_Write, type );
	a_Hippo_API_JseRegister( "getTrickPlayModel",		Dmp_HuaweiJse_Read, NULL, type );

#ifdef STBTYPE_QTEL
	a_Hippo_API_JseRegister( "dlna.setFriendlyName",    NULL, s_Jse_Dlna_setFriendlyName, type );
	a_Hippo_API_JseRegister( "friendlyName",			s_Jse_Dlna_getFriendlyName, NULL,type );
#endif
}
#endif

/*-----------------------------------------------------------------------------------------*/
/* internal sub functions as below */
/*-----------------------------------------------------------------------------------------*/
static int s_dlna_module_EventHandler(enum_DlnaEvent type,int handle, char *str)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, type, str);
	int ret = -1;
	switch(type)
	{
		case enum_DlnaEvent_JsonString:
			//if(str && strstr(str, "TwonkyMedia"))
			if(str && strstr(str, "STB 192.168.1.240"))
				g_dlna_homenas_found = 1;
			ret = browserEventSend(str, NULL);
			break;

		case enum_DlnaEvent_OnlyVirtualKey:
			//ret = BrowserInputMsg_Rtn(767); NODEFINE_DINGLEI
			break;

		default:
			break;
	}
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int s_dlna_module_get_ip(void)
{
	int ret = 0;
	char			dlna_interface[128] = {0};
	char			dlna_ip[128] = {0};
	char			dlna_mask[128] = {0};

	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, 0);

/*#if(defined(DLNA_TEST_MODE) && (DLNA_TEST_MODE==DLNA_TEST_OVERSEA) )
	strcpy(dlna_interface, "eth0");
	mid_dlna_getlocalhostname(dlna_ip);
	strcpy(dlna_mask, "255.0.0.0");
*/
    network_default_ifname(dlna_interface, 32);
    network_address_get(dlna_interface, dlna_ip, sizeof(dlna_ip));
    network_netmask_get(dlna_interface, dlna_mask, sizeof(dlna_mask));

	if( strcmp(s_dlna_interface, dlna_interface) || strcmp(s_dlna_ip, dlna_ip) || strcmp(s_dlna_mask, dlna_mask))
	{
		strcpy(s_dlna_interface,	dlna_interface);
		strcpy(s_dlna_ip,			dlna_ip);
		strcpy(s_dlna_mask, 		dlna_mask);
		ret = 1;
	}

	HT_DBG_FUNC(0, s_dlna_interface);
	HT_DBG_FUNC(0, s_dlna_ip);
	HT_DBG_FUNC_END(ret, s_dlna_mask);
	return ret;
}


static void s_dlna_add_str(void *handle, char *key, char *val)
{
	json_object_object_add((json_object *)handle, key, json_object_new_string(val));
}
static int s_dlna_module_start(char *dlna_interface, char* ip)
{
	int ret = -1;

	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_FEW, 0, ip);

	ret = UpnpStackInit(dlna_interface, ip, DLNA_CFG_PATH);
	HT_DBG_FUNC(ret, "ret = ");
	if( ret )
        return ret;

	HT_DBG_FUNC(dlna_dmp, "dlna_dmp =");
	if(dlna_dmp)
	{
	 	json_object *dmc = json_object_new_object();
#if defined(ENABLE_DLNA_TEST)
		s_dlna_add_str(dmc,  "show_inner_dms", 		"true");
#endif
		s_dlna_add_str(dmc,  "dms_total_limit", 	"32");
		HT_DBG_FUNC(dlna_dmp, json_object_to_json_string(dmc));
		Dmc_Init((char*)json_object_to_json_string(dmc));
		json_object_put(dmc);
#if defined(HUAWEI_V1R5)
		s_HuaweiJS_JseMap(IoctlContextType_eHWBase);
#else
		s_HuaweiJS_JseMap(IoctlContextType_eHWBaseC20);
#endif
		DmsList_jseAPI_Register(IoctlContextType_eHWBase);
		Dmc_Start();
	}

#ifdef INCLUDE_DMS
	mid_dlna_init_dms();
//	 int port=UpnpGetServerPort();
//	render_start(s_dlna_ip, port);

#endif
#ifdef INCLUDE_DMR
	char name[128] = {0};
	appSettingGetString("userSTBName", name, 127, 0);

	Dmr_Init(name);
	//Dmr_Start(DMREventCallback);
	DMRStart();
#endif

	g_dlna_running_flag = 1;
	HT_DBG_FUNC_END(0, dlna_interface);
	return 0;
}
static int s_dlna_module_restart(char *dlna_interface, char* ip)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_FEW, 0, ip);
	int rc = UpnpStackInit(dlna_interface, ip, DLNA_CFG_PATH);
	if( rc )
		return rc;

	if(dlna_dmp)
		Dmc_Start();

#ifdef INCLUDE_DMS
	mid_dlna_start_dms();
#endif
	//Dmr_Start(DMREventCallback);
#ifdef INCLUDE_DMR
	char name[128] = {0};
	appSettingGetString("userSTBName", name, 127, 0);
	Dmr_Init(name);
	DMRStart();
#endif
	HT_DBG_FUNC_END( 0, dlna_interface);
	return 0;
}
static int s_dlna_module_stop(void)
{
	if(dlna_dmp)
		Dmc_Stop();

#ifdef INCLUDE_DMS
	mid_dlna_stop_dms();
#endif
	Dmr_Stop();
	UpnpStackDestroy();
	return 0;
}

/*-----------------------------------------------------------------------------------------*/
/* public functions as below */
/*-----------------------------------------------------------------------------------------*/
int mid_dlna_start(int restart)
{
	int ret = 1;
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, ret, s_dlna_ip);
    if(!ret)
		return -1;
//	mid_dlna_sem_wait();

	ret = s_dlna_module_get_ip();
	if(!g_dlna_running_flag)
		ret = s_dlna_module_start(s_dlna_interface, s_dlna_ip);
	else
	{
		if(ret)
		{
			s_dlna_module_stop();
			ret = s_dlna_module_restart(s_dlna_interface, s_dlna_ip);
		}
	}

//	mid_dlna_sem_post();
	HT_DBG_FUNC_END(ret, s_dlna_ip);

//    StartscanDmsList(NULL,NULL,NULL,0);

	return ret;
}
int mid_dlna_stop()
{
	if(1 == g_dlna_running_flag)
	{
		s_dlna_module_stop();
		g_dlna_running_flag = 0;
	}
	return 0;
}
int mid_dlna_restart(void)
{
	int ret = -1;

	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, g_dlna_running_flag, s_dlna_ip);
   // mid_dlna_sem_wait();

	if(g_dlna_running_flag)
	{
		if(s_dlna_module_get_ip())
		{
			s_dlna_module_stop();
			ret = s_dlna_module_restart(s_dlna_interface, s_dlna_ip);
		}
	}

    //mid_dlna_sem_post();
	HT_DBG_FUNC_END(ret, s_dlna_ip);
	return ret;
}



void mid_dlna_GetEvent(unsigned int msgno, int type, int stat)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_MANY, type, 0);

#if(defined(ENABLE_DLNA_TEST) && (DLNA_TEST_MODE==DLNA_TEST_HW_JS) )
	if(!mid_dlna_test(msgno, type, stat))
		return;
#endif

	if(YX_EVENT_SYSTEM == type)
	{
		SEM_WAIT();
		if(g_dlna_running_flag)
			HYSDK_AVPlayer_GetEvent(msgno, type, stat);
		SEM_POST();
	}

#ifdef INCLUDE_DMS
	if(YX_EVENT_MOUNT == type || YX_EVENT_UNMOUNT == type)
	{
		SEM_WAIT();
		if(g_dlna_running_flag)
			mid_dlna_UpdateMountList();
		SEM_POST();
	}

	#ifdef STBTYPE_QTEL
	if(YX_EVENT_KEYDOWN == type && YX_FP_POWER == msgno)
		mid_dlna_stop_dms();
	#endif
#endif

	HT_DBG_FUNC_END(msgno, 0);
}


/*-----------------------------------------------------------------------------------------*/
/* the inialization */
/*-----------------------------------------------------------------------------------------*/
/* standard log setting as below,  it only can be temporarily modified by testing engineers to capture log*/
static void s_dlna_log_default_setting(void)
{
    HT_EnableModule(HT_MOD_UPNP);
	HT_EnableModule(HT_MOD_DMC);
	HT_EnableModule(HT_MOD_DMS);
	HT_EnableModule(HT_MOD_DMR);
	HT_EnableModule(HT_MOD_APP);
	HT_EnableModule(HT_MOD_HYSDK);
	HT_EnableModule(HT_MOD_IPC);

	HT_EnableLevel(HT_BIT_KEY);
	HT_EnableLevel(HT_BIT_FEW);
	HT_EnableLevel(HT_BIT_MANY);
	HT_EnableLevel(HT_BIT_MYRIAD);
	//HT_SetDelay(2);
}
static int s_dlna_log_printf( char * str, int len)
{
//	printf("%s",str);
	DLNA_LOG("%d [%d] :%s",__FILE__,__LINE__,str);
}
int mid_dlna_init(void)
{
	enum_DlnaAppMode mode;
    enum_DlnaEvent event_ype = enum_DlnaEvent_JsonString;

	sem_init(&s_dlna_sem,0,1);
	mid_dlna_sem_wait();

	HT_Initialize(s_dlna_log_printf);
#ifdef ENABLE_DLNA_TEST
    /* personal  log setting as below, dlna developer sets it by your requirement  for debugging or test */
    HT_EnableModule(HT_MOD_UPNP);
	HT_EnableModule(HT_MOD_DMC);
	HT_EnableModule(HT_MOD_DMS);
	HT_EnableModule(HT_MOD_DMR);
	HT_EnableModule(HT_MOD_APP);
	HT_EnableModule(HT_MOD_HYSDK);
	HT_EnableModule(HT_MOD_IPC);

	HT_EnableLevel(HT_BIT_KEY);
	HT_EnableLevel(HT_BIT_FEW);
	HT_EnableLevel(HT_BIT_MANY);
	HT_EnableLevel(HT_BIT_MYRIAD);
	HT_SetDelay(-30);
#else
//	s_dlna_log_default_setting();
 /*   HT_EnableModule(HT_MOD_UPNP);
	HT_EnableModule(HT_MOD_DMC);
	HT_EnableModule(HT_MOD_DMS);
	HT_EnableModule(HT_MOD_DMR);
	HT_EnableModule(HT_MOD_APP);
	HT_EnableModule(HT_MOD_HYSDK);
	HT_EnableModule(HT_MOD_IPC);

	HT_EnableLevel(HT_BIT_KEY);
	HT_EnableLevel(HT_BIT_FEW);
	HT_EnableLevel(HT_BIT_MANY);
	HT_EnableLevel(HT_BIT_MYRIAD);
	HT_SetDelay(-30);
*/
#endif

#ifdef INCLUDE_DMS
	Dms_SharePvr_Preset();
#endif

#ifdef ENABLE_DLNA_TEST
	//Dlna_IpcModeInit(enum_DlnaIPC_InSameProcess, 0);//enum_DlnaIPC_InSameProcess
	Dlna_IpcModeInit(enum_DlnaIPC_NULL, 0);
#else
	Dlna_IpcModeInit(enum_DlnaIPC_NULL, 0);
#endif


#if defined(STBTYPE_MAROC)
	mode = enum_DlnaAppMode_HUAWEI_MAROC;
#elif defined(HUAWEI_V1R5)
	mode = enum_DlnaAppMode_HUAWEI_V1R5;
#elif defined(STBTYPE_QTEL)
	mode = enum_DlnaAppMode_HUAWEI_QTEL;
#else
	mode = enum_DlnaAppMode_HUAWEI;
#endif

#if(defined(ENABLE_DLNA_TEST))
	mode = enum_DlnaAppMode_HUAWEI;
#endif
#if(defined(ENABLE_DLNA_TEST) && (DLNA_TEST_MODE==DLNA_TEST_OVERSEA) )
	mode = enum_DlnaAppMode_ImitatedObject;
#endif

	if(mode == enum_DlnaAppMode_ImitatedObject)
		event_ype = enum_DlnaEvent_OnlyVirtualKey;
	Dlna_ModeInit(mode, NULL, s_dlna_module_EventHandler, event_ype);
	Set_RmDmsFileList_Callback(DeletescanDmsList);
	mid_dlna_sem_post();
	return 0;
}

/*-----------------------------------------------------------------------------------------*/
/* the end */
/*-----------------------------------------------------------------------------------------*/
#endif



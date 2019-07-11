#ifdef INCLUDE_DLNA
#ifdef INCLUDE_DMS
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "semaphore.h"

#include "json/json.h"
#include "upnp/LinkedList.h"
//#include "../localPlayer/Hy_Api_LocalPlayer.h"

#include "dlna_api.h"
#include "mid_dlna_ex.h"
#include "hitTime.h"
#include "dms.h"


extern int g_dlna_running_flag;
extern void mid_dlna_sem_wait(void);
extern void mid_dlna_sem_post(void);

typedef enum {
	x_IP_CHANGED = 1,
	x_NORMAL,
	x_PVR,
	x_YOUTUBE,
} x_DO_DMS;

static sem_t s_device_sem;
static LinkedList s_new_device, s_old_device, *pNewDevice = NULL, *pOldDevice = NULL;
static int dlna_dms =1;/* only for debug */
static int dms_running = -1; /* >0: runing; <0: stopped */


void mid_dlna_disable_dms(void)
{
	dlna_dms = 0;
}

int s_Dms_AddUsbRoot(int parent_id, char *keyword, char *title, int object_type, int source_type)
{
	char *info = NULL;
	
	C_DMS_CCI *container = Dms_VirtualObject_CreateContainerSelf(keyword, title, &Container_MIME_Type);
	C_DMS_VO *entry = Dms_VirtualObject_Create(parent_id, container, object_type, source_type, NULL, NULL);
	Dms_VirtualObject_To_String(entry, 0, &info);
	
#ifdef ENABLE_DLNA_TEST
	{
		char *xinfo = NULL;
		C_DMS_VO *xentry = NULL;
		Dms_VirtualObject_From_String(info, &xentry);
		if(xentry)
		{
			Dms_VirtualObject_To_String(xentry, 0, &xinfo);
			Dms_VirtualObject_Free(xentry);
			if(xinfo)
				free(xinfo);
		}
	}
#endif	

	int id = Dms_VirtualFileTree_AddChild(parent_id, info);
	Dms_VirtualObject_Free(entry);
	if(info)
		free(info);

	return id;
}

static ListNode *s_Dms_IsNodeExist(LinkedList *list, ListNode *aim)
{
  	ListNode *node = ListHead(list);
	while(node)
	{
		if(	strcmp(aim->item, node->item) == 0 )
			return node;
		node = ListNext(list, node);
	}
	return NULL;
}
void mid_dlna_UpdateMountList(void)
{
//#if defined(STBTYPE_HUNGARY) || defined(ENABLE_DLNA_TEST)
	char path[1024], fs[128],name[128],pName[128];
	int totalSize, freeSize, ret, type;
	int position,storageFlag;
    if(!dlna_dms)
		return;

	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, NULL);
	sem_wait(&s_device_sem);

	int i = getDiskCount();
	HT_DBG_FUNC(i, "HApi_LPlayer_Device_GetCount = ");
	while(i--)
	{
		ret = getDiskInfo(i, name, 128, &totalSize, &position, &storageFlag);
		// 0;//_Device_GetInfo(i, &type, path, sizeof(path), &totalSize, &freeSize);
		//HT_DBG_FUNC(ret, path);
		if(ret)
			continue;
		//if(type == 0) //0: internal HDD; 1:external HDD
		//	continue;

		int j =  getPartitionCount(name);//_Partition_GetCount(i);
		
		HT_DBG_FUNC(j, "HApi_LPlayer_Partition_GetCount = ");
		while(j--)
		{
			//ret = 0;//_Partition_GetInfo(i, j,  path, sizeof(path), &totalSize, &freeSize, fs, sizeof(fs));
			ret = getPartitionInfo(name, j, pName,128, &path, 128, &totalSize, &freeSize, fs, 128, &storageFlag);
			HT_DBG_FUNC(ret, path);
			if( ret )
				continue;

			coo_str_rid_tail(path, '/');
			ListAddTail(pNewDevice, strdup(path));
		}
	}

	HT_DBG_FUNC(2,NULL);
	ListNode *pNew, *pOld;
	pNew = ListHead(pNewDevice);
	while(pNew)
	{
		pOld = s_Dms_IsNodeExist(pOldDevice, pNew);
		if( pOld )
		{
			free(pOld->item);
			ListDelNode(pOldDevice, pOld, 0);
		}
		else
		{
			char *fullpath = pNew->item;
			char *title = strrchr(fullpath, '/') + 1;
			int id = s_Dms_AddUsbRoot(0, fullpath, title, e_OBJTYPE_CONTAINER, e_SOURCE_USB);
			Dms_ShareUsb_AddFolder(id, fullpath);
		}

		pNew = ListNext(pNewDevice, pNew);
	}

	HT_DBG_FUNC(3,NULL);
	pOld = ListHead(pOldDevice);
	while(pOld)
	{
		int id = Dms_VirtualFileTree_GetChildID(0, e_OBJTYPE_CONTAINER, pOld->item, 0);
		Dms_VirtualFileTree_DeleteChild(0, id);

		free(pOld->item);
		ListDelNode(pOldDevice, pOld, 0);
		pOld = ListHead(pOldDevice);
	}

	HT_DBG_FUNC(4,NULL);
	LinkedList*temp = pOldDevice;
	pOldDevice = pNewDevice;
	pNewDevice = temp;

	sem_post(&s_device_sem);
	HT_DBG_FUNC_END(8,NULL);
//#endif
}

static void s_dms_start(int mode)
{
	int old_state = dms_running;
	int new_state = 0; 

	if(!g_dlna_running_flag || !dlna_dms)
		return;
	
	switch(mode)
	{
		case x_IP_CHANGED:
		case x_NORMAL:
			if(old_state == -mode)
			{
				Dms_Start();
				new_state = 1;
			}
			break;
			
		case x_PVR:
			if(old_state == -mode)
			{
				Dms_SharePvr_MakeHome();
				Dms_SetStoppingHttp(0);
				Dms_Start();
				new_state = 1;
			}
			break;
			
		case x_YOUTUBE:
			if(old_state == -mode)
			{
				Dms_SetStoppingHttp(0);
				Dms_SetPause(0);
				Dms_Start();
				new_state = 1;
			}
			break;
			
		default:
			break;
	}	
	if(new_state)
		dms_running = new_state;
}
static void s_dms_stop(int mode)
{
	int old_state = dms_running;
	int new_state = 0; 

	if(!g_dlna_running_flag || !dlna_dms)
		return;
	
	switch(mode)
	{
		case x_IP_CHANGED:
		case x_NORMAL:
			if(old_state > 0)
			{
				Dms_Stop();
				new_state = -mode;
			}
			break;
			
		case x_PVR:
			if(old_state > 0)
			{
				Dms_SharePvr_RemoveHome();
				Dms_SetStoppingHttp(1);
				Dms_Stop();
				new_state = -mode;
			}
			break;
			
		case x_YOUTUBE:
			if(old_state > 0)
			{
				Dms_SetStoppingHttp(1);
				Dms_SetPause(1);
				Dms_Stop();
				new_state = -mode;
				usleep(100*1000);
			}
			break;
			
		default:
			break;
	}	

	if(new_state)
		dms_running = new_state;
}
static void s_dlna_add_str(void *handle, char *key, char *val)
{
	json_object_object_add((json_object *)handle, key, json_object_new_string(val));
}
int mid_dlna_init_dms(void)
{   
	if(!dlna_dms)
		return -1;
	
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, 0);
	
	char name[128] = {0};
	json_object *dms = json_object_new_object();
	appSettingGetString("userSTBName", name, 127, 0);
	//memcpy(name,"device",127);
	s_dlna_add_str(dms,  "friendlyName",		name);
	s_dlna_add_str(dms,  "manufacturer",		"Huawei Technologies Co., Ltd");
	s_dlna_add_str(dms,  "manufacturerURL", 	"http://www.huawei.com/");
	
	s_dlna_add_str(dms,  "modelDescription",	"DLNA1.5 MediaServer");
	s_dlna_add_str(dms,  "modelName",			"Huawei DLNA1.5 DMS");
	s_dlna_add_str(dms,  "modelNumber", 		"1");
	s_dlna_add_str(dms,  "modelURL",			"http://www.huawei.com/");
	s_dlna_add_str(dms,  "serialNumber",		"001");
#ifdef ENABLE_DLNA_TEST
	//s_dlna_add_str(dms,  "icon",				DLNA_CFG_PATH"ios_fetchtv_512.png");//B-JPEG_S-12
	s_dlna_add_str(dms,  "icon",				DLNA_CFG_PATH"B-JPEG_S-12.jpg");
#endif	
	s_dlna_add_str(dms,  "object_total_limit",	"5000");
	Dms_Init((char*)json_object_to_json_string(dms));
	json_object_put(dms);
	
	Dms_SharePvr_Init();
	
	sem_init(&s_device_sem,0,1);
	ListInit (&s_new_device, 0, 0);
	ListInit (&s_old_device, 0, 0);
	pNewDevice = &s_new_device;
	pOldDevice = &s_old_device;
	mid_dlna_UpdateMountList();//添加共享目录。遍历文件
	
	Dms_Start();
	dms_running = 1;

	HT_DBG_FUNC_END(0, 0);
    return 0;
}

int mid_dlna_start_dms(void)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, 0);
    mid_dlna_sem_wait();
	
	s_dms_start(x_NORMAL);
	
	mid_dlna_sem_post();
	HT_DBG_FUNC_END(0, 0);
    return 0;
}

int mid_dlna_stop_dms(void)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, 0);
    mid_dlna_sem_wait();
	
	s_dms_stop(x_NORMAL);
	
	mid_dlna_sem_post();
	HT_DBG_FUNC_END(0, 0);
    return 0;
}

int mid_dlna_StartDms_AddPvr(void)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, 0);
    mid_dlna_sem_wait();
	
	s_dms_start(x_PVR);
	
	mid_dlna_sem_post();
	HT_DBG_FUNC_END(0, 0);
    return 0;
}

int mid_dlna_StopDms_RemovePvr(void)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, 0);
    mid_dlna_sem_wait();
	
	s_dms_stop(x_PVR);
	
	mid_dlna_sem_post();
	HT_DBG_FUNC_END(0, 0);
    return 0;
}

int mid_dlna_ResumeDms(void)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, 0);
    mid_dlna_sem_wait();
	
	s_dms_start(x_YOUTUBE);
	
	mid_dlna_sem_post();
	HT_DBG_FUNC_END(0, 0);
    return 0;
}

int mid_dlna_PauseDms(void)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, 0);
    mid_dlna_sem_wait();
	
	s_dms_stop(x_YOUTUBE);
	
	mid_dlna_sem_post();
	HT_DBG_FUNC_END(0, 0);
    return 0;
}


#endif
#endif



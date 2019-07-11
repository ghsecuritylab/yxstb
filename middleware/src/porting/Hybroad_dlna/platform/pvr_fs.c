#ifdef INCLUDE_DLNA	
#ifdef INCLUDE_DMS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <fcntl.h>

#include "json/json.h"
#include "ind_pvr.h"
//include "../localPlayer/Hy_Api_LocalPlayer.h"

#include "mid_dlna_ex.h"
#include "hitTime.h"
#include "dms.h"
#include "dlna_api.h"

//#define Dms_VirtualFileTree_	Raw_Dms_VirtualFileTree_
extern int ISO8859_2toUtf8(unsigned char *stringIn, int stringInLen, unsigned char *utf8string, int *utf8Len );
static int s_Pvr_CharCodeToUtf8(const char *old, char **utf)
{
	char *p;
	int ret = -1, len, utf_len;
	if(!old)
		return ret;
	
	len = strlen(old);
	utf_len = len * 6 + 2;
	p = malloc(utf_len);
	p[0] = 0;
	
//	ret = ISO8859_2toUtf8((unsigned char *)old, len, (unsigned char *)p, &utf_len);
	strcpy(p, old);
	ret = 0;

	*utf = p;
	return ret;
}

/*-----------------------------------------------------------------------------------------*/
/* customer-related */
/*-----------------------------------------------------------------------------------------*/
/*
channelNO	M	String(4)	频道号。当内容来自IPTV时，此参数为必选。
iptvcontentID	O	String(32)	IPTV系统中的内容ID，此信息由PVR或download时传入。
contentSource	O	String(1)	该媒体的来源：
?	contentSource=0，来自用户，无特殊含义
?	contentSource=1，来自IPTV
?	contentSource=2，来自DVB
iptvuserID	O	String(16)	IPTV系统用户帐号。获取该内容的IPTV账号，此信息在创建PVR或download内容时创建。
contentType	O	String(16)	该内容在IPTV系统中的类型，取值含义：
?	contentType=10，此内容为VOD内容；
?	contentType=300，此内容为PVR内容；
?	contentType=600，此内容为IPTV中的TVOD
serialProgramSn	O	String(8)	表示系列录制集数，0~255的数值。只有“folderType”值为3的文件夹中的内容才需要携此属性，
programRate	O	String(8)	节目级别。
programTitle	O	String(256)	该节目的名称，由IPTV或download时下发给DMS，在DVB中则是机顶盒解析SI信息获取。
programDescription	O	string	cPVR的节目描述信息，最长长度不超过1536。

*/
/* Represent the CDS DIDL Message Item Resource Size value. */
#define DIDL_HW_CONTENT_SOURCE 			"contentSource"		//int
#define DIDL_HW_CHANNEL_NO				"channelNO"
#define DIDL_HW_IPTV_CONTENT_ID			"iptvcontentID"
#define DIDL_HW_IPTV_USER_ID			"iptvuserID"
#define DIDL_HW_CONTENT_TYPE			"contentType"
#define DIDL_HW_PROGRAM_TITLE			"programTitle"
#define DIDL_HW_LOCAL_ENCRYPTED			"localEncrypted"	//int
#define DIDL_HW_LOCAL_ENCRYPTION_KEY	"localEncryptionKey"
#define DIDL_HW_PROGRAM_RATE			"programRate"

#define DIDL_HW_SERIAL_TITLE			"title"
#define DIDL_HW_SERIAL_PROGRAM_SN		"serialProgramSn"
#define DIDL_HW_PROGRAM_DESCRIPTION		"programDescription"
#define DIDL_HW_FOLDER_TYPE				"folderType"

#define DIDL_HW_PARENT_TASK_NAME		"parentTaskName"
#define DIDL_HW_SERIAL_TASK_NAME		"serialTaskName"
#define DIDL_HW_PERIOD_TASK_NAME		"periodTaskName"

struct json_object* s_json_object_new_string(const char *str)
{
	return json_object_new_string( str? str : "");
}
static void s_PvrInfo_Add_Int(void *list, char *key, int val)
{
	struct json_object *json = (json_object*)list;
	char temp[32];
	sprintf(temp, "%d", val);
	json_object_object_add(json, key, s_json_object_new_string(temp));
}
static void s_PvrInfo_Add_String(void *list, char *key, const char *val)
{
	struct json_object *json = (json_object*)list;
	json_object_object_add(json, key, s_json_object_new_string(val));
}
static int s_PvrInfo_Add_PvrRes(json_object *old_list, char *old_key,  int is_int, void *new_list, char *key )
{
	struct json_object *json = json_object_object_get(old_list, old_key);
	if(!json)
		return 0;
	
	if( is_int )
	{
		int x = json_object_get_int(json);
		s_PvrInfo_Add_Int(new_list, key, x);
	}
	else
	{
		const char *str = json_object_get_string(json);
		char *utf = NULL;
		s_Pvr_CharCodeToUtf8(str, &utf);
		if(utf)
		{
			s_PvrInfo_Add_String(new_list, key, utf);
			free(utf);
		}
		else
			s_PvrInfo_Add_String(new_list, key, str);
	}
	
	return 1;
}

/*	contentSource=0，来自用户，无特殊含义
	contentSource=1，来自IPTV
	contentSource=2，来自DVB*/
static int s_PvrInfo_ContentSource(int type)// not correct, need to strictly correct by huawei's definition
{
	switch(type)
	{
		case 0:
		case 3:
		case 6:
			return 1;//iptv, network
		case 1:
			return 2;//dvb
			
		default:
			break;
	}
	return 0;
}

/*	contentType=10，此内容为VOD的下载内容
	contentType=300，此内容为PVR内容
	contentType=600，此内容为TVOD的下载内容 	*/
static const char *s_PvrInfo_ContentType(int type)
{
	switch(type)
	{
		case 0:
		case 1:
            return "300";
		case 3:
			return "10";
		case 6:
			return "600";
			
		default:
			break;
	}
	return NULL;
}


/*
VOD JSON举例：
json_add_str(json_buf, "dir_name", p_down_node->dir_name);
json_add_str(json_buf, "name", p_down_node->name);
json_add_str(json_buf, "content_id", p_down_node->content_id);
json_add_str(json_buf, "content_name", p_down_node->content_name);
json_add_int(json_buf, "type", atoi(p_down_node->service_type)+3);
json_add_str(json_buf, "dl_type", p_down_node->type);
json_add_str(json_buf, "drm_type", p_down_node->drm_type);
json_add_str(json_buf, "metadata", p_down_node->metadata);
json_add_str(json_buf, "url", p_down_node->url);
json_add_str(json_buf, "rate", p_down_node->rate);
json_add_str(json_buf, "download_model", p_down_node->download_model);
json_add_str(json_buf, "service_type", p_down_node->service_type);
json_add_str(json_buf, "start_time", p_down_node->start_time);
json_add_str(json_buf, "task_id", p_down_node->task_id);
json_add_int(json_buf, "status", p_down_node->status);
json_add_int(json_buf, "pid", p_down_node->pid);
json_add_longlong(json_buf, "file_size", p_down_node->file_size);
json_add_longlong(json_buf, "total_size", p_down_node->total_size);
   
json_add_str(json_buf, "parental_rating", p_down_node->parental_rating);
json_add_str(json_buf, "program_description", p_down_node->program_description);
json_add_int(json_buf, "auto_delete", p_down_node->auto_delete);
json_add_int(json_buf, "reserved_duration", p_down_node->reserved_duration);
json_add_int(json_buf, "period_of_Subscribe", p_down_node->period_of_Subscribe);
json_add_int(json_buf, "dl_finish_time", p_down_node->dl_finish_time);
json_add_int(json_buf, "fst_play_time", p_down_node->fst_play_time);
sysNtvuserGet(user);
json_add_str(json_buf, "user_id", user);
*/
static struct json_object *s_VodInfo_To_Extension(PvrFInfo *p, struct json_object *old_json, int xtype)
{	
	int x;
	const char *str;
	char temp[64] = "";

	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_MANY, (int)xtype, 0);
	
	struct json_object * new_obj = json_object_new_object();
	if( !new_obj )
		return new_obj;
	
	x = s_PvrInfo_ContentSource(xtype);
	s_PvrInfo_Add_Int(new_obj, DIDL_HW_CONTENT_SOURCE, x);
	str = s_PvrInfo_ContentType(xtype);
	if( str)
		s_PvrInfo_Add_String(new_obj, DIDL_HW_CONTENT_TYPE, str);
	
	/* programTitle programRate*/
	s_PvrInfo_Add_PvrRes(old_json, "content_id", 			0, new_obj, DIDL_HW_IPTV_CONTENT_ID);
	s_PvrInfo_Add_PvrRes(old_json, "user_id", 				0, new_obj, DIDL_HW_IPTV_USER_ID);
	s_PvrInfo_Add_PvrRes(old_json, "content_name", 			0, new_obj, DIDL_HW_PROGRAM_TITLE);
	s_PvrInfo_Add_PvrRes(old_json, "parentalRating", 		0, new_obj, DIDL_HW_PROGRAM_RATE);
	s_PvrInfo_Add_PvrRes(old_json, "program_description",	0, new_obj, DIDL_HW_PROGRAM_DESCRIPTION);
	
	/* localEncrypted */
	x = (p->key)? 1: 0;
	s_PvrInfo_Add_Int(new_obj, DIDL_HW_LOCAL_ENCRYPTED, x);
	if( x )
	{
		sprintf(temp, "%lld", p->key);
		s_PvrInfo_Add_String(new_obj, DIDL_HW_LOCAL_ENCRYPTION_KEY, temp);
	}

	HT_DBG_FUNC_END(0, json_object_to_json_string(new_obj));
	return new_obj;
}
static int s_VodInfo_To_Std(PvrFInfo *p, struct json_object *old_json, int xtype, C_DMS_CMI *minfo)
{	
	struct json_object *old_obj;
	const char *str;

	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_MANY, (int)minfo, 0);
	xtype = 0;
	
	minfo->duration = p->time_len*1000;
	minfo->size  	= p->byte_len;

	old_obj = json_object_object_get(old_json, "rate");
	if( old_obj )
		minfo->bitrate = json_object_get_int(old_obj)*128;/* x(Kbps)*1024/8 = x*128(bytes/second) */
	
	old_obj = json_object_object_get(old_json, "resolution");
	if(old_obj && (str = json_object_get_string(old_obj)))
	{
		int x = 0, y = 0;
		sscanf(str, "%dx%d", &x, &y);
		minfo->res_width	= (ushort)x;
		minfo->res_height	= (ushort)y;
	}
	
	HT_DBG_FUNC_END(xtype, 0);
	return 0;
}

/*
PVR JSON：

{ "file_id": 1362476673,
 "bitrate": 0,
 "type": 1,
 "task_state": 7,
"last_task_state": 5,
"error_code": 0,
"start_time": 1362476670,
"end_time": 1362477599,
"real_start_time": 1362476673,
"real_end_time": 0,
 "time_stamp": 0,
 "protection_mode": 2,
"priority": 0,
"sync_flag": 1,
"rec_type": 1,
"episode_type": 0,
 "serial_rec_subtask": 0,
"period_rec_subtask": 0,
"auto_delete": 0,
"reserved_duration": 0,
 "channel_num": "5",
"schedule_id": "20130305094429201303051000005",
"channel_name": "cctv1",
 "channel_name_all_lang": "{ \"count\": 2, \"eng\": \"cctv1\", \"hun\": \"\" }",
"prog_id": "undefined",
"prog_title": "abc",
"prog_title_all_lang": "{ \"count\": 2, \"eng\": \"abc\", \"hun\": \"\" }",
"package_id": "", "package_name": "",
"parentalRating": "2",
"channel_id": "",
"serial_sn": "",
"season_id": "",
"serial_prog_sn": "",
 "program_description": "123456789",
 "program_description_all_lang": "{ \"count\": 2, \"eng\": \123456789"\", \"hun\": \"\" }",
"serial_name": "abc_s1e2",
 "serial_name_all_lang": "{ \"count\": 2, \"eng\": \"abc_s1e2\", \"hun\": \"\" }",
"period_type": "",
"period_every_week": "",
 "period_name": "",
"createMode": "DVB",
"fatherScheduleID": "",
"main_task_id": "",
"user_id": "cxw0128" } 

*/
static struct json_object *s_PvrInfo_To_Extension(PvrFInfo *p, struct json_object *old_json, int xtype)
{	
	int x;
	const char *str;
	char temp[64] = "";

	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_MANY, (int)xtype, 0);
	
	struct json_object * new_obj = json_object_new_object();
	if( !new_obj )
		return new_obj;

	x = s_PvrInfo_ContentSource(xtype);
	s_PvrInfo_Add_Int(new_obj, DIDL_HW_CONTENT_SOURCE, x);
	str = s_PvrInfo_ContentType(xtype);
	if( str)
		s_PvrInfo_Add_String(new_obj, DIDL_HW_CONTENT_TYPE, str);
	
	/* channelNO iptvcontentID iptvuserID */
	s_PvrInfo_Add_PvrRes(old_json, "channel_num", 			0, new_obj, DIDL_HW_CHANNEL_NO);
	s_PvrInfo_Add_PvrRes(old_json, "prog_id", 				0, new_obj, DIDL_HW_IPTV_CONTENT_ID);
	s_PvrInfo_Add_PvrRes(old_json, "user_id", 				0, new_obj, DIDL_HW_IPTV_USER_ID);
	s_PvrInfo_Add_PvrRes(old_json, "parentalRating", 		0, new_obj, DIDL_HW_PROGRAM_RATE);
	s_PvrInfo_Add_PvrRes(old_json, "serial_prog_sn", 		0, new_obj, DIDL_HW_SERIAL_PROGRAM_SN);
	s_PvrInfo_Add_PvrRes(old_json, "father_task_name", 		0, new_obj, DIDL_HW_PARENT_TASK_NAME);
	s_PvrInfo_Add_PvrRes(old_json, "period_name", 			0, new_obj, DIDL_HW_PERIOD_TASK_NAME);

//	s_PvrInfo_Add_String(new_obj, DIDL_HW_PROGRAM_TITLE, "{ \"count\": 2, \"eng\": \"CA-TVoD01-20130327122500-60\", \"hun\": \"abc\" }");
	if(!s_PvrInfo_Add_PvrRes(old_json, "prog_title_all_lang",			0, new_obj, DIDL_HW_PROGRAM_TITLE))
		s_PvrInfo_Add_PvrRes(old_json, "prog_title",					0, new_obj, DIDL_HW_PROGRAM_TITLE);
	if(!s_PvrInfo_Add_PvrRes(old_json, "program_description_all_lang",	0, new_obj, DIDL_HW_PROGRAM_DESCRIPTION))
		s_PvrInfo_Add_PvrRes(old_json, "program_description",			0, new_obj, DIDL_HW_PROGRAM_DESCRIPTION);
	if(!s_PvrInfo_Add_PvrRes(old_json, "serial_name_all_lang",			0, new_obj, DIDL_HW_SERIAL_TASK_NAME))
		s_PvrInfo_Add_PvrRes(old_json, "serial_name",					0, new_obj, DIDL_HW_SERIAL_TASK_NAME);
	
	/* localEncrypted */
	x = (p->key)? 1: 0;
	s_PvrInfo_Add_Int(new_obj, DIDL_HW_LOCAL_ENCRYPTED, x);
	if( x )
	{
		sprintf(temp, "%lld", p->key);
		s_PvrInfo_Add_String(new_obj, DIDL_HW_LOCAL_ENCRYPTION_KEY, temp);
	}

	HT_DBG_FUNC_END(0, json_object_to_json_string(new_obj));
	return new_obj;
}

static int s_PvrInfo_To_Std(PvrFInfo *p, struct json_object *old_json, int xtype, C_DMS_CMI *minfo)
{	
	struct json_object *old_obj;
	const char *str;

	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_MANY, (int)minfo, 0);
	xtype = 0;

	minfo->duration	= p->time_len*1000;
	minfo->size  	= p->byte_len;
	
	old_obj = json_object_object_get(old_json, "bitrate");
	if( old_obj )
		minfo->bitrate = json_object_get_int(old_obj)*128;/* x(Kbps)*1024/8 = x*128(bytes/second) */

	old_obj = json_object_object_get(old_json, "resolution");
	if(old_obj && (str = json_object_get_string(old_obj)))
		sscanf(str, "%hdx%hd", &(minfo->res_width), &(minfo->res_height));
	
	HT_DBG_FUNC_END(xtype, 0);
	return 0;
}

static char* s_MustHaveTitle(struct json_object *new_json, char *title)
{
	char *ret = NULL;
	const char *str;
	struct json_object *old_obj;
	
	old_obj = json_object_object_get(new_json, DIDL_HW_SERIAL_TASK_NAME);
	if(old_obj && (str = json_object_get_string(old_obj)))
	{
		if((strlen(str) > 0) && !coo_str_equal(coo_strtrim_ends((char*)str), ""))
			ret = strdup(str);
	}
	else
	{
		old_obj = json_object_object_get(new_json, DIDL_HW_PROGRAM_TITLE);
		if(old_obj && (str = json_object_get_string(old_obj)))
			if((strlen(str) > 0) && !coo_str_equal(coo_strtrim_ends((char*)str), ""))
				ret = strdup(str);
	}

	if(!ret)
	{
		if(title)
			ret = strdup(title);	
		else
			ret = strdup("unknown");	
	}
	return ret;
}



int HySDK_PvrGetInfo_Ex(char *keyword,  char *title, C_DMS_CMI *minfo)
{
	const char *str;
	struct json_object *old_json, *jtype, *obj;
	char pvrInfoAtJson[4096] = "";
	int ret = -1, r;
	PvrFInfo fileInfo;
	
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_MANY, (int)minfo, keyword);
	r = 0;//ind_pvr_finfo( (char*)keyword, &fileInfo, pvrInfoAtJson, sizeof(pvrInfoAtJson));
	HT_DBG_FUNC(r, pvrInfoAtJson);
//	if(r)
//		goto s_EXIT;

	ret--;
	old_json = dlna_json_tokener_parse(pvrInfoAtJson);
	if(!old_json )
		goto s_EXIT;

	ret--;
	jtype = json_object_object_get(old_json, "type");
	/*
	给你的JSON串，VOD和PVR会不一样，但是他们都有type，type的值定义如下：
	0 - IPTV PVR
	1 - DVB PVR
	2 - old VOD/TVOD, by service_type
	3 - VOD DOWNLOAD
	6 - TVOD DOWNLOAD
	*/
	if(jtype)
	{
		int type = json_object_get_int(jtype);
		switch(type)
		{
			case 0:
	        case 1:
				minfo->keyword	= strdup(keyword);
				minfo->extension= s_PvrInfo_To_Extension(&fileInfo, old_json, type);
				minfo->title	= s_MustHaveTitle(minfo->extension, title);
                ret = s_PvrInfo_To_Std(&fileInfo, old_json, type, minfo);
				break;
	            
			case 2:
				/*
				type为2是以前版本下载的文件，2就是vod下载的文件，需要根据service_type的值去判断是TVOD还是VOD
				service_type
				0 － Webtv点播		 vod download
				1 － Webtv直播
				2 － Webtv时移
				3 － Webtv录播	   tvod download
				*/
				obj = json_object_object_get(old_json, "service_type");
				if(obj &&(str = json_object_get_string(obj)))
				{
					type = atoi(str);
					if(type == 0)
						type = 3;
					else if(type == 3)
						type = 6;
					else
						type = -1;
				}				
			case 3:
			case 6:
				if(type > 0)
				{
					minfo->keyword	= strdup(keyword);
					minfo->extension= s_VodInfo_To_Extension(&fileInfo, old_json, type);
					minfo->title 	= s_MustHaveTitle(minfo->extension, title);
					ret = s_VodInfo_To_Std(&fileInfo, old_json, type, minfo);
				}
				break;
				
			default:
				ret--;
				break;
		}
	}

	json_object_put(old_json);	

s_EXIT:	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

int s_Dms_SharePvr_CmpObject(C_DMS_VO*x, C_DMS_VO*y)
{
	int ret = -2;
	C_DMS_CCI *container_x = NULL, *container_y = NULL;
	C_DMS_CMI *item_x = NULL, *item_y = NULL;

	if( !x || !y || !(x->object) || !(y->object))
		return -2;

	if( x->source_type != e_SOURCE_PVR || y->source_type != e_SOURCE_PVR)
		return -3;
	
	if(x->is_item)
		item_x = x->object;
	else
		container_x = x->object;
	
	if(y->is_item)
		item_y = y->object;
	else
		container_y = y->object;

	if(item_x && item_y)
	{
		ret = strcoll(item_x->title, item_y->title);
	}
	else if(item_x && container_y)
	{
		ret = strcoll(item_x->title, container_y->title);
	}
	else if(container_x && item_y)
	{
		ret = strcoll(container_x->title, item_y->title);
	}
	else
	{
		ret = strcoll(container_x->title, container_y->title);
	}

	if(ret > 0)
		ret = 1;
	else if(ret < 0)
		ret = -1;
	else
		ret = 0;
	
	return ret;
}

typedef struct _x_virtual_pvr_file_interface_ {
	PvrFile_t		fd;
	int				flag;
	long long		file_size;
	long long		offset;
}x_VPFI;
int s_PvrInit(int is_ipc)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, is_ipc, 0);
	//mid_record_init();
	HT_DBG_FUNC_END(0, 0);
	return 0;
}
static int s_PvrGetInfo(const char *keyword, C_FILE_INFO *info)
{
	const char *str;
	char pvrInfoAtJson[4096] = "";
	int ret = -1;
	PvrFInfo fileInfo = {0};
	
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, (int)info, keyword);
	if(info)
	{
		ret = 0;//ind_pvr_finfo( (char*)keyword, &fileInfo, pvrInfoAtJson, sizeof(pvrInfoAtJson));
		HT_DBG_FUNC(ret, pvrInfoAtJson);
		HT_DBG_PRINTF(HT_MOD_APP, HT_BIT_KEY, "file_size = %lld\r\n", fileInfo.byte_len);
		if(ret >= 0)
			ret = 0;
		if(info)
			info->file_length = fileInfo.byte_len;
	}
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
static int s_PvrOpen(const char *filename, int mode)
{
	x_VPFI *me = NULL;
	C_FILE_INFO cfi;
	
	int ret = s_PvrGetInfo(filename, &cfi);
	if(!ret)
	{
		mode = 0;
		PvrFile_t fd = NULL;//ind_pvr_fopen((char*)filename);
		if(fd)
		{
			me = COO_OBJECT_NEW(x_VPFI);
			me->fd 			= fd;
			me->file_size	= cfi.file_length;
		}
	}

	return (int)me;
}
static int s_PvrRead(int fileHnd, char *buf, size_t buflen)
{
	x_VPFI *me = (x_VPFI *)fileHnd;
	return 0;//ind_pvr_fread(me->fd, buf, buflen);
}
static int s_PvrReadEx(int fileHnd, char *buf, size_t buflen, int *flag)
{
	x_VPFI *me = (x_VPFI *)fileHnd;
	int ret = 0;//ind_pvr_fread(me->fd, buf, buflen);
	if(!ret && (me->file_size < 0))
	{
		if(flag)
			*flag = 1;
	}

	return ret;
}
static int s_PvrWrite(int fileHnd, char *buf, size_t buflen)
{
	return -1;
}
static int s_PvrSeek(int fileHnd, off_t offset, int origin)
{
	x_VPFI *me = (x_VPFI *)fileHnd;
	return 0;//ind_pvr_fseek(me->fd, offset);
}
static void s_PvrClose(int fileHnd)
{
	x_VPFI *me = (x_VPFI *)fileHnd;
	//ind_pvr_fclose(me->fd);
	free(me);
}
static C_DMS_VFO s_dms_pvr_op = {s_PvrInit, s_PvrGetInfo, s_PvrOpen, s_PvrRead, s_PvrReadEx, s_PvrWrite, s_PvrSeek, s_PvrClose};



/*-----------------------------------------------------------------------------------------*/
/* basic code */
/*-----------------------------------------------------------------------------------------*/
#define false					0
#define true					1
#define PVR_TITLE				"VodContent"
#define PVR_FOLDER_IDENTIFIER	"record/00000001" /*is it solid? */

static char *s_pvr_folder_path = NULL;
static int s_pvr_home_id = -1;
static C_DMS_AS *s_dms_share_pvr_dmsas = NULL;

static const C_DMS_IPI avc_ts_mp_hd_aac_pvr = {
  .id = "AVC_TS_MP_HD_AAC",
  .mime = "video/vnd.dlna.mpeg-tts",
  .label = "HD",
  .class = 3
};

static C_DMS_CCI *s_Dms_SharePvr_MakeInfo_VirtualContainer (char *pvr_id, char *title)
{
	C_DMS_CCI *container = Dms_VirtualObject_CreateContainerSelf(pvr_id, title, &Container_MIME_Type);
	return container;
}

static C_DMS_CMI *s_Dms_SharePvr_MakeInfo_Item (char *pvr_id, char *title, struct stat *st)
{
	C_DMS_CMI *item = NULL;
	
	if(st && st->st_size < 10*1024)
		return NULL;

	Dms_VirtualObject_CreateItem(&item, &avc_ts_mp_hd_aac_pvr, NULL);
	if(item)
		HySDK_PvrGetInfo_Ex(pvr_id, title, item);
	return item;
}

int s_Dms_SharePvr_AddObject(int parent_id, C_DMS_VO *entry)
{
	char *info = NULL;
	
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_MANY, (int)entry, 0);
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
	HT_DBG_FUNC(id, 0);
	Dms_VirtualObject_Free(entry);
	if(info)
		free(info);

	HT_DBG_FUNC_END(id, 0);
	return id;
}

static int s_Dms_SharePvr_AddChild(int parent_id, char *pvr_id, char *title)
{
	char *vc_title;
	C_DMS_VO *parent, *entry;
	struct json_object *json, *obj;
	int ret = -1;

	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, parent_id, pvr_id);
	if(!pvr_id)
		goto s_EXIT;

	ret--;
	C_DMS_CMI * item = s_Dms_SharePvr_MakeInfo_Item(pvr_id, title, NULL);
	if(!item)
		goto s_EXIT;
#if 0
	if((json = item->extension) 
		&& (obj = json_object_object_get(json, "parentTaskName"))
		&& (vc_title = (char*)json_object_get_string(obj))) //series
	{
		int vc_id = Dms_VirtualFileTree_GetChildID(parent_id, e_OBJTYPE_CONTAINER, vc_title, 0);
		HT_DBG_FUNC(parent_id, "parent_id =");
		if(vc_id < 1)
		{
			C_DMS_CCI *vc = s_Dms_SharePvr_MakeInfo_VirtualContainer(vc_title, vc_title);
			entry = Dms_VirtualObject_Create(parent_id, vc, e_OBJTYPE_CONTAINER, e_SOURCE_PVR, NULL, NULL);
			parent_id = s_Dms_SharePvr_AddObject(parent_id, entry);
			HT_DBG_FUNC(vc_id, "vc_id =");
			if(parent_id < 0)
			{
				Dms_VirtualObject_FreeItem(item);
				goto s_EXIT;
			}
		}
		else
			parent_id = vc_id;	
	}
#endif
	entry = Dms_VirtualObject_Create(parent_id, item, e_OBJTYPE_ITEM, e_SOURCE_PVR, NULL, NULL);
	ret = s_Dms_SharePvr_AddObject(parent_id, entry);

s_EXIT:
	HT_DBG_FUNC_END(ret, title);
	return ret;
}

static int s_Dms_SharePvr_SyncAddItem(char *pvr_id)
{
	return s_Dms_SharePvr_AddChild(s_pvr_home_id, pvr_id, NULL);
}

static void s_Dms_SharePvr_SyncRemoveItem(char *pvr_id)
{
	int id = Dms_VirtualFileTree_GetChildID(s_pvr_home_id, e_SOURCE_PVR, pvr_id, 1);
	if(id > 0)
	{
		int parent_id = Dms_VirtualFileTree_GetParentID(id);
		Dms_VirtualFileTree_DeleteChild(parent_id, id);
		Dms_VirtualFileTree_DeleteContainer(s_pvr_home_id, parent_id, true);
	}
}

static int s_Dms_SharePvr_AsyncAdd_Callback(C_DMS_AS *dmsas, int parent_id, char *keyword, int user)
{
	struct dirent **namelist;
	struct stat st;
	int n,i,run = 1;
	char *container = keyword;
	
	if(user)
	{
		s_Dms_SharePvr_AddChild(parent_id, container, NULL);
		return 0;
	}

//	sleep(3);
	n = scandir (container, &namelist, 0, alphasort);
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_MANY, n, container);
	if (n < 0)
	{
		perror ("scandir");
		return -2;
	}

	for (i = 0; i < n; i++)
	{
		if(Dms_AsyncShare_Is_Aborted(dmsas))
			run = 0;

		if( run )
		{
			if (namelist[i]->d_name[0] == '.')
			{
				free (namelist[i]);
				continue;
			}

			char *fullpath = (char *)malloc (strlen (container) + strlen (namelist[i]->d_name) + 2);
			sprintf (fullpath, "%s/%s", container, namelist[i]->d_name);

			if (stat (fullpath, &st) < 0)
			{
				free (fullpath);
				free (namelist[i]);
				continue;
			}

			HT_DBG_FUNC(S_ISDIR(st.st_mode), namelist[i]->d_name);
//			if(S_ISDIR(st.st_mode))
			{
				s_Dms_SharePvr_AddChild(parent_id, fullpath, namelist[i]->d_name);
			}

			free (fullpath);
		}
		free (namelist[i]);
	}		
	
	free (namelist);
	HT_DBG_FUNC_END(run, 0);
	return run;
}

static int s_Dms_SharePvr_AsyncAddFolder(char *pvr_id)
{
	return Dms_AsyncShare_AddObject(s_dms_share_pvr_dmsas, s_pvr_home_id, pvr_id, 0);
}

static int s_Dms_SharePvr_AsyncRemoveFolder(char *pvr_id)
{
	return Dms_AsyncShare_RemoveObject(s_dms_share_pvr_dmsas, s_pvr_home_id, pvr_id);
}

static int s_Dms_SharePvr_AsyncAddItem(char *pvr_id)
{
	return Dms_AsyncShare_AddObject(s_dms_share_pvr_dmsas, s_pvr_home_id, pvr_id, 1);
}

static void s_Dms_SharePvr_Callback(unsigned int id, int event)
{
	char itemName[1024] = "";
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, event, s_pvr_folder_path);

	if(s_pvr_folder_path)
	{
		sprintf(itemName, "%s/%08x", s_pvr_folder_path, id);
		if( event == PVR_EVENT_ADD)
			s_Dms_SharePvr_SyncAddItem(itemName);
		else if( event == PVR_EVENT_DELETE )
			s_Dms_SharePvr_SyncRemoveItem(itemName);
	}

	HT_DBG_FUNC_END(event, itemName);
}

static int s_Dms_SharePvr_FindHDD(char **fullpath)
{
	char path[1024], fs[128];
	int totalSize, freeSize, ret, type;

	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, NULL);

	int i = 0;//_Device_GetCount();
	HT_DBG_FUNC(i, "0;//_Device_GetCount = ");
	while(i--)
	{
		ret = 0;//_Device_GetInfo(i, &type, path, sizeof(path), &totalSize, &freeSize);
		HT_DBG_FUNC(ret, path);
		if(ret)
			continue;
		if(type) //0: internal HDD; 1:external HDD
			continue;

		int j = 0;//_Partition_GetCount(i);
		HT_DBG_FUNC(j, "HApi_LPlayer_Partition_GetCount = ");
		while(j--)
		{
			ret = 0;//_Partition_GetInfo(i, j,  path, sizeof(path), &totalSize, &freeSize, fs, sizeof(fs));
			HT_DBG_FUNC(ret, path);
			if( ret )
				continue;

			struct stat st;
			char temp[1024];
			coo_str_rid_tail(path, '/');
			sprintf(temp, "%s/%s", path, PVR_FOLDER_IDENTIFIER);
			if(!stat(temp, &st) && !access(temp, R_OK))
			{
				*fullpath = strdup(temp);
				return 0;
			}
		}
	}

	HT_DBG_FUNC_END(-1, NULL);
	return -1;
}


void Dms_SharePvr_MakeHome(void)
{
	char *fullpath = NULL;
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, 0, NULL);
	
	C_DMS_CCI *vc = s_Dms_SharePvr_MakeInfo_VirtualContainer(PVR_TITLE, PVR_TITLE);
	C_DMS_VO *entry = Dms_VirtualObject_Create(0, vc, e_OBJTYPE_CONTAINER, e_SOURCE_PVR, NULL, NULL);
	s_pvr_home_id = s_Dms_SharePvr_AddObject(0, entry);
	
	if(s_pvr_folder_path)
		free(s_pvr_folder_path);
	s_pvr_folder_path = NULL;
	s_Dms_SharePvr_FindHDD(&fullpath);
	if(fullpath)
	{
		s_Dms_SharePvr_AsyncAddFolder(fullpath);
		s_pvr_folder_path = fullpath;
	}
	
	HT_DBG_FUNC_END(s_pvr_home_id, s_pvr_folder_path);
}
void Dms_SharePvr_RemoveHome (void)
{
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, s_pvr_home_id, NULL);
	int id = s_pvr_home_id;
	s_pvr_home_id = -1;
	Dms_VirtualFileTree_DeleteChild(0, id);
	HT_DBG_FUNC_END(s_pvr_home_id, NULL);
}

void Dms_SharePvr_Init (void)
{
	s_dms_share_pvr_dmsas = Dms_AsyncShare_Init(s_Dms_SharePvr_AsyncAdd_Callback);
	
	Dms_SharePvr_MakeHome();
	//ind_pvr_event_regist(s_Dms_SharePvr_Callback);
}

void Dms_SharePvr_Preset(void)
{
	Dms_PresetPvrFileOperation(&s_dms_pvr_op);
	Dms_PresetPvrObjectCmp(s_Dms_SharePvr_CmpObject);
}

#endif
#endif


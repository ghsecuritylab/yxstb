
#include "libzebra.h"

#include "Assertions.h"
#include "dvbs_port.h"
#include "json/json_public.h"
#ifdef INCLUDE_DVBS

#include "codec.h"


int DTV_Channel_GetCount(void);
int DTV_Channel_GetKeyByIndex(int index );
extern int DTV_dvbs_init(void);

extern int dvb_player_pvr_demux_init(void);
extern int dvb_player_pvr_demux_play(int tunerid, int lcn, DMX_NOTIFY_FUNC fun, int *index);
extern int dvb_player_pvr_demux_stop(int index);

extern int dvb_player_pvr_demux_lock(int tunerIndex, int chanKey);
extern int dvb_player_pvr_demux_play1(int tunerid, int lcn,DMX_NOTIFY_FUNC fun, int *index, DTV_CHANNEL *chanInfo);

extern int YX_dvbs_get_param(YX_DVBS_PARAM *dvbs_sezttings);
extern int YX_dvbs_set_param(const YX_DVBS_PARAM *dvbs_settings);
extern int YX_dvbs_tuner_lock(void);
extern int YX_dvbs_search_channel(void );
extern int YX_dvbs_stop_search(void);

#define DVBS_NUM	4

typedef struct _dvb_demux_param {
int tunerid;
int lcn;
int freq_same; //0:freq same as the before channel, 1:diff
int parseband;
int freq;
int pmt_pid;
int program_number;
}dvb_demux_parameter;

#define DVBSParam dvb_demux_parameter

typedef struct __DVBSDemux {
	int demux;
	int tuner;
	int lcn;
	unsigned int magic;
	dvb_writepmt_f writepmt;
	dvb_writedata_f writedata;
	int				pmtsync;//pmt信息是否同步
} DVBSDemux;

static DVBSDemux g_dvbArray[DVBS_NUM];
static mid_mutex_t g_mutex = NULL;
static mid_mutex_t g_omutex = NULL;

void dvbs_port_init(void)
{
	int i;

	PRINTF("========\n");

	if (g_mutex)
		return;
	g_mutex = mid_mutex_create( );
	g_omutex = mid_mutex_create( );
	for (i = 0; i < DVBS_NUM; i ++)
		g_dvbArray[i].lcn = -1;

	DTV_dvbs_init( );
	dvb_player_pvr_demux_init( );

}

static void dvbs_port_writedata(int filter, char *buf, int len)
{
	int i;
	u_int magic;
	DVBSDemux *dvb;
	DVBSParam *param;
	dvb_writedata_f writedata;
	u_char *ubuf = (u_char *)buf;

	param = (DVBSParam *)filter;
	mid_mutex_lock(g_mutex);
	for (i = 0; i < DVBS_NUM; i ++) {
		dvb = &g_dvbArray[i];
		if (param->lcn == dvb->lcn && param->tunerid == dvb->tuner) {
			magic = dvb->magic;
			if (dvb->pmtsync == 0) {
				DvbCaParam caparam;
				dvb_writepmt_f writepmt = dvb->writepmt;
				PRINTF("#%d samefreq = %d, pmtpid = %d, prognum = %d\n", i, param->freq_same, param->pmt_pid, param->program_number);

				memset(&caparam, 0, sizeof(caparam));
				caparam.samefreq	=	(unsigned int)param->freq_same;
				caparam.prognum		=	(unsigned int)param->program_number;
				caparam.pmtpid		=	(unsigned int)param->pmt_pid;

				mid_mutex_unlock(g_mutex);
				{
					int idx;
					DTV_CHANNEL channelInfo; 
					memset(&channelInfo, 0, sizeof(channelInfo));

					idx = DTV_Channel_GetIndexByKey(dvb->lcn);
					if(idx >= 0) {
						DTV_Channel_GetInfo(idx, &channelInfo);
						caparam.networkid = channelInfo.original_network_id;
						DBG_PRN("#%d idx = %d, networkid = %d\n", i, idx, caparam.networkid);
					} else {
						ERR_PRN("#%d DTV_Channel_GetIndexByKey = %d\n", i, idx);
					}
				}
				writepmt(magic, &caparam);
				mid_mutex_lock(g_mutex);
				dvb->pmtsync = 1;
			}

			writedata = dvb->writedata;
			mid_mutex_unlock(g_mutex);
			writedata(magic, ubuf, len);
			mid_mutex_lock(g_mutex);
		}
	}
	mid_mutex_unlock(g_mutex);
}

int dvbs_port_play(int pIndex, int tuner, char* url, dvb_writepmt_f writepmt, dvb_writedata_f writedata, unsigned int magic)
{
	int result = -1;
	int i, lcn, demux = 0;
	DVBSDemux *dvb;
	struct json_object* object = NULL;
	struct json_object* obj = NULL;
	char sValue[32] = {0};
	
	PRINTF("========: index = %d, tuner = %d\n", pIndex, tuner);

	DTV_CHANNEL dvbInfo;
	memset(&dvbInfo, 0, sizeof(dvbInfo));
	
	object = json_tokener_parse_string(url);
	if(object == NULL){
		ERR_PRN( "invalid json: %s\n", url );
		return -1;
	}
	obj = json_object_get_object_bykey(object,"ChannelKey");
	if(obj==NULL){
		ERR_PRN("string is NULL\n");
		json_object_delete(object);
		return -1;
	} else {
		IND_MEMSET(sValue, 0, sizeof(sValue));
		IND_STRCPY(sValue, json_object_get_string(obj));
		dvbInfo.channelKey = atoi(sValue);
		lcn = dvbInfo.channelKey;
	}
	obj = json_object_get_object_bykey(object,"ProgramNum");
	if(obj==NULL){
		ERR_PRN("string is NULL\n");
		json_object_delete(object);
		return -1;
	} else {
		IND_MEMSET(sValue, 0, sizeof(sValue));
		IND_STRCPY(sValue, json_object_get_string(obj));
		dvbInfo.programmer_number  = atoi(sValue);
	}
	obj = json_object_get_object_bykey(object,"ServiceID");
	if(obj==NULL){
		ERR_PRN("string is NULL\n");
		json_object_delete(object);
		return -1;
	} else {
		IND_MEMSET(sValue, 0, sizeof(sValue));
		IND_STRCPY(sValue, json_object_get_string(obj));
		dvbInfo.service_id = atoi(sValue);
	}
	obj = json_object_get_object_bykey(object,"PMTPID");
	if(obj==NULL){
		ERR_PRN("string is NULL\n");
		json_object_delete(object);
		return -1;
	} else {
		IND_MEMSET(sValue, 0, sizeof(sValue));
		IND_STRCPY(sValue, json_object_get_string(obj));
		dvbInfo.PMTPID = atoi(sValue);
	}
	obj = json_object_get_object_bykey(object,"Freq10KHZ");
	if(obj==NULL){
		ERR_PRN("string is NULL\n");
		json_object_delete(object);
		return -1;
	} else {
		IND_MEMSET(sValue, 0, sizeof(sValue));
		IND_STRCPY(sValue, json_object_get_string(obj));
		dvbInfo.frequency_10KHz = atoi(sValue);
	}

	PRINTF("########=== %d, %d, %d, %d, %d", dvbInfo.channelKey, dvbInfo.programmer_number, dvbInfo.service_id, dvbInfo.PMTPID, dvbInfo.frequency_10KHz);
	json_object_delete(object);

	if (pIndex < 0 || pIndex >= DVBS_NUM || lcn < 0) {
		ERR_PRN("index = %d, lcn = %d\n", pIndex, lcn);
		return -1;
	}
	if (g_mutex == NULL) {
		ERR_PRN("g_mutex is NULL\n");
		return -1;
	}

	mid_mutex_lock(g_omutex);
	mid_mutex_lock(g_mutex);

	dvb = &g_dvbArray[pIndex];

	if (dvb->lcn != -1) {
		for (i = 0; i < DVBS_NUM; i ++) {
			if (i == pIndex)
				continue;
			if (dvb->lcn == g_dvbArray[i].lcn && dvb->tuner == g_dvbArray[i].tuner)
				break;
		}
		if (i >= DVBS_NUM) {
			demux = dvb->demux;
			mid_mutex_unlock(g_mutex);
			dvb_player_pvr_demux_stop(demux);
			mid_mutex_lock(g_mutex);
			PRINTF("close tuner = %d lcn = %d, demux = %d\n", dvb->tuner, dvb->lcn, dvb->demux);
		}
		dvb->lcn = -1;
	}
	dvb->pmtsync = 0;

	for (i = 0; i < DVBS_NUM; i ++) {
		if (lcn == g_dvbArray[i].lcn && tuner == g_dvbArray[i].tuner) {
			dvb->lcn = lcn;
			dvb->tuner = tuner;
			dvb->demux = g_dvbArray[i].demux;
			break;
		}
	}

	dvb->magic = magic;
	dvb->writepmt = writepmt;
	dvb->writedata = writedata;

	if (dvb->lcn == -1) {
		dvb->lcn = lcn;
		dvb->tuner = tuner;
		mid_mutex_unlock(g_mutex);
		if (1) {
			i = dvb_player_pvr_demux_lock(tuner, lcn);
			if (i != 1) {
				ERR_PRN("dvb_player_pvr_demux_lock\n");
			} else {
				i = dvb_player_pvr_demux_play1(tuner, lcn, dvbs_port_writedata, &demux, &dvbInfo);
				if (i)
					ERR_PRN("dvb_player_pvr_demux_play1\n");
			}
		} else {
			i = dvb_player_pvr_demux_play(tuner, lcn, dvbs_port_writedata, &demux);
		}
		mid_mutex_lock(g_mutex);
		if (i) {
			dvb->lcn = -1;
			ERR_OUT("dvb_player_pvr_demux_start\n");
		}
		PRINTF("open tuner = %d lcn = %d, demux = %d\n", tuner, lcn, demux);

		dvb->demux = demux;
	}

	result = 0;
Err:
	mid_mutex_unlock(g_mutex);
	mid_mutex_unlock(g_omutex);
	return result;
}

void dvbs_port_stop(int pIndex)
{
	int i;
	DVBSDemux *dvb;

	PRINTF("========: index = %d\n", pIndex);

	if (pIndex < 0 || pIndex >= DVBS_NUM) {
		ERR_PRN("index = %d\n", pIndex);
		return;
	}
	if (g_mutex == NULL) {
		ERR_PRN("g_mutex is NULL\n");
		return;
	}

	mid_mutex_lock(g_omutex);
	mid_mutex_lock(g_mutex);

	dvb = &g_dvbArray[pIndex];

	if (dvb->lcn != -1) {
		for (i = 0; i < DVBS_NUM; i ++) {
			if (i == pIndex)
				continue;
			if (dvb->lcn == g_dvbArray[i].lcn && dvb->tuner == g_dvbArray[i].tuner)
				break;
		}
		if (i >= DVBS_NUM) {
			int demux = dvb->demux;
			mid_mutex_unlock(g_mutex);
			dvb_player_pvr_demux_stop(demux);
			mid_mutex_lock(g_mutex);
			PRINTF("close tuner = %d lcn = %d, demux = %d\n", dvb->tuner, dvb->lcn, dvb->demux);
		}
		dvb->lcn = -1;
	}

	mid_mutex_unlock(g_mutex);
	mid_mutex_unlock(g_omutex);
}

void dvbs_port_change(int pIndex, int type, int *del_pid, int del_num, int *add_pid, int add_num)
{
}


static void dvbs_port_record_init(void)
{
}

int dvbs_port_record_open(int pIndex)
{
	return -1;
}

void dvbs_port_record_push(int pIndex, char* buf, int len)
{
}

void dvbs_port_record_close(int pIndex)
{
}

int dvbs_port_record_check(int pIndex)
{
	return 1;
}

#else
void dvbs_port_init(void)
{
	return;
}

int dvbs_port_play(int pIndex, int tuner, char* url, dvb_writepmt_f writepmt, dvb_writedata_f writedata, unsigned int magic)
{
	return 0;
}

void dvbs_port_stop(int pIndex)
{
	return;
}

void dvbs_port_change(int pIndex, int type, int *del_pid, int del_num, int *add_pid, int add_num)
{
	return;
}

int dvbs_port_record_open(int pIndex)
{
	return -1;
}

void dvbs_port_record_push(int pIndex, char* buf, int len)
{
}

void dvbs_port_record_close(int pIndex)
{
}

int dvbs_port_record_check(int pIndex)
{
	return 1;
}

#endif//ENABLE_DVBS

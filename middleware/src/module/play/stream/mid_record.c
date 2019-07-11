
#ifdef INCLUDE_PVR

#include "stream.h"

void mid_record_init(void)
{
	static int inited = 0;
	PvrOperation op;

	if (inited)
		return;
	inited = 1;

	record_port_init( );

	op.mutex_create = (mutex_create_f)mid_mutex_create;
	op.mutex_lock = (mutex_lock_f)mid_mutex_lock0;
	op.mutex_unlock = (mutex_unlock_f)mid_mutex_unlock;

	op.disk_size = record_port_disk_size;
	op.cfg_param = record_port_cfg_param;

	op.dir_create = record_port_dir_create;
	op.dir_delete = record_port_dir_delete;
	op.dir_open = record_port_dir_open;
	op.dir_read = record_port_dir_read;
	op.dir_close = record_port_dir_close;

	op.file_size = record_port_file_size;
	op.file_truncate = record_port_file_truncate;

	op.file_open = record_port_file_open;
	op.file_seek = record_port_file_seek;
	op.file_write = record_port_file_write;
	op.file_sync = record_port_file_sync;
	op.file_read = record_port_file_read;
	op.file_close = record_port_file_close;
	op.file_delete = record_port_file_delete;

	op.strm_key = record_port_strm_key;
	op.strm_write = record_port_strm_write;
	op.strm_read = record_port_strm_read;
	op.strm_error = record_port_strm_error;

	ind_pvr_init(&op);
}

#ifndef ENABLE_WASTE
void mid_record_mount(void)
{
	uint32_t clk = mid_10ms( );
	LOG_STRM_PRINTF("++++++++ clk = %08x\n", clk);
	ind_pvr_mount(clk);
}

void mid_record_unmount(void)
{
	uint32_t clk = mid_10ms( );
	LOG_STRM_PRINTF("-------- clk = %08x\n", clk);
	ind_pvr_unmount(clk);
}
#endif

void mid_record_virtual(int flag)
{
	LOG_STRM_PRINTF("virtual = %d\n", flag);
	ind_pvr_virtual(flag);
}

unsigned int mid_record_get_num(void)
{
	unsigned int num;

	num = ind_pvr_get_num( );

	return num;
}

int mid_record_get_info(int idx, struct PVRInfo *info, char *info_buf, int* pinfo_len)
{
	int ret = -1;

	ret = ind_pvr_get_info_ex(idx, info, info_buf, pinfo_len);
	if (ret)
		LOG_STRM_ERROUT("#%d ind_pvr_get_info\n", idx);

	LOG_STRM_DEBUG("id = %08x recording = %d, time_len = %d / %d, byte_len = %lld / %lld\n", info->id, info->recording, info->time_len, info->time_length, info->byte_len, info->byte_length);
	if (info && info->recording == 2 && info->time_len < info->time_length && info->time_len + 5 >= info->time_length - info->time_length / 100)
		info->time_len = info->time_length;

	ret = 0;
Err:
	return ret;
}

int mid_record_user_write(uint32_t id, char *buf, int len)
{
	int ret = -1;

	ret = ind_pvr_user_write(id, buf, len);
	if (ret)
		LOG_STRM_ERROUT("id = %08x ind_pvr_user_write len = %d\n", id, len);

	LOG_STRM_PRINTF("id = %08x len = %d\n", id, len);

Err:
	return ret;
}

int mid_record_user_read(uint32_t id, char *buf, int size)
{
	int ret = -1;

	ret = ind_pvr_user_read(id, buf, size);
	if (ret < 0)
		LOG_STRM_ERROUT("id = %08x ind_pvr_user_read\n", id);

	LOG_STRM_PRINTF("id = %08x ret = %d\n", id, ret);

Err:
	return ret;
}

void mid_record_delete(uint32_t id, PvrMsgCall call)
{
	LOG_STRM_PRINTF("id = 0x%x call = %p\n", id, call);

	int_record_delete(id);

	if (call)
		strm_async_cmd(id, ASYNC_CMD_DELETE_PVR, call);
	else
		ind_pvr_delete(id, NULL);
}

void mid_record_suffix(char* sinfo, char* smedia)
{
}

void mid_record_check_bandwidth(int idx)
{
	int apptype = int_record_type(idx);

	LOG_STRM_PRINTF("#%d apptype = %d\n", idx, apptype);
	if (apptype != APP_TYPE_HTTP)
		return;
	int_record_message(idx, STREAM_CMD_RECORD_CHECK_BANDWIDTH, 0, 0, 0, 0);
}

void mid_record_limit_bandwidth(int idx, int bitrate)
{
	int apptype = int_record_type(idx);

	LOG_STRM_PRINTF("#%d apptype = %d, bitrate = %d\n", idx, apptype, bitrate);
	if (apptype != APP_TYPE_HTTP)
		return;
	int_record_message(idx, STREAM_CMD_RECORD_LIMIT_BANDWIDTH, bitrate, 0, 0, 0);
}

void mid_record_adjust_bandwidth(int idx, int percent, int second)
{
	int apptype = int_record_type(idx);

	LOG_STRM_PRINTF("#%d apptype = %d, percent = %d, second = %d\n", idx, apptype, percent, second);
	if (apptype != APP_TYPE_HTTP)
		return;
	int_record_message(idx, STREAM_CMD_RECORD_ADJUST_BANDWIDTH, percent, second, 0, 0);
}

void mid_record_set_endtime(int idx, uint32_t id, uint32_t endtime)
{
	LOG_STRM_PRINTF("#%d id = %08x, endtime = %u / %u\n", idx, id, endtime, mid_time( ));
	int_record_message(idx, STREAM_CMD_RECORD_END, (int)id, (int)endtime, 0, 0);
}

#endif//INCLUDE_PVR

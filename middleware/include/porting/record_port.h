
#ifndef __RECORD_PORT_H__
#define __RECORD_PORT_H__

#include "ind_ts.h"
#include "mid_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

#define		CHANNEL_NUM_MAX			999
#define		CHANNEL_NUM_MIN			0
typedef void (*record_msg_handle)(int pIndex, unsigned int id, STRM_MSG message, int arg);

void record_port_msg_hdl_set(record_msg_handle msg_hdl);

void local_timeshift_port_hdl_set(void);
void record_port_filename(int id, char *name);
void record_port_init(void);

void record_port_message(int pIndex, unsigned int id, STRM_MSG msg, int arg);

/*
	rate k字节/秒
 */
void record_port_byterate(int pIndex, unsigned int id, int rate);

int record_port_disk_size(unsigned int* psize, unsigned int* pspace);
void record_port_set_param(char *rootpath);
int record_port_equal_param(char *rootpath);
void record_port_cfg_param(char *path, int rootsize, int *pvrnum);
int record_port_dir_create(char *path);
void record_port_dir_delete(char *path);
void* record_port_dir_open(char *path);
int record_port_dir_read(void* dir, char* name);
void record_port_dir_close(void* dir);

int record_port_file_size(char* path, int* filesize);
int record_port_file_truncate(char* path, int filesize);

#define record_port_file_open	record_port_file_open_v2
int record_port_file_open(char *name, int flags);
int record_port_file_seek(int fd, int offset);
int record_port_file_write(int fd, char *buf, int len);
#define record_port_file_sync record_port_file_sync_v1
int record_port_file_sync(int fd);
int record_port_file_read(int fd, char *buf, int len);
void record_port_file_close(int fd);
void record_port_file_delete(char *path);

void record_port_msg_hdl_delete(record_msg_handle msg_hdl);
/*
	设置解密key
 */
void record_port_strm_setkey(long long key);
/*
	解密
 */
void record_port_strm_decrypt(char *buf, int len);

int record_port_strm_key(int64_t* len);
int record_port_strm_write(int fd, char *buf, int len);
int record_port_strm_read(int fd, char *buf, int len);
void record_port_strm_error(unsigned int id);

int vd_descrambler_set_current_apid(int pid);

typedef void (*record_write_f)(unsigned int magic, char *buf, int len);
int record_port_encrypt_set(int encrypt);

#ifdef __cplusplus
}
#endif

#endif

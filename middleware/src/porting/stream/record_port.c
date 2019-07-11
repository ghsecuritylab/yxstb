#include <dirent.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include "app_include.h"
#include "config.h"
#include "record_port.h"
#include "mid_stream.h"
#include "sys_msg.h"
#include "openssl/md5.h"
#include "config/pathConfig.h"

#include <errno.h>

#if defined(INCLUDE_PVR)

//#define ENABLE_3DES

extern int ymm_stream_createEncryptKeyHandle(char *keys, int keylen);
extern int ymm_stream_createDecryptKeyHandle(char *keys, int keylen);
extern int ymm_stream_cryptStream(int crypthandle,char *buf, int len,char *output);
extern int yhw_mem_alloc(int size, int alignment,void **addr);
extern int yhw_mem_free(void *addr);
static char g_pvrkey[16];
static int g_encHandle = 0;
static int g_decHandle = 0;
static char* g_strbuf = NULL;
static mid_mutex_t g_mutex = NULL;

#define STRM_BUFFER_SIZE		(1316*8)

static char g_rootpath[256] = DEFAULT_EXTERNAL_DATAPATH"/record";
#define MAX_HANDLE_CNT 5
static record_msg_handle g_record_msg_hdl[MAX_HANDLE_CNT] = {NULL};
void record_port_msg_hdl_set(record_msg_handle msg_hdl)
{
	record_msg_handle handle;
	int i = 0;

	PRINTF("record_port_msg_hdl_set\n");
	mid_mutex_lock(g_mutex);
	for( i = 0; i < MAX_HANDLE_CNT; i ++){
		if(g_record_msg_hdl[i] && g_record_msg_hdl[i] == msg_hdl){
			PRINTF("already added\n");
			goto end;
		}
	}
	for( i = 0; i < MAX_HANDLE_CNT; i ++){
		if(g_record_msg_hdl[i] == NULL){
			g_record_msg_hdl[i] = msg_hdl;
			goto end;
		}
	}
	PRINTF("error to set record msg handler, full\n");
end:
		mid_mutex_unlock(g_mutex);
		return;
	}

void record_port_msg_hdl_delete(record_msg_handle msg_hdl)
{
	record_msg_handle handle;
	int i = 0;

	mid_mutex_lock(g_mutex);
	for( i = 0; i < MAX_HANDLE_CNT; i ++){
		if(g_record_msg_hdl[i] && g_record_msg_hdl[i] == msg_hdl){
			g_record_msg_hdl[i] = NULL;
			goto end;
		}
	}
end:
	mid_mutex_unlock(g_mutex);
	return;
}

//老库运行依赖函数
void record_port_filename(int id, char *name)
{
}
// lh 2010-3-20 同步刘建华库。依赖此函数
void record_port_byterate(int pIndex, uint32_t id, int rate)
{

   return;
}

static int app_md5_pvrkey(long long key, char *pvrkey, int size)
{
	unsigned char mac[6], buf[18];
	int i;
	unsigned char c;
	MD5_CTX ctx;

	if (pvrkey == NULL || size < 24)
		ERR_OUT("pvrkey = %p, size = %d\n", pvrkey, size);

	MD5_Init(&ctx);

	for (i = 5; i >= 0; i --) {
		mac[i] = (u_char)key;
		key >>= 8;
	}
	sprintf((char*)buf, "%02x:%02x:%02x:%02x:%02x:%02x", (uint32_t)mac[0], (uint32_t)mac[1], (uint32_t)mac[2], (uint32_t)mac[3], (uint32_t)mac[4], (uint32_t)mac[5]);

	MD5_Update(&ctx, buf, 17);
	MD5_Final(buf, &ctx);

	for(i = 0; i < 12; i ++) {
		c = buf[i] >> 4;
		if (c < 10)
			pvrkey[i * 2] = '0' + c;
		else
			pvrkey[i * 2] = 'A' + (c - 10);

		c = buf[i] & 0xf;
		if (c < 10)
			pvrkey[i * 2 + 1] = '0' + c;
		else
			pvrkey[i * 2 + 1] = 'A' + (c - 10);
	}

	return 24;
Err:
	return -1;
}

void record_port_init(void)
{
	long long key;

#ifdef ENABLE_3DES
	key = 0;
	record_port_strm_key(&key);
	app_md5_pvrkey(key, g_pvrkey, 16);

	g_encHandle = ymm_stream_createEncryptKeyHandle(g_pvrkey,16);
	g_decHandle = ymm_stream_createDecryptKeyHandle(g_pvrkey,16);
#endif
	PRINTF("record encrypt key = %s\n", g_pvrkey);

	g_mutex = mid_mutex_create( );
	if (yhw_mem_alloc(STRM_BUFFER_SIZE, 4, (void **)&g_strbuf)) {
		ERR_PRN("yhw_mem_alloc\n");
		g_strbuf = NULL;
	}
}

static int record_port_msg(int pIndex, unsigned int pvr_id, STRM_MSG msg)
{
	//一般录制，如pvr和下载等
	switch(msg){
	    case RECORD_MSG_DISK_ERROR:
			//PRINTF("RECORD_MSG_ERROR(%d) --> RECORD_MSG_DISK_ERROR(0x%x)\n",RECORD_MSG_DISK_ERROR,PVR_MSG_DISK_NOT_FOUNDED);
			//browser_msg_send(PVR_MSG_DISK_NOT_FOUNDED, (unsigned int)pvr_id, 0);
			break;
		case RECORD_MSG_ERROR:
			//PRINTF("RECORD_MSG_ERROR(%d) --> PVR_MSG_ERROR(0x%x)\n",RECORD_MSG_ERROR,PVR_MSG_ERROR);
			//browser_msg_send(PVR_MSG_ERROR, (unsigned int)pvr_id, 0);
			break;
		case RECORD_MSG_DISK_FULL:
			//browser_msg_send(PVR_MSG_DISK_FULL, (unsigned int)pvr_id, 0);
			break;
		case RECORD_MSG_SUCCESS_BEGIN:
			//browser_msg_send(PVR_MSG_SUCCESS_BEGIN, (unsigned int)pvr_id, 0);

			break;
		case RECORD_MSG_REFUSED_SERVER:
			//browser_msg_send(DOWNLOAD_SERVER_BUSSY, (unsigned int)pvr_id, 0 );
			break;
		case RECORD_MSG_NOT_FOUND:
			//browser_msg_send(DOWNLOAD_MEDIA_404, (unsigned int)pvr_id, 0 );
			break;
		case RECORD_MSG_DATA_DAMAGE:
			//browser_msg_send(DOWNLOAD_MEDIA_404, (unsigned int)pvr_id, 0 );
			break;
		case RECORD_MSG_DISK_WARN:
		case RECORD_MSG_CLOSE:
			//browser_msg_send(PVR_MSG_CLOSE, (unsigned int)pvr_id, 0 );   //响应PVR close 消息 lh

			break;
		default:
			break;
	}

	return 0;
}

void record_port_message(int pIndex, u_int pvr_id, STRM_MSG msg, int arg)
{
	int i = 0;
	PRINTF("####pvr_id=%d msg = %d\n",pvr_id, msg);

	//record_port_msg(pIndex, pvr_id, msg);
	for(i = 0; i < MAX_HANDLE_CNT; i ++){
		if(g_record_msg_hdl[i])
			g_record_msg_hdl[i](pIndex, pvr_id, (int)msg, arg);
	}
}


/*
struct statfs {
	long    f_type;     // 文件系统类型
	long    f_bsize;    // 经过优化的传输块大小
	long    f_blocks;   // 文件系统数据块总数
	long    f_bfree;    // 可用块数
	long    f_bavail;   // 非超级用户可获取的块数
	long    f_files;    // 文件结点总数
	long    f_ffree;    // 可用文件结点数
	fsid_t  f_fsid;     // 文件系统标识
	long    f_namelen;  // 文件名的最大长度
};
	-1 磁盘不存在或错误
	0 成功
 */
int record_port_disk_size(u_int* ptotalblock, u_int* pfreeblock)
{
     long type;
     struct statfs fs;

     if (statfs("/mnt", &fs))
          ERR_OUT("statfs errno = %s\n", strerror(errno));
     type = fs.f_type;
     if (statfs(DEFAULT_EXTERNAL_DATAPATH, &fs))
          ERR_OUT("statfs errno = %s\n", strerror(errno));
/*
	0x6969	NFS
	0x4d44	MSDOS
 */
     if(type == fs.f_type)
          ERR_OUT("disk not found!\n");

     *ptotalblock = (long long)fs.f_bsize * (long long)fs.f_blocks / 1024 / 1024;
     *pfreeblock = (long long)fs.f_bsize * (long long)fs.f_bfree / 1024 / 1024;
     PRINTF("size = %d, space = %d\n", *ptotalblock, *pfreeblock);

     return 0;
Err:
     return -1;
}

//设置录制根目录
void record_port_set_param(char *rootpath)
{
	IND_STRCPY(g_rootpath, rootpath);
}

int record_port_equal_param(char *rootpath)
{
	if( rootpath && (strcmp(g_rootpath, rootpath) == 0) )
		return 1;
	return 0;
}

void record_port_cfg_param(char *rootpath, int rootsize, int *prognum)
{
	IND_STRCPY(rootpath, g_rootpath);
	*prognum = 1024;
}

int record_port_dir_create(char *path)
{
	if (path == NULL)
		ERR_OUT("path is NULL\n");
	if (mkdir(path, S_IRWXU))
		ERR_OUT("path = %s, err = %s\n", path, strerror(errno));
	sync( );
	PRINTF("path = %s\n", path);

	return 0;
Err:
	return -1;
}


static int remove_file(const char *path)
{
	DIR *dp;
	struct dirent *d;

	if (unlink(path) == 0)
		return 0;

	WARN_PRN("unlink '%s', err = %s\n", path, strerror(errno));

		dp = opendir(path);
		if (dp == NULL)
		ERR_OUT("opendir %s, err = %s\n", path, strerror(errno));

	while ((d = readdir(dp)) != NULL) {
		char subpath[256];

		if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		if (snprintf(subpath, 255, "%s/%s", path, d->d_name) <= 0) {
			ERR_PRN("snprintf %s %s\n", path, d->d_name);
			continue;
		}
		if (remove_file(subpath) < 0) {
			ERR_PRN("remove_file %s\n", subpath);
			continue;
		}
	}

	if (closedir(dp) < 0)
		ERR_OUT("closedir '%s', err = %s\n", path, strerror(errno));

	if (rmdir(path) < 0)
		ERR_OUT("rmdir '%s', err = %s\n", path, strerror(errno));

	return 0;
Err:
	return -1;
}

void record_port_dir_delete(char *path)
{
	DIR *dp;
	struct dirent *d;

	if (path == NULL)
		ERR_OUT("path = %p\n", path);

	dp = opendir(path);
	if (dp == NULL)
		ERR_OUT("opendir %s, err = %s\n", path, strerror(errno));

	while ((d = readdir(dp)) != NULL) {
		char subpath[256];

		if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		if (snprintf(subpath, 255, "%s/%s", path, d->d_name) <= 0) {
			ERR_PRN("snprintf %s %s\n", path, d->d_name);
			continue;
		}
		if (remove_file(subpath) < 0) {
			ERR_PRN("remove_file %s\n", subpath);
			continue;
		}
	}

	if (closedir(dp) < 0)
		ERR_OUT("closedir '%s', err = %s\n", path, strerror(errno));

	if (rmdir(path) < 0)
		ERR_OUT("rmdir '%s', err = %s\n", path, strerror(errno));

	sync( );
	PRINTF("path = %s\n", path);
Err:
	return;
}

void* record_port_dir_open(char *path)
{
	DIR *dir;

	if (path == NULL)
		ERR_OUT("path is NULL\n");
	dir = opendir(path);
	if (dir == NULL)
		ERR_OUT("opendir, path = %s, err = %s\n", path, strerror(errno));
	PRINTF("dir = %p, path = %s\n", dir, path);

	return (void*)dir;
Err:
	return NULL;
}

int record_port_dir_read(void* dir, char* name)
{
	struct dirent *ent;
	if (dir == NULL)
		ERR_OUT("path is NULL\n");

	ent = readdir(dir);
	PRINTF("--------------------------------------------\n");
	if (ent == NULL)
		PRINTF("ent is NULL\n");
	else
		PRINTF("d_type = %d, name = %s\n", ent->d_type, ent->d_name);

	if (ent)
		IND_STRCPY(name, ent->d_name);
	else
		name[0] = 0;

	return 0;
Err:
	return -1;
}

void record_port_dir_close(void* dir)
{
	if (dir == NULL)
		ERR_OUT("path is NULL\n");
	PRINTF("dir = %p\n", dir);
	closedir(dir);
Err:
	return;
}

int record_port_file_open(char *name, int flags)
{
	int fd;

	if (name == NULL)
		ERR_OUT("name = %p, flags = %d\n", name, flags);
	fd = open(name, flags);
	if (fd == -1)
		ERR_OUT("fopen %s, flags = %d, err = %s\n", name, flags, strerror(errno));
	PRINTF("fd = %p, flags = %d, flags = %d\n", fd, name, flags);

	return fd;
Err:
	return -1;
}

int record_port_file_size(char* path, int* psize)
{
	struct stat info;

	if (path == NULL || psize  == NULL)
		ERR_OUT("path = %p, psize = %p\n", path, psize);
	if (stat(path, &info))
		ERR_OUT("stat %s, err = %s\n", path, strerror(errno));

	*psize = (int)info.st_size;

	return 0;
Err:
	return -1;
}

int record_port_file_truncate(char* path, int size)
{
	if (path == NULL || size < 0)
		ERR_OUT("path = %p, size = %d\n", path, size);

	if (truncate(path, (off_t)size))
		ERR_OUT("truncate %s %d, err = %s\n", path, size, strerror(errno));

	return 0;
Err:
	return -1;
}

int record_port_file_seek(int fd, int offset)
{
	if (fd == -1)
		ERR_OUT("fp is NULL\n");
	if (lseek(fd, (off_t)offset, SEEK_SET) == -1)
		ERR_OUT("fseek err = %s\n", strerror(errno));

	return 0;
Err:
	return -1;
}

int record_port_file_write(int fd, char *buf, int len)
{

	if (fd == -1)
		ERR_OUT("fp is NULL\n");

	return write(fd, buf, len);
Err:
	return -1;
}

int record_port_file_sync(int fd)
{
	if (fd >= 0)
		fsync(fd);
	else
		sync( );
	return 0;
}

int record_port_file_read(int fd, char *buf, int len)
{
	if (fd == -1 || len <= 0)
		ERR_OUT("fd = %d, len = %d\n", fd, len);
	return read(fd, buf, len);
Err:
	return -1;
}

void record_port_file_close(int fd)
{
	if (fd < -1)
		ERR_OUT("fp is NULL\n");
	close(fd);
	PRINTF("fd = %d\n", fd);
Err:
	return;
}

void record_port_file_delete(char *path)
{
	if (unlink(path))
		ERR_OUT("unlink = %s\n", path);
	PRINTF("path = %s\n", path);
	sync( );
Err:
	return;
}


int record_port_strm_write(int fd, char *buf, int len)
{
#ifdef ENABLE_3DES
	int l, ret, bytes;

	if (g_strbuf == NULL || fd == -1 || len % 188)
		ERR_OUT("g_strbuf = %p, fd = %d, len = %d\n", g_strbuf, fd, len);

	bytes = 0;
	mid_mutex_lock(g_mutex);
	while(len > 0) {
		l = len;
		if (l > STRM_BUFFER_SIZE)
			l = STRM_BUFFER_SIZE;
		IND_MEMCPY(g_strbuf, buf, l);
		ymm_stream_cryptStream(g_encHandle, g_strbuf, l, g_strbuf);
		ret = write(fd, g_strbuf, l);
		if (ret < l) {
			ERR_PRN("fwrite l = %d, ret = %d, err = %s\n", l, ret, strerror(errno));
			bytes = -1;
		}
		buf += l;
		len -= l;
		bytes += l;
	}
	mid_mutex_unlock(g_mutex);

	return bytes;
Err:
	return -1;
#else
	return record_port_file_write(fd, buf, len);
#endif
}

int record_port_strm_read(int fd, char *buf, int len)
{
#ifdef ENABLE_3DES
	int l, ret;

	if (g_strbuf == NULL || fd == -1 || len <= 0 || len % 188)
		ERR_OUT("g_strbuf = %p, fd = %d, len = %d\n", g_strbuf, fd, len);

	mid_mutex_lock(g_mutex);
	if (len > STRM_BUFFER_SIZE)
		len = STRM_BUFFER_SIZE;

	l = read(fd, g_strbuf, len);
	ret = l;
	if (l < 0) {
		ERR_PRN("fread len = %d, ret = %d, err = %s\n", len, l, strerror(errno));
	} else {
		while (l % 188) {
			ret = read(fd, g_strbuf + l, 188 - (l % 188));
			if (ret <= 0) {
				ERR_PRN("fwrite l = %d, ret = %d, err = %s\n", l, ret, strerror(errno));
				ret = -1;
				break;
			}
			l += ret;
		}
	}
	if (ret > 0) {
		ymm_stream_cryptStream(g_decHandle, g_strbuf, l, g_strbuf);
		IND_MEMCPY(buf, g_strbuf, l);
		ret = l;
	}
	mid_mutex_unlock(g_mutex);

	return ret;
Err:
	return -1;
#else
	return record_port_file_read(fd, buf, len);
#endif
}

//获取加密key
int record_port_strm_key(int64_t* key)
{
	long long k;
#ifdef ENABLE_3DES
	int i;
	unsigned char mac[6];

	mid_net_mac_digit((char*)mac);
	k = 0;
	for (i = 0; i < 6; i ++) {
		k <<= 8;
		k += (long long)((uint32_t)mac[i]);
	}
#else
	k = 2;
#endif
	if (key)
		*key = k;

	PRINTF("key = %lld\n", k);

	return 0;
}

void record_port_strm_setkey(long long key)
{
	PRINTF("key = %lld\n", key);
return;
	ymm_stream_destroyDecryptHandle(g_decHandle);
	if (key == 0)
		record_port_strm_key(&key);
	app_md5_pvrkey(key, g_pvrkey, 16);
	g_decHandle = ymm_stream_createDecryptKeyHandle(g_pvrkey,16);
}

void record_port_strm_decrypt(char *buf, int len)
{
	int l;

	mid_mutex_lock(g_mutex);

	if (g_strbuf == NULL || buf == NULL || len % 188)
		ERR_OUT("g_strbuf = %p, buf = %p, len = %d\n", g_strbuf, buf, len);
#ifdef ENABLE_3DES
	while(len > 0) {
		l = len;
		if (l > STRM_BUFFER_SIZE)
			l = STRM_BUFFER_SIZE;
		IND_MEMCPY(g_strbuf, buf, l);
		ymm_stream_cryptStream(g_decHandle, g_strbuf, l, g_strbuf);
		IND_MEMCPY(buf, g_strbuf, l);
		buf += l;
		len -= l;
	}
#endif
Err:
	mid_mutex_unlock(g_mutex);
}

void record_port_strm_error(u_int id)
{
	PRINTF("id = 0x%08x\n", id);
}

#else

void record_port_init(void)
{
}

void record_port_message(int pIndex, u_int pvr_id, STRM_MSG msg, int arg)
{
}

/*
	rate 千字节/秒
 */
void record_port_byterate(int pIndex, uint32_t id, int rate)
{
}

/*
struct statfs {
	long    f_type;     // 文件系统类型
	long    f_bsize;    // 经过优化的传输块大小
	long    f_blocks;   // 文件系统数据块总数
	long    f_bfree;    // 可用块数
	long    f_bavail;   // 非超级用户可获取的块数
	long    f_files;    // 文件结点总数
	long    f_ffree;    // 可用文件结点数
	fsid_t  f_fsid;     // 文件系统标识
	long    f_namelen;  // 文件名的最大长度
};
	-1 磁盘不存在或错误
	0 成功
 */
int record_port_disk_size(uint32_t* ptotalblock, uint32_t* pfreeblock)
{
	return -1;
}

void record_port_cfg_param(char *rootpath, int rootsize, int *prognum)
{
	strcpy(rootpath, DEFAULT_EXTERNAL_DATAPATH"/record");
	*prognum = 1024;
}

int record_port_dir_create(char *path)
{
	return -1;
}

void record_port_dir_delete(char *path)
{
}

void* record_port_dir_open(char *path)
{
	return NULL;
}

int record_port_dir_read(void* dir, char* name)
{
	return -1;
}

void record_port_dir_close(void* dir)
{
	return;
}

int record_port_file_open(char *name, int flags)
{
	return NULL;
}

int record_port_file_size(char* path, int* psize)
{
	return -1;
}

int record_port_file_truncate(char* path, int size)
{
	return -1;
}

int record_port_file_seek(int fd, int offset)
{
	return -1;
}

int record_port_file_write(int fd, char *buf, int len)
{
	return -1;
}

int record_port_file_sync(int fd)
{
	return 0;
}

int record_port_file_read(int fd, char *buf, int len)
{
	return -1;
}

void record_port_file_close(int fd)
{
	return;
}

void record_port_file_delete(char *path)
{
	return;
}


int record_port_strm_write(int fd, char *buf, int len)
{
	return -1;
}

int record_port_strm_read(int fd, char *buf, int len)
{
	return -1;
}

//获取加密key
int record_port_strm_key(int64_t* key)
{
	return 0;
}

void record_port_strm_error(uint32_t id)
{
	PRINTF("id = 0x%08x\n", id);
}

void record_port_strm_setkey(long long key)
{
	return;
}

void record_port_strm_decrypt(char *buf, int len)
{
	return;
}
#endif



#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/statfs.h>

#include "app/Assertions.h"
#include "ind_string.h"
#include "ind_pvr.h"

#include "int_pvr.h"
#include "ind_mem.h"

struct __PvrFile {
	int			fd;
	uint32_t		id;

	int			strm_num;
	int			strm_index;

	int			base_sn;
	int			base_len;

	int64_t		byte_len;
	int64_t		byte_off;
	int64_t*	byte_array;
	char 		path[PVR_PATH_LEN];
	char 		pathlen;
};
typedef struct __PvrFile PvrFile;

static int int_pvr_finfo(char* path, PvrSInfo* sinfo, char* info_buf, int info_size)
{
	int l, len;

	int fd = -1, result = -1;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		ERR_OUT("open %s err = %d/%s\n", path, errno, strerror(errno));

	l = read(fd, (char*)&len, sizeof(len));
	if (l != sizeof(len))
		ERR_OUT("file_read len ret = %d\n", l);
	if (len != sizeof(PvrSInfo))
		ERR_OUT("len = %d / %d\n", len, sizeof(PvrSInfo));
	l = read(fd, (char*)sinfo, len);
	if (l != len)
		ERR_OUT("read PvrSInfo = %d / %d\n", l, len);

	if (info_buf && info_size > 0) {
		l = read(fd, (char*)&len, sizeof(len));
		if (l != sizeof(len))
			ERR_OUT("file_read len ret = %d\n", l);
		if (len > info_size)
			WARN_PRN("len = %d / %d\n", len, info_size);
		l = read(fd, info_buf, info_size);
		if (l != len)
			ERR_OUT("read PvrSInfo = %d / %d\n", l, len);

		result = len;
	} else {
		result = 0;
	}

Err:
	if (fd >= 0)
		close(fd);
	return result;
}

int ind_pvr_finfo(char* path, PvrFInfo_t info, char* info_buf, int info_size)
{
	PvrSInfo sinfo;
	int len = 0;

	if (info == NULL)
		ERR_OUT("info is NULL %s\n", path);

	len = int_pvr_finfo(path, &sinfo, info_buf, info_size);
	if (len < 0)
		ERR_OUT("int_pvr_finfo %s\n", path);

	if (sinfo.encrypt)
		info->key = sinfo.key;
	else
		info->key = 0;
	if (sinfo.byte_len <= sinfo.byte_base)
		info->byte_len = 0;
	else
		info->byte_len = sinfo.byte_len - sinfo.byte_base;
	if (sinfo.time_len <= sinfo.time_base)
		info->time_len = 0;
	else
		info->time_len = sinfo.time_len - sinfo.time_base;

	return len;
Err:
	return -1;
}

static void int_pvr_fclose(PvrFile_t pf)
{
	if (pf) {
		if (pf->fd != -1) {
			close(pf->fd);
			pf->fd = -1;
		}
		if (pf->byte_array) {
			IND_FREE(pf->byte_array);
			pf->byte_array = NULL;
		}
		IND_FREE(pf);
	}
}

PvrFile_t ind_pvr_fopen(char* path)
{
	int i, sn, len, size, fragment_time;
	PvrSInfo sinfo;
	PvrFile_t pf = NULL;

	if (int_pvr_finfo(path, &sinfo, NULL, 0) < 0)
		ERR_OUT("int_pvr_finfo %s\n", path);

	if (sinfo.version < 11)
		fragment_time = FRAGMENT_TIME_DEFAULT;
	else
		fragment_time = FRAGMENT_TIME_CURRENT;

	pf = (PvrFile_t)IND_CALLOC(sizeof(PvrFile), 1);
	if (pf == NULL)
		ERR_OUT("malloc PvrFile\n");

	len = strlen(path);
	if (len <= 18)
		ERR_OUT("len = %d %s\n", len, path);
	len -= 18;
	if ('/' != path[len])
		ERR_OUT("invalid %s\n", path);
	IND_MEMCPY(pf->path, path, len);

	sprintf(pf->path + len, "/%08x", sinfo.base_id);
	pf->pathlen = strlen(pf->path);

	if (sinfo.time_len <= sinfo.time_base)
		pf->strm_num = 1;
	else
		pf->strm_num = sinfo.time_len / fragment_time + 1 - sinfo.time_base / fragment_time;
	pf->byte_array = (int64_t*)IND_MALLOC(sizeof(int64_t) * pf->strm_num);
	pf->base_len = sinfo.base_len;
	pf->byte_len = sinfo.byte_len - sinfo.byte_base;

	pf->base_sn = sinfo.time_base / fragment_time;
	sn = pf->base_sn;
	for (i = 0; i < pf->strm_num; i ++, sn ++) {
		sprintf(pf->path + pf->pathlen, "/media%d", sn);
		{
			struct stat info;
			if (stat(pf->path, &info))
				ERR_OUT("stat %s, err = %d/%s\n", pf->path, errno, strerror(errno));
			size = (int)info.st_size;
		}
		if (i == 0) {
			if (pf->base_len < size)
				pf->byte_array[i] = size - pf->base_len;
			else
				pf->byte_array[i] = size;
		} else {
			pf->byte_array[i] = pf->byte_array[i - 1] + size;
		}
		if (pf->byte_array[i] > pf->byte_len) {
			pf->byte_array[i] = pf->byte_len;
			pf->strm_num = i + 1;
			break;
		}
	}

	sn = pf->base_sn;
	DBG_PRN("base_sn = %d, strm_num = %d\n", sn, pf->strm_num);
	sprintf(pf->path + pf->pathlen, "/media%d", sn);
	pf->fd = open(pf->path, O_RDONLY);
	if (pf->fd == -1)
		ERR_OUT("file_open %s\n", pf->path);
	if (pf->base_len > 0) {
		if (lseek(pf->fd, (off_t)pf->base_len, SEEK_SET) == -1)
			ERR_OUT("fseek %s off = %d, err = %d/%s\n", pf->path, pf->base_len, errno, strerror(errno));
	}

	pf->byte_off = 0;
	pf->strm_index = 0;

	return pf;
Err:
	int_pvr_fclose(pf);
	return NULL;
}

int ind_pvr_fseek(PvrFile_t pf, int64_t offset)
{
	int i, num, off;
	int64_t *byte_array;
	if (pf == NULL)
		ERR_OUT("pf is NULL\n");

	num = pf->strm_num;
	byte_array = pf->byte_array;

	if (offset < 0 || offset >= pf->byte_len)
		ERR_OUT("offset = %lld / %lld\n", offset, pf->byte_len);

	for (i = 0; i < num; i ++) {
		if (byte_array[i] > offset)
			break;
	}
	if (pf->fd != -1) {
		close(pf->fd);
		pf->fd = -1;
	}
	if (i >= num) {
		WARN_PRN("offset = %lld / %lld\n", offset, byte_array[num - 1]);
		pf->strm_index = -1;
	} else {
		int sn = pf->base_sn + i;

		sprintf(pf->path + pf->pathlen, "/media%d", sn);
		pf->fd = open(pf->path, O_RDONLY);
		if (pf->fd == -1)
			ERR_OUT("file_open %s\n", pf->path);
		pf->strm_index = i;
	
		if (0 == i)
			off = pf->base_len + (int)offset;
		else
			off = (int)(offset - byte_array[i - 1]);
	
		DBG_PRN("i = %d, num = %d, off = %d, base_len = %d, offset = %lld\n", i, num, off, pf->base_len, offset);
		if (lseek(pf->fd, (off_t)off, SEEK_SET) == -1)
			ERR_OUT("fseek %s off = %d, err = %d/%s\n", pf->path, off, errno, strerror(errno));
		pf->byte_off = offset;
	}

	return 0;
Err:
	return -1;
}

int ind_pvr_fread(PvrFile_t pf, char* buf, int len)
{
	int i, ret;

	if (!pf || !buf || len <= 0)
		ERR_OUT("pf = %p, buf = %p, len = %d\n", pf, buf, len);

	if (pf->strm_index == pf->strm_num - 1) {
		int byte_len = pf->byte_len - pf->byte_off;
		if (byte_len <= 0) {
			WARN_PRN("byte_len = %lld, byte_off = %lld\n", pf->byte_len, pf->byte_off);
			return 0;
		}
		if (len > byte_len)
			len = byte_len;
	}
	if (-1 == pf->fd) {
Fill:
		for (ret = 0; ret + 188 <= len; ret += 188) {
			unsigned char *fill = (unsigned char *)(buf + ret);
			fill[0] = 0x47;
			fill[1] = 0x1F;
			fill[2] = 0xFF;
			fill[3] = 0x10;
			for (i = 4; i < 188; i ++)
				fill[i] = 0xFF;
		}
		pf->byte_off += ret;
		return ret;
	}
	ret = read(pf->fd, buf, len);
	if (ret < 0)
		ERR_OUT("file_read %s\n", pf->path);

	if (ret == 0) {
		close(pf->fd);
		pf->fd = -1;

		if (pf->strm_index >= pf->strm_num - 1) {
			WARN_PRN("strm_index = %d / %d\n", pf->strm_index, pf->strm_num);
			goto Fill;
		}

		pf->strm_index ++;
		sprintf(pf->path + pf->pathlen, "/media%d", pf->base_sn + pf->strm_index);
		pf->fd = open(pf->path, O_RDONLY);
		if (pf->fd == -1)
			ERR_OUT("file_open %s\n", pf->path);
		ret = read(pf->fd, buf, len);
		if (ret < 0)
			ERR_OUT("read %s\n", pf->path);
		if (ret == 0)
			WARN_PRN("read %s\n", pf->path);
	}
	pf->byte_off += ret;

	return ret;
Err:
	return -1;
}

void ind_pvr_fclose(PvrFile_t pf)
{
	int_pvr_fclose(pf);
}

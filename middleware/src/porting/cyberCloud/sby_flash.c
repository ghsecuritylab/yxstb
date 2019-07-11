#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "cloud_api.h"
#include "config/pathConfig.h"

#define CLOUD_DATA_FILE_NAME DEFAULT_MODULE_CLOUD_DATAPATH"/flash_data"
#define CLOUD_FIRST_BLOCK_SIZE 0x10000
#define CLOUD_SECOND_BLOCK_SIZE 0x40000

void cybercloud_flash_init(void)
{
	int fd = 0;

	if(!access (CLOUD_DATA_FILE_NAME, F_OK)){
		CLOUD_LOG_TRACE("cloud file(%s) exist:\n", CLOUD_DATA_FILE_NAME);
	    return ;
	}

	if((fd = open(CLOUD_DATA_FILE_NAME,  O_RDWR|O_CREAT, 0644 )) < 0){
	    CLOUD_LOG_TRACE("[%s] file create fiailue!!\n", CLOUD_DATA_FILE_NAME);
	    return ;
	}

	close(fd);
}

static C_U32 cybercloud_flash_read(C_U32 offset, C_U8* buf, C_U32 count)
{
	C_U32 fd;
	C_U32 read_len;

	//CLOUD_LOG_TRACE("file_path read = %s\n", file_path);
	if(access (CLOUD_DATA_FILE_NAME, F_OK) != 0){
		CLOUD_LOG_TRACE("read error %d offset = %d  buf = %#x, count = %d, file_path = %s\n", __LINE__, offset, buf, count, CLOUD_DATA_FILE_NAME);
	    return -1;
	}

	if((fd = open(CLOUD_DATA_FILE_NAME, O_RDONLY)) < 0){
		CLOUD_LOG_TRACE("read error %d\n", __LINE__);
		return -1;
	}

	if(lseek(fd, offset, SEEK_SET) < 0){
		CLOUD_LOG_TRACE("read error %d\n", __LINE__);
 		close(fd);
		return -1;
	}

	read_len = read(fd, buf, count);
 	if(read_len != count){
		CLOUD_LOG_TRACE("read Line:%d read_len = %d, count = %d\n", __LINE__, read_len, count);
 	}

	close(fd);
	return read_len;
}

static int cybercloud_flash_erase(C_U32 offtet, C_U32 block_size)
{
	C_U8 fill_data[1024];
	C_U32 fd, size;

	if((fd = access (CLOUD_DATA_FILE_NAME, F_OK)) != 0){
		CLOUD_LOG_TRACE("flash_erase error\n");
        return -1;
	}
	fd = open(CLOUD_DATA_FILE_NAME, O_WRONLY);
	if(fd < 0){
		CLOUD_LOG_TRACE("flash_erase error\n");
		return -1;
	}
 	if(lseek(fd, offtet, SEEK_SET) < 0){
		CLOUD_LOG_TRACE("flash_erase error\n");
	}

	memset(fill_data, 0xFF, 128);
	for(size = 0; size < block_size; size += 1024){
 		if(write(fd, fill_data, 1024) != 1024)
			break;
	}

	close(fd);
 	if (size != block_size){
		CLOUD_LOG_TRACE("flash_erase error\n");
  		return -1;
 	}

	return 0;
}

static C_U32 cybercloud_flash_write(C_U32 offset, C_U8 const *buf, C_U32 count)
{
	C_U32 fd;
	C_U32 ret;

	//CLOUD_LOG_TRACE("write offset = %d, buf = %s\n", offset, buf);
	if(access (CLOUD_DATA_FILE_NAME, F_OK) != 0){
		CLOUD_LOG_TRACE("flash_write error\n");
        return -1;
	}

	fd = open(CLOUD_DATA_FILE_NAME, O_WRONLY);
	if(fd < 0){
		CLOUD_LOG_TRACE("flash_write error\n");
		return -1;
	}
 	if(lseek(fd, offset, SEEK_SET) < 0){
		CLOUD_LOG_TRACE("flash_write error\n");
		return -1;
	}

 	if(write(fd, buf, count) != count){
		CLOUD_LOG_TRACE("flash_write error\n");
		close(fd);
		return -1;
 	}
 	close(fd);

 	return count;
}

C_RESULT  CStb_FlashRead(IN C_U8 uBlockID, OUT C_U8 *pBuf,IN C_U32 uBytesToRead, OUT C_U32 *puBytesRead)
{
	C_U32 read_len;
	C_U32 offset;

	switch(uBlockID){
	case CLOUD_FLASH_BLOCK_A:
		offset = 0;
		break;
	case CLOUD_FLASH_BLOCK_B:
		offset = CLOUD_FIRST_BLOCK_SIZE;
		break;
	default:
		CLOUD_LOG_TRACE("Block Error\n");
		return CLOUD_FAILURE;
	}

	read_len = cybercloud_flash_read(offset, pBuf, uBytesToRead);
	//CLOUD_LOG_TRACE("pBuf = %s, offset = %#x uBytesToRead = %d read_len = 0\n", pBuf, offset, uBytesToRead, read_len);
	if(read_len == -1){
		*puBytesRead = 0;
		return CLOUD_FAILURE;
	}

	*puBytesRead = read_len;
	return CLOUD_OK;
}

C_RESULT  CStb_FlashWrite (IN C_U8 uBlockID, IN C_U8 const *pBuf, IN C_U32 uBytesToWrite)
{
	C_U32 offset = 0;
	C_U32 size = 0;

	switch(uBlockID){
	case CLOUD_FLASH_BLOCK_A:
		offset = 0;
		size = CLOUD_FIRST_BLOCK_SIZE;
		break;
	case CLOUD_FLASH_BLOCK_B:
		offset = CLOUD_FIRST_BLOCK_SIZE;
		size = CLOUD_SECOND_BLOCK_SIZE;
		break;
	default:
		CLOUD_LOG_TRACE("Block Error\n");
		return CLOUD_FAILURE;
	}

	//CLOUD_LOG_TRACE("Write Flash offset = %#x, pBuf = %s\n", offset , pBuf);
	cybercloud_flash_erase(offset, size);

	if(cybercloud_flash_write(offset, pBuf, uBytesToWrite) == -1)
		return CLOUD_FAILURE;

	return CLOUD_OK;
}


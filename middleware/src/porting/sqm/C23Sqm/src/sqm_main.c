/*
 * filename : sqm_main.c
 * create date: 2010.12.7
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include "sqm_port.h"
#include "sqm_types.h"


#define SQM_PRINT(x...) \
	do{ \
        char w_buf[256] = { 0 }; \
		sprintf ( w_buf, "[SQM]:[%s]:[%4d]", __FILE__, __LINE__ ); \
        write( fd_info_w, w_buf, 256 ); \
        bzero( w_buf, 256 ); \
        sqm_sent_log( w_buf, x ); \
        write( fd_info_w, w_buf, 256 ); \
	}while(0)

static int fd_info_w;

/*
 * ===  FUNCTION  ===============================================================
 *         Name:  sqm_sent_log
 *  Description:  将独立模块sqm.elf日志(含有变参)格式化为字符串.
 *        Input:  str : 指向格式化后的字符串; fmt & ... : 变参 & 变量
 *       Output:  格式化后的字符串的长度.
 *        Other:  在SQM_PRINT宏函数中被调用.
 *       Author:  Michael           03/09/2011 04:47:36 PM
 * ==============================================================================
 */
int sqm_sent_log( char * str, char * fmt, ... )
{
	int count;
	int size = 256;
	va_list ap;
	va_start( ap, fmt );
	count = vsnprintf( str, size, fmt, ap );
	va_end( ap );
	return count;
}



// sqm log call back function
MDI_INT16 sqm_log(MDI_CHAR *pLogData)
{
	if(pLogData == NULL){
		SQM_PRINT("NULL string pointer\n");
		return -1;
	}
	SQM_PRINT("sqm log: %s\n", pLogData);
	return 0;
}


int main(int argc, char *argv[])
{
	int ret;
	int fd_mid_to_sqm;
	int size;
	int paralen;
	int leftlen;
	int flag_sqm_start = 0;                  // to indicate whether sqm start interface is called
	unsigned char readbuf[MAX_MSG_LEN];
	fd_set fdSets;
	SQM_STATUS sqm_stat;
	MDI_UINT32 mdi_ret;
	MDI_INT16 mdiErrorNum;
	MDI_LOGLEVELS LogLevel;
	MDI_UINT32 MqmcIp;
	MDI_STB_INFO StbInfo;
	MDI_CHANNEL_INFO ChannelInfo;

	/* Eagle. add 2011年01月19日 */
	unsigned int sqm_listen_port = 0;
	unsigned int sqm_server_port = 0;

	umask(0);

	/* Eagle. add for using iptv.elf's telnet debug  at 02/18/2011 11:33:18 AM */
	if ( -1 == access ( TELNET_FIFO, F_OK ) ){
		if ( mkfifo( TELNET_FIFO, FIFO_MODE) < 0 ){
			perror( " mkfifo error! " );
		}
	}
	fd_info_w = open ( TELNET_FIFO, O_RDWR | O_NONBLOCK );
	if ( -1 == fd_info_w ){
		perror( " open fifo error ! " );
		exit(-1);
	}

	SQM_PRINT("\nSQM is running, build time is %s %s\n", __TIME__, __DATE__);

	ret = mknod(MID_TO_SQM, S_IFIFO | 0666, 0);
	if(ret == -1){
		SQM_PRINT("error to create fifo!\n");
		//exit(-1);
	}

	fd_mid_to_sqm = open(MID_TO_SQM, 0666);
	if(fd_mid_to_sqm == -1){
		SQM_PRINT("error to open fifo: %s\n", MID_TO_SQM);
		exit(-1);
	}

	//register the sqm log call back function
	mdi_ret = PROBE_RegisterLogCallBackFunction(sqm_log, &mdiErrorNum);
	if(mdi_ret != 0){
		SQM_PRINT("ERROR: error number = %d\n", mdiErrorNum);
	}

	sqm_stat = SQM_READY;
	memset(&StbInfo, 0, sizeof(StbInfo));
	while(1){
		FD_ZERO(&fdSets);
		FD_SET(fd_mid_to_sqm, &fdSets);

		ret = select(fd_mid_to_sqm + 1, &fdSets, NULL, NULL, NULL);
		if(ret > 0){
			if(FD_ISSET(fd_mid_to_sqm, &fdSets)){
				memset(readbuf, 0, MAX_STR_LEN);
				size = read(fd_mid_to_sqm, readbuf, 3);
				if(size != 3){
					SQM_PRINT("Can NOT read enough data!\n");
					continue;
				}
				paralen = (readbuf[1]<<8) + readbuf[2];
				size = read(fd_mid_to_sqm, &readbuf[3], paralen);
				leftlen = paralen;
				while(leftlen > size){
					leftlen -= size;
					SQM_PRINT("read again, left %d bytes\n", leftlen);
					sleep(1);
					size = read(fd_mid_to_sqm, &readbuf[paralen - leftlen + 3], leftlen);
				}
				switch(readbuf[0]){
				case SQM_CMD_INIT:
					SQM_PRINT("receive init command!\n");
					LogLevel = readbuf[3];
					MqmcIp = (readbuf[4]<<24) + (readbuf[5]<<16) + (readbuf[6]<<8) + readbuf[7];
					memcpy(StbInfo.sUserId, &readbuf[8],32);
					memcpy(StbInfo.sMacAddr, &readbuf[40], 17);
					memcpy(StbInfo.sStbId, &readbuf[57], 32);

					StbInfo.EpgIp = (readbuf[89]<<24) + (readbuf[90]<<16) + (readbuf[91]<<8) + readbuf[92];
					StbInfo.StbIpInfo.ServiceIp = (readbuf[93]<<24) + (readbuf[94]<<16) + (readbuf[95]<<8) + readbuf[96];
					switch(readbuf[97]){
					case TYPE_ETH0:
						strcpy(StbInfo.StbIpInfo.sServiceNetName, NET_NAME_ETH0);
						break;
					case TYPE_PPPOE0:
						strcpy(StbInfo.StbIpInfo.sServiceNetName, NET_NAME_PPPOE0);
						break;
					case TYPE_ETH0_WI:
						strcpy(StbInfo.StbIpInfo.sServiceNetName, NET_NAME_ETH0_WI);
						break;
					default:
						SQM_PRINT("Illegal net name: %d\n", readbuf[102]);
					}
					StbInfo.MutilcastNetInfo.ServiceIp = (readbuf[98]<<24) + (readbuf[99]<<16) + (readbuf[100]<<8) + readbuf[101];
					switch(readbuf[102]){
					case TYPE_ETH0:
						strcpy(StbInfo.MutilcastNetInfo.sServiceNetName, NET_NAME_ETH0);
						break;
					case TYPE_PPPOE0:
						strcpy(StbInfo.MutilcastNetInfo.sServiceNetName, NET_NAME_PPPOE0);
						break;
					case TYPE_ETH0_WI:
						strcpy(StbInfo.MutilcastNetInfo.sServiceNetName, NET_NAME_ETH0_WI);
						break;
					default:
						SQM_PRINT("Illegal net name: %d\n", readbuf[102]);
					}

					/* Eagle add and modify . 2011年01月19日 */
					sqm_listen_port = sqm_server_port = 0;
					sqm_listen_port = (readbuf[103]<<24) + (readbuf[104]<<16) + (readbuf[105]<<8) + readbuf[106];
					if ( 0 == sqm_listen_port )
					{
						sqm_listen_port = EM_RECV_MQMC_PORT; /* 37001 */
					}
					sqm_server_port = (readbuf[107]<<24) + (readbuf[108]<<16) + (readbuf[109]<<8) + readbuf[110];
					if ( 0 == sqm_server_port )
					{
						sqm_server_port = SENDTO_MQMC_PORT; /* 37000 */
					}
					if ( MAX_ID_LEN < paralen - 110 + 3 )
					{
						SQM_PRINT("pppoe acc len is %d, bigger than max len %d\n", paralen - 110, MAX_ID_LEN);
						break;
					}
					memcpy(StbInfo.sPPPoEAccount, &readbuf[111], paralen + 2 - 110);
					StbInfo.sPPPoEAccount[paralen + 2 - 110 + 1] = '\0';

					mdi_ret = PROBE_initProbeCBB(DEVICE_STB, LogLevel, sqm_listen_port, sqm_server_port, \
						MqmcIp, &StbInfo, NULL, &mdiErrorNum);
					if(mdi_ret != 0)
					{
						SQM_PRINT("ERROR: error number = %d\n", mdiErrorNum);
					}
					SQM_PRINT("log level=%d, mqmc ip=%#x, stb ip=%#x, stb net=%s,  EPG ip=%#x listen_por=%d, server_port=%d.\n", LogLevel, MqmcIp, \
						StbInfo.StbIpInfo.ServiceIp, StbInfo.StbIpInfo.sServiceNetName,StbInfo.EpgIp, sqm_listen_port, sqm_server_port);
					sqm_stat = SQM_INIT_OK;
					break;
				case SQM_CMD_START_MONITOR:
					SQM_PRINT("receive start monitor command!\n");
					if(flag_sqm_start != 0){
						SQM_PRINT("already called sqm start interface! \n");
					}else{
						if(sqm_stat != SQM_INIT_OK){
							SQM_PRINT("sqm status is %d, wrong start command!\n", sqm_stat);
							break;
						}
						mdi_ret = PROBE_StartMornitor(&mdiErrorNum);
						if(mdi_ret != 0){
							SQM_PRINT("ERROR: error number = %d\n", mdiErrorNum);
						}
						flag_sqm_start = 1;       // alread called PROBE_StartMornitor()
					}
					sqm_stat = SQM_START_OK;
					break;
				case SQM_CMD_SETCHANNELINFO:
					SQM_PRINT("receive set channel info command!\n");
					if((sqm_stat != SQM_START_OK) && (sqm_stat != SQM_SETCHANNELINFO_OK)){
						SQM_PRINT("sqm status is %d, wrong set channel info command!\n", sqm_stat);
						break;
					}
					memset(ChannelInfo.ChannelUrl, 0, MAX_STR_LEN);
					ChannelInfo.MediaType = readbuf[3];
					if(ChannelInfo.MediaType == STB_IDLE){
						ChannelInfo.ChannelIp = 0;
						ChannelInfo.ChannelPort = 0;
						ChannelInfo.StbPort = 0;
					}else{
						ChannelInfo.ChannelIp = (readbuf[4]<<24) + (readbuf[5]<<16) + (readbuf[6]<<8) + readbuf[7];
						ChannelInfo.ChannelPort = (readbuf[8]<<24) + (readbuf[9]<<16) + (readbuf[10]<<8) + readbuf[11];
						ChannelInfo.StbPort = (readbuf[12]<<24) + (readbuf[13]<<16) + (readbuf[14]<<8) + readbuf[15];
						if(paralen - 13 > MAX_STR_LEN){
							SQM_PRINT("channel url len is %d, bigger than max len %d\n", paralen - 16, MAX_STR_LEN);
							break;
						}
						strcpy(ChannelInfo.ChannelUrl, &readbuf[16]);
					}
					// set channel info
					mdi_ret = PROBE_SetChannelInfo(&ChannelInfo, &mdiErrorNum);
					if(mdi_ret != 0){
						SQM_PRINT("ERROR: error number = %d\n", mdiErrorNum);
					}
					SQM_PRINT("media stat = %d, channel ip = %#x, channel port = %d, stb port = %d\n", \
						ChannelInfo.MediaType, ChannelInfo.ChannelIp, ChannelInfo.ChannelPort, ChannelInfo.StbPort);
					sqm_stat = SQM_SETCHANNELINFO_OK;
					break;
				case SQM_CMD_STOP_MONITOR:
					SQM_PRINT("receive stop monitor command!\n");
					if((sqm_stat == SQM_READY) || (sqm_stat == SQM_STOP_OK)){
						SQM_PRINT("sqm status is %d, wrong stop monitor command!\n", sqm_stat);
						break;
					}

					if(flag_sqm_start == 0){
						SQM_PRINT("Don't called sqm start interface! \n");
					}else{
						mdi_ret = PROBE_StopMornitor(&mdiErrorNum);
						if(mdi_ret != 0){
							SQM_PRINT("ERROR: error number = %d\n", mdiErrorNum);
						}
						flag_sqm_start = 0;
					}
					sqm_stat = SQM_STOP_OK;
					break;
				default:
					SQM_PRINT("Unknown sqm command: %#x!\n", readbuf[0]);
				}
			}
		}else{
			SQM_PRINT("time out!\n");
		}
		SQM_PRINT("sqm program running!\n");
		sleep(1);
	}
	return 0;
}

/****************************end of file******************************************/


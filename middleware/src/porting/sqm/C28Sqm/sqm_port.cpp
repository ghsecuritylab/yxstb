/*
 * filename  : sqm_port.c
 * create date: 2010.12.7
 *
 */
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/un.h>
#include <dirent.h>
#include <sys/time.h>

#include "sqm_port.h"
#include "sqm_types.h"
#include "sys_basic_macro.h"
#include "AppSetting.h"
#include "SysSetting.h"
#include "VersionSetting.h"
#include "app_sys.h"

#include "mid_sys.h"
#include "log_sqm_port.h"
#include "mid/mid_mutex.h"
#include "LogModule.h"
#include "NetworkFunctions.h"
#include "config/pathConfig.h"
#include "mid/mid_tools.h"
#include "customer.h"

#define SQM_MSG_FIFO DEFAULT_PIPE_DATAPATH"/sqm_msg_fifo"
#define STB_PACK_CACHE_NUM 1024
#define IND_STRNCPY(dest, src, n)	strncpy(dest, src, n)
#define IND_MEMCPY(dest,src, n)	memcpy(dest,src, n)
#define IND_STRCPY(dest, src)		strcpy(dest, src)
#define CONFIG_FILE_DIR "/root"

static int sqmpro_flag = 0;

typedef struct sqm_data_cache {
	unsigned int pkg_count;
	unsigned int pkg_count_max;
	unsigned int base;

	unsigned int head_num;
	unsigned int tail_num;
	unsigned int data_len;
	STB_TS_PACK stb_pack[STB_PACK_CACHE_NUM];
}SQM_DATA_CACHE;

static int fd_sqm_msg_fifo = 0;

// fd of fifo
static int fd_write_to_sqm;

//status record
static SQM_STATUS sqm_port_stat;

static CHN_STAT g_chn_stat = STB_IDLE;// channel statu
static int g_chn_ip;          // channel ip address
static int g_chn_port;        // channel port
static int g_local_chn_port;  // local stream port
static char g_chn_url[MAX_SQM_URL_STR_LEN];  // channel url

/* Eagle add. 2011年01月19日*/
static unsigned int g_listen_port = 37001;      /* default */
static unsigned int g_server_port = 37000;      /* default */
static int fd_info_r = -1;                      /* description  /var/telnet_fifo */
static void * print_task ( void * para );       /* get print info from sqm.elf for using telnet debug */

static char g_mqmip[16];      // mqm ip

//messge buffer
static char msgbuf[MAX_MSG_LEN];

/*纠错状态从纠错后切换到纠错前，最大的判断值*/
static int pushdata_sockfd = -1;
static struct sockaddr_un servaddr = {0};
static struct sockaddr_un cliaddr = {0};
static SQM_DATA_CACHE g_pack_cache;
//C26 SQM  需要的参数.
static char root_path[20] = {"/root/sqm.ini"};
static char var_path[20] = { "/var/sqm.ini" };
static int pushmsg_sockfd = -1;
static SQM_DATA_CACHE g_msg_cache;
static struct sockaddr_un msg_servaddr = {0};
static struct sockaddr_un msg_cliaddr = {0};
//C28 SQM
static int get_sqm_data_type = 0;


extern "C" char* Hybroad_getHWtype();
static void sqm_port_init();
static void sqm_port_start();
static void sqm_port_set_info();
static void sqm_port_stop();
static void sqm_cache_save(STB_TS_PACK *stb_pack);
static int sqm_cache_load_begin(STB_TS_PACK *stb_pack);
static void sqm_cache_load_end(void);
static void sqm_cache_clear(void);
static int sqm_port_pushmsg(int pIndex, SQM_MSG_C26 *sqm_msg_c26, int len, SQM_MSG sqm_msg);

SQM_STATUS sqm_port_getStatus(void)
{
	return sqm_port_stat;
}

void sqm_port_setStatus(SQM_STATUS status)
{
	sqm_port_stat = status;
}

// change string of number to dec
static int strTOdec(const char *pstr, char *decnum)
{
	if((pstr == NULL) || (decnum == NULL)){
		PRINTF("NULL string pointer\n");
		return -1;
	}
	if(strlen(pstr) > 3){
		PRINTF("string is too long, maybe bigger than int can hold\n");
		return -1;
	}

	*decnum = 0;

	while(*pstr != '\0'){
		if((*pstr < '0') || (*pstr > '9')){
			PRINTF("Illegal number string: %s\n", pstr);
			return -1;
		}
		*decnum *= 10;
		*decnum += *pstr - 0x30;
		pstr++;
	}

	return 0;
}

// change ip string to char array
// psrc: 11.22.33.212  then
// pdes: pdes[0] = 11; pdes[1]=22; pdes[2]=33; pdes[3]=212;
static int stripTOuint(char *pdes, const char *psrc)
{
	char *pstr;
	char *pstr1;
	char tmpbuf[4];
	int tmplen;
	int ret;

	if((pdes == NULL) || (psrc == NULL)){
		PRINTF("NULL string pointer\n");
		return -1;
	}

	pstr = (char *)strchr(psrc, '.'); //WZW modified to fix pc-lint Error 158
	if(pstr == NULL){
		PRINTF("Illegal ip string, can NOT find the seperater. --1st\n");
		return -1;
	}
	tmplen = pstr - psrc;
	memcpy(tmpbuf, psrc, tmplen);
	tmpbuf[tmplen] = '\0';
	ret = strTOdec(tmpbuf, &pdes[0]);
	if(ret == -1){
		PRINTF("get ip error 1st\n");
		return -1;
	}

	pstr1 = pstr + 1;
	pstr = strchr(pstr1, '.');
	if(pstr == NULL){
		PRINTF("Illegal ip string, can NOT find the seperater. --2nd\n");
		return -1;
	}
	tmplen = pstr - pstr1;
	memcpy(tmpbuf, pstr1, tmplen);
	tmpbuf[tmplen] = '\0';
	ret = strTOdec(tmpbuf, &pdes[1]);
	if(ret == -1){
		PRINTF("get ip error 2nd\n");
		return -1;
	}

	pstr1 = pstr + 1;
	pstr = strchr(pstr1, '.');
	if(pstr == NULL){
		PRINTF("Illegal ip string, can NOT find the seperater. --3rd\n");
		return -1;
	}
	tmplen = pstr - pstr1;
	memcpy(tmpbuf, pstr1, tmplen);
	tmpbuf[tmplen] = '\0';
	ret = strTOdec(tmpbuf, &pdes[2]);
	if(ret == -1){
		PRINTF("get ip error 3rd\n");
		return -1;
	}

	pstr1 = pstr + 1;
	tmplen = strlen(psrc) - (pstr1 - psrc);
	memcpy(tmpbuf, pstr1, tmplen);
	tmpbuf[tmplen] = '\0';
	ret = strTOdec(tmpbuf, &pdes[3]);
	if(ret == -1){
		PRINTF("get ip error 4th\n");
		return -1;
	}
	return 0;
}

static void sqm_port_write(int fd, char *buf, int len)
{
	int ret;
	int retrytimes;
	int datasend;

	retrytimes = 0;
	datasend = 0;

	if( buf == NULL ){
		PRINTF("buf is a NULL pointer!\n");
		return;
	}
	if((fd == 0) || (len ==0)){
		PRINTF("wrong fd or no data to write; fd = %d, len = %d\n", fd, len);
		return;
	}

	while(datasend < len){
		ret = write(fd, buf+datasend, len - datasend);
		if(ret == -1){
			PRINTF("write data to sqm program failed!send data len = %d, total len = %d\n", datasend, len);
			return;
		}else{
			datasend += ret;
			PRINTF("send data len = %d, current send data len = %d\n", datasend, ret);
			retrytimes ++;
			if(retrytimes > 3){
				PRINTF("already retry %d times, send data = %d, total = %d\n", retrytimes, datasend, len);
				usleep(200000);         // wait 200 ms
				//return;
			}
		}
	}
}

// write start message to the sqm module
static void sqm_port_start()
{
	msgbuf[0] = SQM_CMD_START_MONITOR;
	msgbuf[1] = 0;
	msgbuf[2] = 0;
	sqm_port_write(fd_write_to_sqm, msgbuf, 3);

	PRINTF("[SQM_PORT]: send start command to sqm module\n");
	sqm_port_stat = SQM_START_OK;
}

// parse and save channel info
// when the pointer is NULL, this function will do nothing to the acordding para;
void parseChnInfo(CHN_STAT pStat, struct sockaddr_in* serv_sin, struct sockaddr_in* data_sin,char* url)
{
	//char tmpbuf[MAX_SQM_URL_STR_LEN];
	char *pstr;
	int tmplen;

//	mid_mutex_lock(g_sqm_mutex);
	g_chn_stat = pStat;

	if(serv_sin != NULL){  // single
		g_chn_ip = ntohl(serv_sin->sin_addr.s_addr);
		g_chn_port = ntohs(serv_sin->sin_port);
		if(data_sin != NULL){
			g_local_chn_port = ntohs(data_sin->sin_port);
		}else{
			PRINTF("error to get local port in single mode\n");
		}
	}else{    // multi
		if(data_sin != NULL){
			g_chn_ip = ntohl(data_sin->sin_addr.s_addr);
			g_chn_port = ntohs(data_sin->sin_port);
			g_local_chn_port = ntohs(data_sin->sin_port);
		}
	}

	if(url != NULL){
		memset(g_chn_url, 0, MAX_SQM_URL_STR_LEN);      // clear url array
		if(serv_sin != NULL){    // unicast
			pstr = strchr(url, '?');         // drop the parameters in url
			if(pstr == NULL){
				PRINTF("Illegal ip string, can NOT find the seperater ? in channel url\n");
//				mid_mutex_unlock(g_sqm_mutex);
				return;
			}
			tmplen = pstr - url;
		}else{                // multicast
			tmplen = strlen(url);
		}
		if(tmplen > MAX_SQM_URL_STR_LEN){
			PRINTF("The channel url is too long! len=%d, bigger than %d\n", tmplen, MAX_SQM_URL_STR_LEN);
		}else{
			memcpy(g_chn_url, url, tmplen);
		}
	}
//	mid_mutex_unlock(g_sqm_mutex);
}

// mqm_ip_set
void sqm_port_mqmip_set( const char *ipaddr)
{
	int tmplen;

	tmplen = strlen(ipaddr);
	if(tmplen > 15){             // ipaddr format: xxx.xxx.xxx.xxx
		PRINTF("The ip address is too long, len = %d\n", tmplen);
		return;
	}else{
//		mid_mutex_lock(g_sqm_mutex);
		memset(g_mqmip, 0, 16);
		strcpy(g_mqmip, ipaddr);
//		mid_mutex_unlock(g_sqm_mutex);
	}
}

// mqm_ip_get
void sqm_port_mqmip_get(char *ipaddr)
{
	strcpy(ipaddr, g_mqmip);
	return ;
}


// write msg to fifo
int sqm_port_msg_write(SQM_MSG msg)
{
	int ret;
	fd_set wset;
	struct timeval tv;

	tv.tv_sec = 2;         // wait 2s for message fifo write
	tv.tv_usec = 0;

	FD_ZERO(&wset);
	FD_SET(fd_sqm_msg_fifo, &wset);
	ret = select(fd_sqm_msg_fifo + 1, NULL, &wset, NULL, &tv);
	if(ret <= 0){
		PRINTF("Can't send msg, wait time out!\n");
		return -1;
	}

	ret = write(fd_sqm_msg_fifo, &msg, 1);
	if(ret != 1){
		PRINTF("write msg failed!\n");
		return -1;
	}

	return 0;
}

static void sqm_port_stop()
{
	if(sqm_port_stat == SQM_STOP_OK){
		PRINTF("ERROR: sqm already stop, now stat is %d\n", sqm_port_stat);
        return;
	}
	msgbuf[0] = SQM_CMD_STOP_MONITOR;
	msgbuf[1] = 0;
	msgbuf[2] = 0;
	sqm_port_write(fd_write_to_sqm, msgbuf, 3);

	PRINTF("[SQM_PORT]: send stop command to sqm module\n");
	sqm_port_stat = SQM_STOP_OK;
}

// sqm task
static void *sqm_task(void *para)
{
	int ret;
	fd_set rset;
	char tmpbuf[5];

	while(1){
		FD_ZERO(&rset);
		FD_SET( fd_sqm_msg_fifo, &rset);

		ret = select(fd_sqm_msg_fifo + 1, &rset, NULL, NULL, NULL);
		if(ret > 0){
			ret = read(fd_sqm_msg_fifo, tmpbuf, 1);
			if(ret == 1){
				switch(tmpbuf[0]){
					case MSG_START:
						sleep(1);         // wait 2s to start
						sqm_port_start();
						break;
					case MSG_STOP:
						sqm_port_stop();
						break;
					default:
						PRINTF("unknown sqm message!\n");
				}
			}
		}else{
			PRINTF("select error on sqm msg read!\n");
        }
    }
    return NULL;
}

// prepare for sqm porting module
void sqm_port_prepare(void)
{
    pthread_t sqm_thread_handle = 0;
    pthread_t print_thread_handle = 0;

    umask(0);
    /* Eagle. add one thread print_task at 02/18/2011 01:07:51 PM */
    if ( -1 == access ( TELNET_FIFO, F_OK ) )
    {
        if ( mkfifo ( TELNET_FIFO, FIFO_MODE ) < 0 )
        {
            PRINTF( " Error: mkfifo TELNET_FIFO! \n" );
        }
    }

    fd_info_r = open ( TELNET_FIFO, O_RDONLY | O_NONBLOCK );

    if ( -1 == fd_info_r )
    {
        PRINTF( " Error: open TELNET_FIFO! \n");
        return;
    }

    if (pthread_create(&print_thread_handle, NULL, print_task, NULL))
    {
        PRINTF( "Error: create pthread print_task! \n" );
        close ( fd_info_r );
    }

    fd_write_to_sqm = open(MID_TO_SQM, 0666);
    if(fd_write_to_sqm == -1){
        PRINTF("error to open FIFO!\n");
        sqm_port_stat = SQM_ERROR;
        return;
    }

    int ret = unlink(SQM_MSG_FIFO);
	PRINTF("Path:/var/sqm_msg_fifo, unlink=%d, errno:%d[%s]\n", ret, errno, strerror(errno));

    if (mknod(SQM_MSG_FIFO, S_IFIFO|0666,0)) {/*creat a fifo*/
        PRINTF("ERROR: mknod fail!\n");
        close(fd_write_to_sqm);
        sqm_port_stat = SQM_ERROR;
        return;
    }
    fd_sqm_msg_fifo = open(SQM_MSG_FIFO, 0666);
    if(fd_sqm_msg_fifo == -1){
        PRINTF("ERROR: can NOT open sqm msg pipe!\n");
        close(fd_write_to_sqm);
        sqm_port_stat = SQM_ERROR;
        return;
    }

    if (pthread_create(&sqm_thread_handle, NULL, sqm_task, NULL)){
        PRINTF("ERROR: create sqm thread fail!\n");
        close(fd_write_to_sqm);
        close(fd_sqm_msg_fifo);
        sqm_port_stat = SQM_ERROR;
        return;
    }

    //	g_sqm_mutex = mid_mutex_create();
    //	if(g_sqm_mutex == NULL){
    //		PRINTF("create sqm mutex failed!\n");
    //	}

    sqm_port_stat = SQM_READY;
}


/* Eagle. add. 2011骞?1鏈?9鏃?*/
unsigned int sqm_get_listen_port ( void )
{
    return g_listen_port;
}
/* -----  end of function sqm_get_listen_port  ----- */

unsigned int sqm_get_server_port ( void )
{
    return g_server_port;
}
/* -----  end of function sqm_get_server_port  ----- */

void sqm_set_listen_port ( unsigned int port )
{
    if ( 0 != port )
    {
        g_listen_port = port;
    }
    return ;
}
/* -----  end of function sqm_set_listen_port  ----- */

void sqm_set_server_port ( unsigned int port )
{
    if ( 0 != port )
    {
        g_server_port = port;
    }
    return ;
}
/* -----  end of function sqm_set_server_port  ----- */

/*
 * ===  FUNCTION  ===============================================================
 *         Name:  print_task
 *  Description:  get output information from sqm.elf by fifo, then print them
 *                    using PRINTF.
 *        Input:  generic pointer (void *)
 *       Output:  generic pointer (void *)
 *        Other:  nono
 *       Author:  Eagle          02/18/2011 01:26:50 PM
 * ==============================================================================
 */
static void * print_task ( void *para )
{
    int ret;
    fd_set rset;

    while (true) {
        FD_ZERO(&rset);
        FD_SET(fd_info_r, &rset);

        ret = select(fd_info_r+1, &rset, NULL, NULL, NULL);
        if (ret > 0 && FD_ISSET( fd_info_r, &rset )) {
            char r_buf[256] = { 0 };

            if ( read ( fd_info_r, r_buf, 256 ) < 0 )
            {
                continue;
            }
            PRINTF( r_buf );
        }
    }
    close( fd_info_r );
    return NULL;
}
/* -----  end of function print_task  ----- */

int sqm_port_pushdata_open(void)
{
	int ret = 0;
//	struct timeval timeout = {0,500};
	int buf_size = MAX_BUFF_SIZE;

	PRINTF("OPEN_PUSHDATA_SOCKET\n");
	if(pushdata_sockfd > 0){
		close(pushdata_sockfd);
	}

	pushdata_sockfd = -1;
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	pushdata_sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if(pushdata_sockfd < 0){
		PRINTF("create pushdata_sockfd error!\n");
		return -1;
	}
	if (-1 == fcntl(pushdata_sockfd, F_SETFL, O_NONBLOCK)){
        PRINTF("set pushdata_sockfd nonblock error!\n");

		close(pushdata_sockfd);
		pushdata_sockfd = -1;

		return -1;
	}
	//设置socket发送缓存,buf_size 取probe_external_api.h中的MAX_BUFF_SIZE.
    if (-1 == setsockopt(pushdata_sockfd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size))){
    	PRINTF("set pushdata_sockfd buffer size error!\n");

		close(pushdata_sockfd);
		pushdata_sockfd = -1;

		return -1;
    }
//	ret = setsockopt(pushdata_sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
//	PRINTF("setsockopt=%d\n", ret);
	ret = unlink(SQM_SOCKET_TX);
	PRINTF("unlink=%d, errno = %d[%s]\n", ret, errno, strerror(errno));
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sun_family = AF_LOCAL;
    strcpy(cliaddr.sun_path, SQM_SOCKET_TX);	/* bind an address for us */

	if(-1 == bind(pushdata_sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)))
	{
		PRINTF("bind pushdata_sockfd error!, errno = %d[%s]\n", errno, strerror(errno));

		close(pushdata_sockfd);
		pushdata_sockfd = -1;
		unlink(SQM_SOCKET_TX);

		return -1;
	}

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, SQM_SOCKET_RX); 	/* fill in server's address */

	sqm_port_pushdata_clear();

	return pushdata_sockfd;
}

int sqm_port_pushdata(int pIndex, char *buf, int len, int scale)
{
	//指数采样发送数据包
	SQM_DATA_CACHE *pack_cache = &g_pack_cache;

	STB_TS_PACK stb_pack;
	int send_len = 0;
	int ret = 0, err = 0;

	if(pushdata_sockfd <= 0){
		ERR_OUT("[socket error!]\n");
	}

	if(NULL == buf || len > MAX_PKT_SIZE || len < 188){
		ERR_OUT("[data error!], len = %d\n", len);
	}

	memset(&stb_pack, 0, sizeof(stb_pack));
	memcpy(stb_pack.pack, buf, len);

	stb_pack.total_len = len;
	stb_pack.seq_num = scale;
	gettimeofday(&stb_pack.tv, NULL);
	send_len = len + 16;

	//指数退避检查纠错状态切换
	if(pack_cache->base) {
		if(pack_cache->base > STB_PACK_CACHE_NUM) {
			pack_cache->base = STB_PACK_CACHE_NUM;
			pack_cache->pkg_count = pack_cache->pkg_count_max;
		}
		pack_cache->base--;

		sqm_cache_save(&stb_pack);
		//PRINTF("save packet\n");
	} else if(pack_cache->data_len > 0) {
		sqm_cache_save(&stb_pack);
		while(pack_cache->data_len > 0) {
			send_len = sqm_cache_load_begin(&stb_pack) + 16;
			ret = sendto(pushdata_sockfd, &stb_pack, send_len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
			if(ret != send_len) {
				//PRINTF("re-send data failed, ret = %d, seq = %d, errno = [%d]%s\n", ret, stb_pack.seq_num, errno, strerror(errno));
				break;
			} else {
				sqm_cache_load_end();
			}
		}
		if(pack_cache->data_len > 0) {
			;
		} else {
			pack_cache->pkg_count = 0;
			pack_cache->base = 0;
		}
	} else {
		ret = sendto(pushdata_sockfd, &stb_pack, send_len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if(ret != send_len) {
			if(ret == -1) {
				err = errno;

				pack_cache->pkg_count_max = pack_cache->pkg_count;
				pack_cache->pkg_count++;
				pack_cache->base = 1<<pack_cache->pkg_count;

				if(err == 2 || err == 11) {
					sqm_cache_save(&stb_pack);
					//PRINTF("send data failed, ret = %d, seq = %d, errno = [%d]%s\n", ret, stb_pack.seq_num, errno, strerror(errno));
				} else {
					//PRINTF("send data failed, ret = %d, seq = %d, errno = [%d]%s\n", ret, stb_pack.seq_num, errno, strerror(errno));
				}
			} else {
				//PRINTF("send data failed, ret = %d, seq = %d, errno = [%d]%s\n", ret, stb_pack.seq_num, errno, strerror(errno));
			}
		} else {
			//PRINTF("send data failed, ret = %d, seq = %d, errno = [%d]%s\n", ret, stb_pack.seq_num, errno, strerror(errno));
		}
	}
	return ret;

Err:
	return -1;
}

int sqm_port_pushdata_close(void)
{
	int ret = 0;

	if(pushdata_sockfd > 0){
		close(pushdata_sockfd);
	}

	pushdata_sockfd = -1;

	ret = unlink(SQM_SOCKET_TX);

	PRINTF("unlink=%d\n", ret);

	return ret;
}

void sqm_port_pushdata_clear(void)
{
	SQM_DATA_CACHE *pack_cache = &g_pack_cache;

	memset(pack_cache, 0, sizeof(SQM_DATA_CACHE));
}

static void sqm_cache_clear(void)
{
	SQM_DATA_CACHE *pack_cache = &g_pack_cache;

	pack_cache->head_num = 0;
	pack_cache->tail_num = 0;
	pack_cache->data_len = 0;
	memset(&(pack_cache->stb_pack), 0, sizeof(STB_TS_PACK) * STB_PACK_CACHE_NUM);
}

static void sqm_cache_save(STB_TS_PACK *stb_pack)
{
	SQM_DATA_CACHE *pack_cache = &g_pack_cache;
	unsigned int num = 0, len = 0;

	len = pack_cache->data_len;

	if(len >= STB_PACK_CACHE_NUM || len == 0) {
		sqm_cache_clear();
	}

	num = pack_cache->tail_num;

	memset(&(pack_cache->stb_pack[num]), 0, sizeof(STB_TS_PACK));
	memcpy(pack_cache->stb_pack[num].pack, stb_pack->pack, stb_pack->total_len);
	memcpy(&(pack_cache->stb_pack[num].tv), &(stb_pack->tv), sizeof(struct timeval));
	pack_cache->stb_pack[num].total_len = stb_pack->total_len;
	pack_cache->stb_pack[num].seq_num = stb_pack->seq_num;

	if(pack_cache->tail_num >= STB_PACK_CACHE_NUM - 1)
		pack_cache->tail_num = 0;
	else
		pack_cache->tail_num++;

	pack_cache->data_len++;
}

static int sqm_cache_load_begin(STB_TS_PACK *stb_pack)
{
	SQM_DATA_CACHE *pack_cache = &g_pack_cache;
	int num = 0;

	num = pack_cache->head_num;

	memset(stb_pack, 0, sizeof(STB_TS_PACK));
	memcpy(stb_pack->pack, pack_cache->stb_pack[num].pack, pack_cache->stb_pack[num].total_len);
	memcpy(&(stb_pack->tv), &(pack_cache->stb_pack[num].tv), sizeof(struct timeval));
	stb_pack->total_len = pack_cache->stb_pack[num].total_len;
	stb_pack->seq_num = pack_cache->stb_pack[num].seq_num;

	return stb_pack->total_len;
}

static void sqm_cache_load_end(void)
{
	SQM_DATA_CACHE *pack_cache = &g_pack_cache;

	if(pack_cache->head_num >= STB_PACK_CACHE_NUM - 1)
		pack_cache->head_num = 0;
	else
		pack_cache->head_num++;

	if(pack_cache->data_len > 0)
		pack_cache->data_len--;
	else
		pack_cache->data_len = 0;
}

#if 0
int sqm_getmsg_test_1(void)
{
	printf("========sqm_getmsg_test_1=============\n");
	SQM_MSG_GET data_get =  {0};
	int size = sizeof(SQM_MSG_GET);
	int data_type = 0;

	while(1) {
		memset(&data_get, 0, sizeof(SQM_MSG_GET));
		tr069_port_get_sqm_data(&data_get,  size , data_type );
		printf("================================\n");
		printf("sqm_getmsg_test_1:::%d-%d-%d-%d-%d	\n",  data_get.StbData[0].df,  data_get.StbData[0].mlr,  data_get.StbData[0].jitter,  data_get.StbData[0].OtherIndex1,  data_get.StbData[0].OtherIndex2);
		printf("sqm_getmsg_test_1:::%d-%d-%d-%d-%d	\n", data_get.StbData[1].df,  data_get.StbData[1].mlr,  data_get.StbData[1].jitter,  data_get.StbData[1].OtherIndex1,  data_get.StbData[1].OtherIndex2);
		printf("sqm_getmsg_test_1:::%d-%d-%d-%d-%d	\n",  data_get.StbData[2].df,  data_get.StbData[2].mlr, data_get.StbData[2].jitter,  data_get.StbData[2].OtherIndex1,  data_get.StbData[2].OtherIndex2);
		printf("================================\n");
		printf("@@@@@@@@@@@  data_type = %d\n",data_type);
		if (data_type== 1)
			data_type = 0;
		else
			data_type = 1;
		sleep(5);

	}
	return 0;

}
#endif
//读取root下的sqm.ini信息,保存到var下, 并启动sqoloader.
int  sqm_port_sqmloader_start(void)
{
    if (SQM_START_OK == sqm_port_stat) {
        PRINTF("ERROR: sqm already start, now stat is %d, first stop sqm\n", sqm_port_stat);
        sqm_port_msg_write(MSG_STOP);
    }

	system("rm  /var/sqm.ini");
	sqm_info_copy(var_path);

    sqm_port_msg_write(MSG_START);

#ifdef Sichuan
    sysSettingSetString("mqmcIp", g_mqmip);
#endif
	//等待 一分钟后,读取sampro版本号修改有修改
	//mid_timer_create(60, 1, sqmInfo_to_root, 0);
	//mid_timer_create(20, 1, sqm_getmsg_test_1, 0);

	return 0;
}

int  sqm_create_msg(SQM_MSG sqm_msg, SQM_MSG_C26 *sqm_msg_c26 )
{
	//char var[MAX_MSG_SIZE] = {0}, server_ip[32], stb_ip[32];
	int tmplen;

	switch(sqm_msg) {
		case MSG_PLAY:
			tmplen = strlen(g_chn_url);
			if(tmplen > MAX_SQM_URL_STR_LEN){
				PRINTF("ERROR: wrong channel url! len = %d\n", tmplen);
				return -1;
			}

			if (g_chn_stat == UNICAST_CHANNEL_TYPE || g_chn_stat == MULTICAST_CHANNEL_TYPE){

				sqm_msg_c26->MediaType = g_chn_stat;
				sqm_msg_c26->ChannelIp = g_chn_ip;
				sqm_msg_c26->ChannelPort = g_chn_port;
				sqm_msg_c26->StbPort = g_local_chn_port;
				strcpy(sqm_msg_c26->ChannelURL, g_chn_url);

				PRINTF( "IP:%d.%d.%d.%d@@PORT:%d",  (g_chn_ip >> 24) & 0xff, (g_chn_ip >> 16) & 0xff,(g_chn_ip >> 8) & 0xff,g_chn_ip & 0xff,g_chn_ip);
				PRINTF( "%d-%d-%d-%d-%s \n",sqm_msg_c26->MediaType,sqm_msg_c26->ChannelIp,sqm_msg_c26->ChannelPort,sqm_msg_c26->StbPort,sqm_msg_c26->ChannelURL);
			}
			break;
		case MSG_PAUSE:
			sqm_msg_c26->MediaType = g_chn_stat;
			break;
		case MSG_FAST:
			sqm_msg_c26->MediaType = g_chn_stat;
			break;
		case MSG_STOP:
			sqm_msg_c26->MediaType = g_chn_stat;
			break;
		case MSG_LOG_LEVEL:
			sqm_msg_c26->MediaType =  getModuleLevel("cuser");
		case MSG_GET_DATA:									//  1:SecData ????
			sqm_msg_c26->MediaType =  get_sqm_data_type;	// 0:StatData ????
			break;
		default:
			break;
		}

		return 0;
}

//  绑定消息类型
int sqm_port_buildmsg(SQM_MSG sqm_msg)
{
	SQM_MSG_C26 sqm_msg_c26={0};

	switch(sqm_msg) {
		case MSG_PLAY:
			sqm_create_msg(sqm_msg,&sqm_msg_c26);
			break;
		case MSG_PAUSE:
			sqm_create_msg(sqm_msg,&sqm_msg_c26);
			break;
		case MSG_FAST:
			sqm_create_msg(sqm_msg,&sqm_msg_c26);
			break;
		case MSG_STOP:
			sqm_create_msg(sqm_msg,&sqm_msg_c26);
			break;
		case MSG_LOG_LEVEL:
			sqm_create_msg(sqm_msg,&sqm_msg_c26);
			break;
		case MSG_GET_DATA:
			sqm_create_msg(sqm_msg,&sqm_msg_c26);
			break;
		default:
			break;
	}

	int ret = sqm_port_pushmsg(0, &sqm_msg_c26, sizeof(sqm_msg_c26), sqm_msg);

	PRINTF("send to sqmpro result:%d\n", ret);
	return ret;
}

int sqm_port_pushmsg_open(void)
{
	int ret = 0;
	int buf_size = MAX_BUFF_SIZE;

	PRINTF("sqm_port_pushmsg_open\n");
	if(pushmsg_sockfd > 0){
		close(pushmsg_sockfd);
	}

	pushmsg_sockfd = -1;
	memset(&msg_servaddr, 0, sizeof(msg_servaddr));
	memset(&msg_cliaddr, 0, sizeof(msg_cliaddr));
	pushmsg_sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if(pushmsg_sockfd < 0){
		PRINTF("create pushdata_sockfd error!\n");
		return -1;
	}
	if (-1 == fcntl(pushmsg_sockfd, F_SETFL, O_NONBLOCK)){
        PRINTF("set pushdata_sockfd nonblock error!\n");

		close(pushmsg_sockfd);
		pushmsg_sockfd = -1;

		return -1;
	}
	//设置socket发送缓存,buf_size 取probe_external_api.h中的MAX_BUFF_SIZE.
    if (-1 == setsockopt(pushmsg_sockfd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size))){
    	PRINTF("set pushdata_sockfd buffer size error!\n");

		close(pushmsg_sockfd);
		pushmsg_sockfd = -1;

		return -1;
    }
//	ret = setsockopt(pushmsg_sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
//	PRINTF("setsockopt=%d\n", ret);
	ret = unlink(SQM_SIG_TX);
	PRINTF("unlink=%d, errno = %d[%s]\n", ret, errno, strerror(errno));
       bzero(&msg_cliaddr, sizeof(msg_cliaddr));
       msg_cliaddr.sun_family = AF_LOCAL;
       strcpy(msg_cliaddr.sun_path, SQM_SIG_TX);	/* bind an address for us */

	if(-1 == bind(pushmsg_sockfd, (struct sockaddr *) &msg_cliaddr, sizeof(msg_cliaddr)))
	{
		PRINTF("bind pushmsg_sockfd error!, errno = %d[%s]\n", errno, strerror(errno));

		close(pushmsg_sockfd);
		pushmsg_sockfd = -1;
		unlink(SQM_SIG_TX);

		return -1;
	}

    bzero(&msg_servaddr, sizeof(msg_servaddr));
    msg_servaddr.sun_family = AF_LOCAL;
    strcpy(msg_servaddr.sun_path, SQM_SIG_RX); 	/* fill in server's address */

	//sqm_port_pushdmsg_clear();

	return pushmsg_sockfd;
}

//int sqm_port_pushmsg(int pIndex, char *buf, int len, SQM_MSG sqm_msg)
static int sqm_port_pushmsg(int pIndex, SQM_MSG_C26 *sqm_msg_c26, int len, SQM_MSG sqm_msg)
{
	//printf("============sqm_port_pushmsg  buf =%s, len =%d, ========\n", buf, len);

    STB_SQM_MSG stb_sqm_msg = {0};
	int send_len = 0;
	int ret = 0;

	if(pushmsg_sockfd <= 0){
		ERR_OUT("[socket error!]\n");
	}

	if(NULL == sqm_msg_c26 || len > MAX_PKT_SIZE ){
		ERR_OUT("[data error!], len = %d\n", len);
	}

	memset(&stb_sqm_msg, 0, sizeof(stb_sqm_msg));
	stb_sqm_msg.Var.MediaType = sqm_msg_c26->MediaType;
	stb_sqm_msg.Var.ChannelIp = sqm_msg_c26->ChannelIp;
	stb_sqm_msg.Var.ChannelPort = sqm_msg_c26->ChannelPort;
	stb_sqm_msg.Var.StbPort = sqm_msg_c26->StbPort;
	strcpy(stb_sqm_msg.Var.ChannelURL, sqm_msg_c26->ChannelURL);

	if (sqm_msg == MSG_STOP)
		stb_sqm_msg.command_id = sqm_msg;
	else
		stb_sqm_msg.command_id = sqm_msg - 4;
	stb_sqm_msg.command_length = len + 4;
	send_len = len + 4;

	PRINTF("\n");
	PRINTF("WILL send to SQMPRO: stb_sqm_msg.command_id = %d, stb_sqm_msg.command_length = %d\n",\
		stb_sqm_msg.command_id,stb_sqm_msg.command_length);
	PRINTF("MediaType  or loglevel=%d-ChannelIp=%d-ChannelPort=%d-StbPort=%d-ChannelURL=%s \n",\
		sqm_msg_c26->MediaType,sqm_msg_c26->ChannelIp,sqm_msg_c26->ChannelPort,\
		sqm_msg_c26->StbPort,sqm_msg_c26->ChannelURL);
	PRINTF("\n");

	ret = sendto(pushmsg_sockfd, &stb_sqm_msg, send_len, 0, (struct sockaddr *)&msg_servaddr, sizeof(msg_servaddr));
	PRINTF("ret = %d, errno=%d[%s]\n",  ret, errno, strerror(errno));
	return ret;

Err:
	return -1;
}
int sqm_port_getmsg( void *GetStbData,  int size, int data_type )
{

	int recv_len = 0;
	SQM_MSG_GET  *data_get = NULL;
	char recv_buf[1024] = {0};

	//PRINTF("size = %d, data_type =%d\n", size ,data_type);
	if(data_type < 0 || data_type > 1)
		ERR_OUT("[data_type error!], data_type = %d\n", data_type);
    get_sqm_data_type = data_type;

	if(size > (int)sizeof(SQM_MSG_GET))
		ERR_OUT("[size error!], size = %d\n", size);

	sqm_port_buildmsg(MSG_GET_DATA);

	memset(GetStbData, 0, size);
	recv_len = recvfrom(pushmsg_sockfd, recv_buf, sizeof(recv_buf), 0,NULL, NULL);
	PRINTF("recv_len=%d\n", recv_len);

	if (recv_len > 0 && recv_len <= 64) {
		data_get =  (SQM_MSG_GET *)(recv_buf) ;

		PRINTF("data_get->result =%d\n", data_get->result );
		PRINTF("00::df-mlr-jitter-index1-index2:::%d-%d-%d-%d-%d	\n",  data_get->StbData[0].df,  data_get->StbData[0].mlr,  data_get->StbData[0].jitter,  data_get->StbData[0].OtherIndex1,  data_get->StbData[0].OtherIndex2);
		PRINTF("11::df-mlr-jitter-index1-index2::::%d-%d-%d-%d-%d	\n", data_get->StbData[1].df,  data_get->StbData[1].mlr,  data_get->StbData[1].jitter,  data_get->StbData[1].OtherIndex1,  data_get->StbData[1].OtherIndex2);
		PRINTF("22::df-mlr-jitter-index1-index2:::%d-%d-%d-%d-%d	\n",  data_get->StbData[2].df,  data_get->StbData[2].mlr, data_get->StbData[2].jitter,  data_get->StbData[2].OtherIndex1,  data_get->StbData[2].OtherIndex2);
		PRINTF("--------------------------------------------------------------\n");
		memcpy(GetStbData, data_get, size);
	}
	else
		ERR_OUT("[recv from sqmpro error!], recv_len = %d\n", recv_len);


	return 0;

Err:
	return -1;

}


int sqm_port_getdata(int data_type) { return 0; }
int getSqmDataMdiMLR(void) { return 0; }
int getSqmDataMdiDF(void) { return 0; }
int getSqmDataJitter(void) { return 0; }
int getSqmDataBadDuration(void) { return 0; }
int getSqmDataPlayduration(void) { return 0; }
int getSqmDataAvailability(void) { return 0; }
int getSqmDataVideoQuality(void) { return 0; }


/************App_sys.c***********/
static void urlCheckSum1(char *buf)
{
    char *p = NULL;
    unsigned int sum = 0;

    p = buf;
    sum = 0;
    while(*p != '\0')
        sum += (unsigned int) * p++;
    sum += 1;
    sprintf(p, "%d", sum);
    return;
}

int sqm_find_file(void)
{
    DIR *dir = NULL;
    struct dirent *dp = NULL;

    if((dir = opendir(CONFIG_FILE_DIR)) != NULL) {
        while((dp = readdir(dir)) != NULL) {
            if((!strcmp(dp->d_name , "sqmpro"))) {
                sqmpro_flag = 1;
            }

            if(sqmpro_flag)
                break;
        }
    } else {
        LogSafeOperError("opendir error!\n");
        closedir(dir);
        return -1;
    }
    closedir(dir);
    if(!sqmpro_flag)
        system("rm "CONFIG_FILE_DIR"/sqmpro");

    return (sqmpro_flag);
}

void * sqmpro_copy(void *)
{
#if defined(INCLUDE_LITTLESYSTEM)
    system("cp /app1/bin/sqmpro  /root"); //TODO android
#else
    system("cp /home/hybroad/bin/sqmpro  /root"); //TODO android
#endif
    return NULL;
}

int sqm_file_check(void)
{
    LogSafeOperDebug("sqm_file_check\n");
    pthread_t pHandle_sqm;
    int find = sqm_find_file();

    if((-1 != find) && (!find)) {
        if(!sqmpro_flag)
            pthread_create(&pHandle_sqm, NULL, sqmpro_copy, 0);
        return 1;
    } else
        return 0;
}

int sqm_info_write(char *sqm_info , int sqminfo_len, char *writePath)
{
    FILE *sqm_write = NULL;

    if(!sqm_info)
        return 0;
    LogSafeOperDebug("Sqm_info = %s, sqminfo_len = %d\n", sqm_info, sqminfo_len);
    mode_t oldMask = umask(0077);
    sqm_write = fopen(writePath, "ab");
    umask(oldMask);
    if(!sqm_write) {
        LogSafeOperError("fopen error!\n");
        return -1;
    }

    fwrite(sqm_info, 1, sqminfo_len, sqm_write);

    fclose(sqm_write);
    return 0;
}

void getStbSqmHardwareType(char* type, int size)
{
    snprintf(type, size, "%s", Hybroad_getHWtype());
    return;
}

int sqm_get_epg_ip(char  *return_ip, int ip_len)
{
    int epg_len = 0;
    char epg_ip[URL_MAX_LEN] = {0}, epg_temp[URL_MAX_LEN] = {0}, *pstr = NULL, *pstr1 = NULL;

    if(ip_len > URL_MAX_LEN) {
        LogSafeOperError("epg_ip  is too lenght\n");
        goto Err;
    }


    snprintf(epg_temp, sizeof(epg_temp), "%s", Hippo::Customer().AuthInfo().AvailableEpgUrl().c_str());
    if(strlen(epg_temp)) {
        pstr = strchr(epg_temp, '/');
        if(pstr == NULL) {
            LogSafeOperError("epg_ip format error: epg_ip =%s\n", epg_temp);
            goto Err;
        }

        while(*pstr == '/') {
            pstr++;
        }
        if(pstr == NULL) {
            LogSafeOperError("get epg_ip error: epg_ip =%s\n", epg_temp);
            goto Err;
        }

        pstr1 = strchr(pstr, ':');
        if(pstr1 == NULL) {
            LogSafeOperError("get epg_ip error: epg_ip =%s\n", epg_temp);
            goto Err;
        }

        epg_len = pstr1 - pstr;
        if(epg_len < 0 || epg_len > ip_len)  {
            LogSafeOperError("epg_len error: epg_len =%d\n", epg_temp);
            goto Err;
        } else {
            IND_MEMCPY(epg_ip, pstr, epg_len);
            epg_ip[epg_len] = '\0';
            IND_STRNCPY(return_ip, epg_ip, ip_len);
        }
    } else {
        LogSafeOperError("get epg_ip error: epg_ip =%s\n", epg_temp);
        goto Err;
    }

    LogSafeOperDebug(" get EPG IP:: %s\n", return_ip);
    return 0;
Err:
    return -1;
}

int sqm_info_copy(char *writePath)
{
    FILE *sqm_open = NULL;
    YX_sqm_ini sqm_file;
    int ret, port;
    char info_temp[URL_MAX_LEN] = {0}, buf_info[URL_MAX_LEN] = {0}, url[URL_MAX_LEN + 4], *mid_info = NULL;
    char ip[64 + 4] = { 0 }; //TODO android

    LogSafeOperDebug("sqm.ini info write to %s \n", writePath);
    //串号
    memset(&sqm_file, 0, sizeof(YX_sqm_ini));
    mid_sys_serial(sqm_file.StbId);
    //sqm_file->StbId[32] = '\0';
    sprintf(info_temp, "StbId=%s\n", sqm_file.StbId);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //userid
    appSettingGetString("ntvuser", sqm_file.UserId, 33, 0);
    sprintf(info_temp, "UserId=%s\n", sqm_file.UserId);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    // mac 地址
    network_tokenmac_get(sqm_file.MacAddress, 20, ':');
    sprintf(info_temp, "MacAddress=%s\n", sqm_file.MacAddress);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //获取 IP

    sysSettingGetInt("connecttype", &ret, 0);
    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    strcpy(sqm_file.StbIP, network_address_get(ifname, ifaddr, URL_LEN));
    sprintf(info_temp, "StbIP=%s\n", sqm_file.StbIP);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //获取机顶盒网卡
    if(ret == 1)
        IND_STRCPY(sqm_file.StbNetName, "ppp0");
    else
        IND_STRCPY(sqm_file.StbNetName, sys_get_net_interface());

    sprintf(info_temp, "StbNetName=%s\n", sqm_file.StbNetName);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //获取组播接收网卡
    if(ret == 1) {
		int pppdMulticast = 0;
        sysSettingGetInt("pppdmulticast", &pppdMulticast, 0);
        if (pppdMulticast) //带PPP头, 组播纠错前从ppp0网卡接收数据
            IND_STRCPY(sqm_file.StbMulticastNetName, "ppp0");
        else //不带PPP头, 组播纠错前从对应网卡接收数据
            IND_STRCPY(sqm_file.StbMulticastNetName, sys_get_net_interface());
    }
    else
        IND_STRCPY(sqm_file.StbMulticastNetName, sqm_file.StbNetName);
    sprintf(info_temp, "StbMulticastNetName=%s\n", sqm_file.StbMulticastNetName);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //获取PPPOE 账号
    sysSettingGetString("netuser", sqm_file.PppoeAccount, 32, 0);
    sprintf(info_temp, "PppoeAccount=%s\n", sqm_file.PppoeAccount);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //epgip
    if(sqm_get_epg_ip(sqm_file.EpgIP, sizeof(sqm_file.EpgIP)))
        IND_STRCPY(sqm_file.EpgIP, "0.0.0.0");
    sprintf(info_temp, "EpgIP=%s\n", sqm_file.EpgIP);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //获取机顶盒软件版本号
    get_upgrade_version(sqm_file.StbSoftwareVersion);
    sprintf(info_temp, "StbSoftwareVersion=%s\n", sqm_file.StbSoftwareVersion);
    sqm_info_write(info_temp , strlen(info_temp), writePath);

    //获取机顶盒硬件型号
	getStbSqmHardwareType(sqm_file.StbHardwareType, sizeof(sqm_file.StbHardwareType));
    IND_STRCPY(sqm_file.StbHardwareType, sqm_file.StbHardwareType);
    sprintf(info_temp, "StbHardwareType=%s\n", sqm_file.StbHardwareType);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //获取MQMC 的IP 地址
#ifdef Sichuan //四川从配置文件读取mqmc地址, 其余局点使用内存中的mqmc地址.
    sysSettingGetString("mqmcIp", sqm_file.MqmcIP, sizeof(sqm_file.MqmcIP), 0);
#else
    sqm_port_mqmip_get(sqm_file.MqmcIP);
 #endif
    sprintf(info_temp, "MqmcIP=%s\n", sqm_file.MqmcIP);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //MQMC 端口, 默认使用37000
    sqm_file.MqmcSendPort = 37000;
    sprintf(info_temp, "MqmcSendPort=%d\n", sqm_file.MqmcSendPort);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //机顶盒探针的监听端口, 默认使用37001
    sqm_file.MqmcRecvPort = 37001;
    sprintf(info_temp, "MqmcRecvPort=%d\n", sqm_file.MqmcRecvPort);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //SQM 日志级别,默认读取log.c中的SQM打印设置
    sqm_file.SqmLogLevel = getModuleLevel("cuser");
    sprintf(info_temp, "SqmLogLevel=%d\n", sqm_file.SqmLogLevel);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
	// 获取nat 设备的ip/port, 此为对应的网关的IP,暂不用写.
    IND_STRCPY(sqm_file.StbNatIP, "");
    sqm_file.StbNatPort = 0;

    sprintf(info_temp, "StbNatIP=%s\n", sqm_file.StbNatIP);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //natport 待补 需修改  目前写死
    sprintf(info_temp, "StbNatPort=%d\n", sqm_file.StbNatPort);
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //upgrade IP
	sysSettingGetString("upgradeUrl", url, URL_MAX_LEN, 0);
    if(strlen(url) && strncmp(url, "http://", 7) == 0) {
        if(mid_tool_checkURL(url, ip, &port)) {
            IND_STRCPY(ip, "0.0.0.0");
            port = 0;
        }
    } else {
        IND_STRCPY(ip, "0.0.0.0");
        port = 0;
    }
    sprintf(sqm_file.UpdateUrl, "http://%s:%d/UPGRADE/jsp/upgrade.jsp?TYPE=%s&MAC=%s&USER=%s&VER=%s&\
CHECKSUM=", ip, port, sqm_file.StbHardwareType, sqm_file.MacAddress, sqm_file.UserId, sqm_file.StbSoftwareVersion);

    urlCheckSum1(sqm_file.UpdateUrl);
    strcat(sqm_file.UpdateUrl, "\n");
    sprintf(info_temp, "UpdateUrl=%s\n", sqm_file.UpdateUrl);
    sqm_info_write(info_temp, strlen(info_temp), writePath);

    //the following contents is writtern by sqmloader
    //机顶盒探针版本号
    IND_STRCPY(info_temp,"SqmVersion=\n");
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //MQMC 的IP地址, 预留
    IND_STRCPY(info_temp,"MqmcIPReg=\n");
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //MQMC的PORT  预留
    IND_STRCPY(info_temp,"MqmcPortReg=\n");
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //机顶盒探针升级服务器地址
    IND_STRCPY(info_temp,"SqmUpdateServerIP=\n");
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //机顶盒探针升级服务器端口
    IND_STRCPY(info_temp,"SqmUpdateServerPort=\n");
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //sqmloader 日志级别
    IND_STRCPY(info_temp,"SqmloaderLogLevel=\n");
    sqm_info_write(info_temp , strlen(info_temp), writePath);
    //sqmpro 日志级别
    IND_STRCPY(info_temp,"SqmproLogLevel=\n");
    sqm_info_write(info_temp , strlen(info_temp), writePath);

    return 1;

}


/**************************end of file****************************/

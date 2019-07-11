/*
 * filename  : sqm_port.c
 * create date: 2010.12.7
 *
 */
#include <errno.h>
#if (defined( SQM_VERSION_C21) || defined( SQM_VERSION_C22 ) || defined(SQM_VERSION_C23))

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/un.h>

#include "sqm_port.h"
#include "sqm_types.h"
#include "sys_basic_macro.h"
#include "AppSetting.h"
#include "SysSetting.h"

#include "mid_sys.h"
#include "log_sqm_port.h"
#include "mid/mid_mutex.h"
#include "config/pathConfig.h"

#define SQM_MSG_FIFO DEFAULT_PIPE_DATAPATH"/sqm_msg_fifo"
#define STB_PACK_CACHE_NUM 1024

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

#if (defined( SQM_VERSION_C22 ) || defined(SQM_VERSION_C23))
/*纠错状态从纠错后切换到纠错前，最大的判断值*/
static int pushdata_sockfd = -1;
static struct sockaddr_un servaddr = {0};
static struct sockaddr_un cliaddr = {0};
static SQM_DATA_CACHE g_pack_cache;
#endif

static void sqm_port_init();
static void sqm_port_start();
static void sqm_port_set_info();
static void sqm_port_stop();
static void sqm_cache_save(STB_TS_PACK *stb_pack);
static int sqm_cache_load_begin(STB_TS_PACK *stb_pack);
static void sqm_cache_load_end(void);
static void sqm_cache_clear(void);

int sqm_port_getStatus(void)
{
	return sqm_port_stat;
}

void sqm_port_setStatus(int status)
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

	pstr = strchr(psrc, '.');
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

// write init message to the sqm module
static void sqm_port_init()
{
    char pppoeuser[33] = {0};
	char tmpbuf[MAX_MSG_LEN];
	char tmpbuf1[16];                      // stop ip string: xxx.xxx.xxx.xxx
	char *pstr;
	char *pstr1;
	int tmplen, paralen;
	int ret;

	// after standby, the sqm_port_stat is SQM_SETCHANNELINFO_OK
	if((sqm_port_stat != SQM_READY) && (sqm_port_stat != SQM_STOP_OK)){
		PRINTF("ERROR: Not ready to init sqm module, now stat is %d\n", sqm_port_stat);
		return;
	}

	memset(msgbuf, 0, MAX_MSG_LEN);  // clear buffer first

	msgbuf[0] = SQM_CMD_INIT;        // sqm init message
	msgbuf[3] = SQM_LOG_LEVEL;       // log level

	ret = stripTOuint(&msgbuf[4], g_mqmip);  // mqmc ip : xxx.xxx.xxx.xxx
	if(ret == -1){
		PRINTF("get mqm ip error!\n");
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}

	//sys_user_get(tmpbuf);        //get user account, this is net account
	appSettingGetString("ntvuser", tmpbuf, MAX_MSG_LEN, 0);
	tmplen = strlen(tmpbuf);
	if(tmplen > 32){
		PRINTF("The user account is too long: %d\n", tmplen);
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}
	memcpy(&msgbuf[8], tmpbuf, tmplen);

    network_tokenmac_get(tmpbuf, MAX_MSG_LEN, ':');
	memcpy(&msgbuf[40], tmpbuf, 17);

	memset(tmpbuf, 0, sizeof(tmpbuf));
	mid_sys_serial(tmpbuf);       // get stb id
	tmplen = strlen(tmpbuf);
	if(tmplen > 32){
		PRINTF("The stb id is too long: %d\n", tmplen);
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}
	memcpy(&msgbuf[57], tmpbuf, tmplen);

	app_epgUrl_get(tmpbuf);       // get epg url, exp: http://10.10.10.39:33200/EPG/jsp
	pstr = strchr(tmpbuf, '/');
	if(pstr == NULL){
		PRINTF("Illegal ip string, can NOT find the seperater. --epg url\n");
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}
	while( *pstr == '/'){
		pstr++;
	}
	if(pstr == NULL){
		PRINTF("Illegal ip string, can NOT find the seperater. **epg url\n");
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}
	pstr1 = strchr(pstr, ':');
	if(pstr == NULL){
		PRINTF("Illegal ip string, can NOT find the seperater : in epg url\n");
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}
	tmplen = pstr1 - pstr;
	memcpy(tmpbuf1, pstr, tmplen);
	tmpbuf1[tmplen] = '\0';
	PRINTF("EPG IP is %s\n", tmpbuf1);
	ret = stripTOuint(&msgbuf[89], tmpbuf1);
	if(ret == -1){
		PRINTF("get epg ip error!\n");
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}

    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
	ret = stripTOuint(&msgbuf[93], network_address_get(ifname, ifaddr, URL_LEN));
	if(ret == -1){
		PRINTF("get local ip error\n");
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}
    int netType = 0;
	switch(mid_net_get_connecttype()){
		case 1: // pppoe
			msgbuf[97] = TYPE_PPPOE0;
			break;
		default: // dhcp or static ip
            sysSettingGetInt("nettype", &netType, 0);
			if(netType == 1){       // wireless
				msgbuf[97] = TYPE_ETH0_WI;
			}else{                            // wire
				msgbuf[97] = TYPE_ETH0;
			}
	}

	//wo have only one net card ,so multicast ip and net name is same
	int pppdMulticast = 0;
    sysSettingGetInt("pppdmulticast", &pppdMulticast, 0);
	if(!pppdMulticast && tmplen == 1){
		memset(tmpbuf, 0, MAX_MSG_LEN);
		mid_net_addr_eth0(tmpbuf);
		ret = stripTOuint(&msgbuf[98], tmpbuf);
		if(ret == -1){
			PRINTF("get local ip error\n");
	//		mid_mutex_unlock(g_sqm_mutex);
			return;
		}
		msgbuf[102] = TYPE_ETH0;
	}else{
		msgbuf[98]  = msgbuf[93];
		msgbuf[99]  = msgbuf[94];
		msgbuf[100] = msgbuf[95];
		msgbuf[101] = msgbuf[96];
		msgbuf[102] = msgbuf[97];
	}
     // Eagle add. 2011年01月19日
    msgbuf[103] = (g_listen_port >> 24) & 0xff;
    msgbuf[104] = (g_listen_port >> 16) & 0xff;
    msgbuf[105] = (g_listen_port >>  8) & 0xff;
    msgbuf[106] = g_listen_port & 0xff;

    msgbuf[107] = (g_server_port >> 24) & 0xff;
    msgbuf[108] = (g_server_port >> 16) & 0xff;
    msgbuf[109] = (g_server_port >>  8) & 0xff;
    msgbuf[110] = g_server_port & 0xff;


	sysSettingGetString("netuser", pppoeuser, 32, 0);
	tmplen = strlen(tmpbuf);
    if ( tmplen + 111 > MAX_MSG_LEN )
    {
		PRINTF("not enough space for net account\n");
        return ;
    }
    memcpy(&msgbuf[111], tmpbuf, tmplen);
	paralen = tmplen + 108;
	msgbuf[1] = (paralen >> 8) & 0xff;
	msgbuf[2] = paralen & 0xff;
	sqm_port_write(fd_write_to_sqm, msgbuf, paralen + 3);
	PRINTF("[SQM_PORT]: send init command to sqm module\n");
	sqm_port_stat = SQM_INIT_OK;
//	mid_mutex_unlock(g_sqm_mutex);
}

// write start message to the sqm module
static void sqm_port_start()
{
//	mid_mutex_lock(g_sqm_mutex);
	if(sqm_port_stat != SQM_INIT_OK){
		PRINTF("ERROR: start sqm monitor only at init_ok stat, now stat is %d\n", sqm_port_stat);
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}
	msgbuf[0] = SQM_CMD_START_MONITOR;
	msgbuf[1] = 0;
	msgbuf[2] = 0;
	sqm_port_write(fd_write_to_sqm, msgbuf, 3);

	PRINTF("[SQM_PORT]: send start command to sqm module\n");
	sqm_port_stat = SQM_START_OK;
//	mid_mutex_unlock(g_sqm_mutex);
}

// write set channel info message to the sqm module
static void sqm_port_set_info()
{
	int tmplen;
//	int len;

//	mid_mutex_lock(g_sqm_mutex);
	if((sqm_port_stat != SQM_START_OK) && (sqm_port_stat != SQM_SETCHANNELINFO_OK)){
		PRINTF("ERROR: need to start sqm monitor first, now stat is %d\n", sqm_port_stat);
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}

	msgbuf[0] = SQM_CMD_SETCHANNELINFO;
	msgbuf[3] = g_chn_stat;
	msgbuf[4] = (g_chn_ip >> 24) & 0xff;
	msgbuf[5] = (g_chn_ip >> 16) & 0xff;
	msgbuf[6] = (g_chn_ip >> 8) & 0xff;
	msgbuf[7] = g_chn_ip & 0xff;
	msgbuf[8] = (g_chn_port >> 24) & 0xff;
	msgbuf[9] = (g_chn_port >> 16) & 0xff;
	msgbuf[10] = (g_chn_port >> 8) & 0xff;
	msgbuf[11] = g_chn_port & 0xff;
	msgbuf[12] = (g_local_chn_port >> 24) & 0xff;
	msgbuf[13] = (g_local_chn_port >> 16) & 0xff;
	msgbuf[14] = (g_local_chn_port >> 8) & 0xff;
	msgbuf[15] = g_local_chn_port & 0xff;

	tmplen = strlen(g_chn_url);
	if(tmplen > MAX_SQM_URL_STR_LEN){
		PRINTF("ERROR: wrong channel url! len = %d\n", tmplen);
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}else{
		memcpy(&msgbuf[16], g_chn_url, tmplen);
	}
	tmplen += 13;
	msgbuf[1] = (tmplen >> 8) & 0xff;
	msgbuf[2] = tmplen & 0xff;
    sqm_port_write(fd_write_to_sqm, msgbuf, tmplen + 3);
    sqm_port_stat = SQM_SETCHANNELINFO_OK;
    PRINTF("[SQM_PORT]: send set channel info command to sqm module\n");
/*
	PRINTF("[SQM_PORT]: the command para is: ");
	for(len = 3; len < tmplen + 3; len++){
		PRINTF("%#x, ", msgbuf[tmplen]);
	}
	PRINTF("\n");
*/
//	mid_mutex_unlock(g_sqm_mutex);
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
//	mid_mutex_lock(g_sqm_mutex);
	if(sqm_port_stat == SQM_STOP_OK){
		PRINTF("ERROR: sqm already stop, now stat is %d\n", sqm_port_stat);
//		mid_mutex_unlock(g_sqm_mutex);
		return;
	}
	msgbuf[0] = SQM_CMD_STOP_MONITOR;
	msgbuf[1] = 0;
	msgbuf[2] = 0;
	sqm_port_write(fd_write_to_sqm, msgbuf, 3);

	PRINTF("[SQM_PORT]: send stop command to sqm module\n");
	sqm_port_stat = SQM_STOP_OK;
//	mid_mutex_unlock(g_sqm_mutex);
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
					case MSG_INIT:
						sqm_port_init();
						break;
					case MSG_START:
						sleep(1);         // wait 2s to start
						sqm_port_start();
						break;
					case MSG_SETINFO:
						sqm_port_set_info();
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

    if ( pthread_create( &print_thread_handle, NULL, (void *)print_task, NULL ) )
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

    if(pthread_create( &sqm_thread_handle, NULL, (void*)sqm_task, NULL )){
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


/* Eagle. add. 2011年01月19日 */
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

    while ( 1 )
    {
        FD_ZERO ( &rset );
        FD_SET ( fd_info_r, &rset );

        ret = select ( fd_info_r+1, &rset, NULL, NULL, NULL );
        if ( ret > 0 && FD_ISSET( fd_info_r, &rset ) )
        {
            char r_buf[256] = { 0 };

            if ( read ( fd_info_r, r_buf, 256 ) < 0 )
            {
                continue;
            }
            PRINTF( r_buf );
        }
    }
    close( fd_info_r );
    return ;
}
/* -----  end of function print_task  ----- */

#if (defined( SQM_VERSION_C22 ) || defined(SQM_VERSION_C23))
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
	int ret = 0, err = 0, num = 0;

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
	int num = 0, len = 0;

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
#endif

#endif                 // SQM_VERSION_C21

/**************************end of file****************************/


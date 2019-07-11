
#include "StatisticCTCSichuan.h"

#include "../../../../../../net_manager/common/nm_common.h"

#include "IPTVMiddleware.h"
#include "TR069Assertions.h"

#include "sqm_port.h"
#include "app_tr069_alarm.h"

#include "mid/mid_ftp.h"

#include "curl.h"

#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define CONFIG_LEN          (32 * 1024)

#define BUFFER_HEAD_SIZE    2048
#define BUFFER_TAIL_SIZE    32

#include "SysSetting.h"
#define IPTVMiddleware_SettingGetStr(name, value, len)  sysSettingGetString(name, value, len, 0)


StatisticCTCSichuan::StatisticCTCSichuan(void) : m_Enable(0),
    m_LogUploadInterval(0),
    m_LogRecordInterval(0)
{
    m_headBuf = (char*)malloc(BUFFER_HEAD_SIZE);
    m_headLen = 0;
    m_tailBuf = (char*)malloc(BUFFER_TAIL_SIZE);
    m_tailLen = 0;

    m_LogServerUrl[0] = 0;
    pthread_mutex_init(&m_prmMutex, NULL);

    m_cfgTree = ind_cfg_create( );
}

StatisticCTCSichuan::~StatisticCTCSichuan(void)
{
}

void StatisticCTCSichuan::statisticConfigRead(char *rootname)
{
#if 0
    int len = 0;
    FILE *fp = NULL;
    char filename[64];

    char *buf = (char *)malloc(CONFIG_LEN);
    if(!buf) {
        //LogTr069Error("malloc :: memory");
        return;
    }

    len = strlen(rootname);
    if(len > 32) {
        //LogTr069Error("%s too large\n", rootname);
        goto End;
    }

    sprintf(filename, "/data/yx_config_%s.ini", rootname);

    fp = fopen(filename, "rb");
    if(NULL == fp) {
        //LogTr069Error("Can not open file [%s]\n", filename);
        goto End;
    }

    len = fread(buf, 1, CONFIG_LEN, fp);
    fclose(fp);
    if(len <= 0 || len >= CONFIG_LEN) {
        //LogTr069Error("len = %d, CONFIG_LEN = %d\n", len, CONFIG_LEN);
        goto End;
    }
    buf[len] = 0;

    pthread_mutex_lock(&m_prmMutex);
    ind_cfg_input(m_cfgTree, rootname, buf, len);
    pthread_mutex_unlock(&m_prmMutex);

End:
    free(buf);
#endif
}

void StatisticCTCSichuan::statisticConfigWrite(char *rootname)
{
#if 0
    int len = 0;
    FILE *fp = NULL;
    char filename[64];

    len = strlen(rootname);
    if(len > 32) {
        LogTr069Error("%s too large\n", rootname);
        return;
    }

    char *buf = (char *)malloc(CONFIG_LEN);
    if(buf == NULL) {
        LogTr069Error("malloc :: memory");
        return;
    }

    pthread_mutex_lock(&m_prmMutex);
    len = ind_cfg_output(m_cfgTree, rootname, buf, CONFIG_LEN);
    pthread_mutex_unlock(&m_prmMutex);

    sprintf(filename, "/data/yx_config_%s.ini", rootname);
    if(len > 0) {
        mode_t oldMask = umask(0077);
        fp = fopen(filename, "wb");
        umask(oldMask);
        if (fp) {
            fwrite(buf, 1, len, fp);
            fclose(fp);
            sync();
        } else {
            LogTr069Error("fopen = %s\n", filename);
        }
    } else {
        LogTr069Error("tree_cfg_input = %d\n", len);
    }

    free(buf);
#endif
}

void changeDataFmt(int a, char *fmt)
{
    if(a == 0)
        sprintf(fmt, "%s", ",");
    else
        sprintf(fmt, "%d", a);
}

int StatisticCTCSichuan::statisticTime2str(char *str, int size, const char *prefix, const char *suffix, unsigned int sec)
{
    time_t t;
    struct tm tm;

    memset(&tm, 0, sizeof(tm));
    t = sec;
    localtime_r(&t, &tm);
    return snprintf(str, size, "%s%04d%02d%02d%02d%02d%02d%s", 
        prefix, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 
        tm.tm_hour, tm.tm_min, tm.tm_sec, suffix);
}

void StatisticCTCSichuan::CreateUploadHead(void)
{
    char buf[1024];

    int len = 0;
    /*-- IPTV终端信息（文件头部）--*/
    buf[0] = 0;
    IPTVMiddleware_SettingGetStr("STBSerialNumber", buf, 1024);

#define BUFFER_HEAD_SIZE    2048
#define BUFFER_TAIL_SIZE    32

    len += snprintf(m_headBuf + len, BUFFER_HEAD_SIZE - len, "STBID,%s\n", buf);
    len += snprintf(m_headBuf + len, BUFFER_HEAD_SIZE - len, "OUI,%s\n", "990060");
    len += snprintf(m_headBuf + len, BUFFER_HEAD_SIZE - len, "Manufacturer,Huawei\n");

    buf[0] = 0;
    IPTVMiddleware_SettingGetStr("STBType", buf, 1024);
    len += snprintf(m_headBuf + len, BUFFER_HEAD_SIZE - len, "productClass,%s\n", buf);

    buf[0] = 0;
    IPTVMiddleware_SettingGetStr("SoftwareHWVersion", buf, 1024);
    len += snprintf(m_headBuf + len, BUFFER_HEAD_SIZE - len, "SoftwareVersion,%s\n",buf);

    buf[0] = 0;
    IPTVMiddleware_SettingGetStr("HardWareVersion", buf, 1024);

    len += snprintf(m_headBuf + len, BUFFER_HEAD_SIZE - len, "HardwareVersion,%s\n", buf);
    buf[0] = 0;
    IPTVMiddleware_SettingGetStr("TokenMACAddress", buf, 1024);
    len += snprintf(m_headBuf + len, BUFFER_HEAD_SIZE - len, "MACAddress,%s\n", buf);

    buf[0] = 0;
    IPTVMiddleware_SettingGetStr("vlanip", buf, 1024);
    len += snprintf(m_headBuf + len, BUFFER_HEAD_SIZE - len, "IPAddress,%s\n", buf);

    buf[0] = 0;
    IPTVMiddleware_SettingGetStr("ntvuser", buf, 1024);
    len += snprintf(m_headBuf + len, BUFFER_HEAD_SIZE - len, "UserID,%s\n", buf);

    buf[0] = 0;
    IPTVMiddleware_SettingGetStr("eds", buf, 1024);
    len += snprintf(m_headBuf + len, BUFFER_HEAD_SIZE - len, "AuthURL,%s\n", buf);

    {
        unsigned nowTime, beginTime;
        nowTime = time(NULL);
        beginTime = nowTime - m_LogUploadInterval;

        len += statisticTime2str(m_headBuf + len, BUFFER_HEAD_SIZE - len, "Startpoint,", "\n", beginTime);
        len += statisticTime2str(m_headBuf + len, BUFFER_HEAD_SIZE - len, "Endpoint", "\n", nowTime);
    }
    len += snprintf(m_headBuf + len, BUFFER_HEAD_SIZE - len, "********\n");

    m_headLen = len;

   m_tailLen = snprintf(m_tailBuf, BUFFER_TAIL_SIZE, "<EndOfStbFile>\n");
}

static int changeMacAddress(char *mac, char *newMac)
{
  	int i = 0, j = 0;

	while(mac[i]){
           if(mac[i] != ':')
               newMac[j++] = mac[i];
	    i++;
	}
	return j;
}

void StatisticCTCSichuan::CreateUploadName(void)
{
    int len;
    char *p, *fileName;
    char buffer[64];
    unsigned int addr;

    fileName = m_fileName;

    buffer[0] = 0;
    IPTVMiddleware_SettingGetStr("TokenMACAddress",buffer, 32);
    len = changeMacAddress(buffer, fileName);
    len += sprintf(fileName + len, "_");

    buffer[0] = 0;
    IPTVMiddleware_SettingGetStr("ntvuser", buffer, 64);
    p = buffer;
    while (*p++) {
        if (*p == '@')
            *p = '_';
    }
    len += sprintf(fileName + len, "%s", buffer);

    if(fileName[len - 1] != '_')
        len += sprintf(fileName + len, "_");

    buffer[0] = 0;
    IPTVMiddleware_SettingGetStr("vlanip", buffer, 32);
    addr = inet_addr(buffer);
    len += sprintf(fileName + len, "%03d-%03d-%03d-%03d_", addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff);

    {
        time_t beginTime;
        struct tm tm;

        beginTime = time(NULL) - m_LogUploadInterval;
        memset(&tm, 0, sizeof(tm));
        localtime_r(&beginTime, &tm);
        len += sprintf(fileName + len, "%02d%02d%02d%02d%02d_", 
            (tm.tm_year + 1900) % 100, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
    }

    sprintf(fileName + len, "%06d_%c.csv", m_LogUploadInterval, m_typeChar);
    LogTr069Debug("fileName = %s\n", fileName);
}

typedef struct {
    int offset;

    char* headBuf;
    int   headLen;
    char* bodyBuf;
    int   bodyLen;
    char* tailBuf;
    int   tailLen;
} LoadBuffer;

static size_t readfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
    char *buf;
    int off, len, space;
    LoadBuffer *lb = (LoadBuffer *)stream;

    len = nmemb;

    if (lb->offset >= lb->headLen + lb->bodyLen) {
        off = lb->offset - (lb->headLen + lb->bodyLen);
        space = lb->tailLen - off;
        buf = lb->tailBuf;
    } else if (lb->offset >= lb->headLen) {
        off = lb->offset - lb->headLen;
        space = lb->bodyLen - off;
        buf = lb->bodyBuf;
    } else {
        off = lb->offset;
        space = lb->headLen - off;
        buf = lb->headBuf;
    }

    if (len > space)
        len = space;
    if (len > 0) {
        memcpy(ptr, buf, len);
        lb->offset += len;
    }

    return len;
}

int StatisticCTCSichuan::UploadFile(void)
{
    char url[512];
    CURL *curl;
    CURLcode r;
    LoadBuffer lb;
    int len;

    strcpy(url, m_LogServerUrl);
    len = strlen(url);
    if (len <= 0)
        return -1;
    if (url[len - 1] == '/')
        snprintf(url + len, 512 - len, "%s", m_fileName);
    else
        snprintf(url + len, 512 - len, "/%s", m_fileName);

    LogTr069Debug("url = %s\n", url);

    //curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init( );

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    curl_easy_setopt(curl, CURLOPT_URL, url); 
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);
    curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 0);

    lb.headBuf = m_headBuf;
    lb.headLen = m_headLen;
    lb.bodyBuf = m_bodyBuf;
    lb.bodyLen = m_bodyLen;
    lb.tailBuf = m_tailBuf;
    lb.tailLen = m_tailLen;

    lb.offset = 0;

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, readfunc);
    curl_easy_setopt(curl, CURLOPT_READDATA, &lb);

    r = curl_easy_perform(curl);
    if (r != CURLE_OK)
        LogTr069Error("%s\n", curl_easy_strerror(r));

    curl_easy_cleanup(curl);

    LogTr069Debug("Upload Succeed!\n");

    return 0;
}

void StatisticCTCSichuan::StatisticRecord(void)
{
    AppendStatisticBody();

    if (m_LogUploadMultiple > 0) {
        m_LogUploadCounter++;
        if (m_LogUploadCounter >= m_LogUploadMultiple) {
            StatisticUpload( );
            m_LogUploadCounter = 0;
        }
    }
}

void StatisticCTCSichuan::StatisticUpload(void)
{
    CreateUploadName( );
    CreateUploadHead( );

    UploadFile( );

    m_bodyLen = 0;
}

void StatisticCTCSichuan::StatisticRefresh(void)
{
    m_bodyLen = 0;

    UnRegistUpload( );
    UnRegistRecord( );

    if (m_LogUploadInterval <= 0 || m_LogRecordInterval <= 0)
        return;
    if (0 == m_LogUploadInterval % m_LogRecordInterval)
        m_LogUploadMultiple = m_LogUploadInterval / m_LogRecordInterval;
    else
        m_LogUploadMultiple = 0;
    m_LogUploadCounter = 0;

    if (m_Enable > 0 && m_LogUploadInterval > 0 && m_LogRecordInterval > 0) {
        RegistRecord( );
        if (0 == m_LogUploadMultiple)
            RegistUpload( );
    }
}

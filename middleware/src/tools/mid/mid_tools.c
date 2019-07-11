

#include "app/Assertions.h"

#include "dns/mid_dns.h"

#include "mid_task.h"
#include "mid_tools.h"

#include "independs/ind_tmr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define MEM_LINE_SIZE 16
#define STRING_LINE_MAX	80
#define LINE_SIZE 80


void mid_tool_mem_line(unsigned char *p)
{
    int i = 0, tIndex = 0;
    unsigned char ch, str[256] = {0};

    snprintf((char *)str, 256, "%p", p);

    for(i = 0; i < MEM_LINE_SIZE; i ++) {
        tIndex = strlen((char *)str);
        if((i % 4) == 0)
            str[tIndex ++] = ' ';
        snprintf((char *)(str + tIndex), (256 - tIndex), "%02x", p[i]);
    }

    tIndex = strlen((char *)str);
    str[tIndex ++] = ':';
    str[tIndex ++] = ' ';
    for(i = 0; i < MEM_LINE_SIZE; i ++) {
        ch = p[i];
        if(ch < 33 || ch > 126)
            str[tIndex] = '.';
        else
            str[tIndex] = ch;
        tIndex ++;
    }
    str[tIndex] = 0;
    LogRunOperDebug("%s\n", str);
    return;
}

void mid_tool_mem_show(unsigned char *p, int len)
{
    int i = 0, n = 0, l = 0;
    unsigned char buf[MEM_LINE_SIZE] = {0};

    if(len < 0)
        ERR_PRN("len = %d\n", len);
    n = len / MEM_LINE_SIZE;
    l = len % MEM_LINE_SIZE;

    for(i = 0; i < n; i ++) {
        mid_tool_mem_line(p);
        p += MEM_LINE_SIZE;
    }

    if(l == 0)
        return;

    memcpy(buf, p, l);
    for(i = l; i < MEM_LINE_SIZE; i ++)
        buf[i] = 0;
    mid_tool_mem_line(buf);
    mid_task_delay(10);
    return;
}

void mid_tool_string_show(unsigned char *str)
{
    int i, n, l;
    unsigned char buf[STRING_LINE_MAX + 4];

    l = strlen((char *)str);
    n = l / STRING_LINE_MAX;
    l = l % STRING_LINE_MAX;

    buf[STRING_LINE_MAX] = 0;
    for(i = 0; i < n; i ++) {
        memcpy(buf, str, STRING_LINE_MAX);
        PRINTF("%s\n", buf);
        //mid_task_delay(100);
        str += STRING_LINE_MAX;
    }
    if(l == 0)
        return;

    memcpy(buf, str, l);
    buf[l] = 0;
    PRINTF("%s\n", buf);
}

/*
U-00000000 - U-0000007F:  0xxxxxxx
U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 */
void mid_tool_string_cutcpy(char *buf, char *str, int max)
{
    int len = 0;

    if(strlen(str) <= max) {
        strncpy(buf, str, max);
        return;
    }

    while(len < max) {
        if((str[len] & 0x80) == 0) {//0xxxxxxx
            len ++;
            continue;
        }
        if((str[len] & 0xE0) == 0xC0) {//110xxxxx 10xxxxxx
            if(len + 2 > max)
                break;
            len += 2;
            continue;
        }
        if((str[len] & 0xF0) == 0xE0) {//1110xxxx 10xxxxxx 10xxxxxx
            if(len + 3 > max)
                break;
            len += 3;
            continue;
        }
        if((str[len] & 0xF8) == 0xF0) {//11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            if(len + 4 > max)
                break;
            len += 4;
            continue;
        }
        if((str[len] & 0xFC) == 0xF8) {//111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            if(len + 5 > max)
                break;
            len += 5;
            continue;
        }
        if((str[len] & 0xFE) == 0xFC) {//1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            if(len + 6 > max)
                break;
            len += 6;
            continue;
        }
        break;
    }
    memcpy(buf, str, len);
    memcpy(buf + len, "...", 3);
    buf[len + 3] = 0;
    return;
}

void mid_tool_lines_show(char *str, int delay)
{
    char *p = NULL, *p0 = NULL;
    int len = 0;
    char buf[STRING_LINE_MAX + 2] = {0};

    p0 = str;
    p = strchr(p0, '\n');
    while(p) {
        len = (int)(p - p0);
        if(len > STRING_LINE_MAX)
            len = STRING_LINE_MAX;
        memcpy(buf, p0, len);
        buf[len] = 0;
        LogRunOperDebug("%s\n", buf);
        if(delay > 0)
            mid_task_delay(delay);
        p0 = p + 1;
        p = strchr(p0, '\n');
    }
    if(*p0 != 0) {
        len = strlen(p0);
        if(len > STRING_LINE_MAX)
            len = STRING_LINE_MAX;
        memcpy(buf, p0, len);
        buf[len] = 0;
        LogRunOperDebug("%s\n", p0);
    }
    return;
}

void mid_tool_line_first(char *string, char *buf)
{
    int i = 0;
    char ch;

    do {
        ch = string[i];
        if(ch == '\r' || ch == '\n' || ch == '\0')
            break;
        buf[i] = ch;
        i ++;
    } while(1);
    buf[i] = '\0';
    return;
}

int mid_tool_line_len(char *string)
{
    int i = 0;

    do {
        char ch = string[i];

        if(ch == '\r' || ch == '\n' || ch == '\0')
            break;
        i ++;
    } while(1);

    return i;
}

int mid_tool_line_print(char *line)
{
    char buf[LINE_SIZE + 1] = {0};
    int len = 0;

    len = mid_tool_line_len(line);
    if(len <= 0)
        return -1;

    if(len > LINE_SIZE)
        len = LINE_SIZE;
    strncpy(buf, line, len);

    LogRunOperDebug("%s\n", buf);
    return 0;
}

unsigned int mid_tool_string2time(char* str)
{
    struct ind_time t;

    if(str == NULL || strlen(str) < 14)
        return 0;
    if(str[4] == ':') {
        if(sscanf(str, "%04d:%02d:%02d:%02d:%02d:%02d", &t.year, &t.mon, &t.day, &t.hour, &t.min, &t.sec) != 6) {
            LogRunOperError("sscanf failed\n");
            return 0;
        }
    } else {
        if(str[8] == 'T') {
            if(sscanf(str, "%04d%02d%02dT%02d%02d%02d", &t.year, &t.mon, &t.day, &t.hour, &t.min, &t.sec) != 6) {
                LogRunOperError("sscanf failed\n");
                return 0;
            }
        } else {
            if(sscanf(str, "%04d%02d%02d%02d%02d%02d", &t.year, &t.mon, &t.day, &t.hour, &t.min, &t.sec) != 6) {
                LogRunOperError("sscanf failed\n");
                return 0;
            }
        }
    }
    return ind_time_make(&t);
}

int mid_tool_time2string(int sec, char* buf, char insert)
{
    struct ind_time t;

    if(buf == NULL) {
        LogRunOperError("buf is null\n");
        return -1;
    }

    ind_time_local((unsigned int)sec, &t);

    switch(insert) {
    case ' ':
        sprintf(buf, "%04d%02d%02d %02d:%02d:%02d", t.year, t.mon, t.day, t.hour, t.min, t.sec);
        break;
    case 'T':
        sprintf(buf, "%04d%02d%02dT%02d%02d%02dZ", t.year, t.mon, t.day, t.hour, t.min, t.sec);
        break;
    case ':':
        sprintf(buf, "%04d%c%02d%c%02d%c%02d%c%02d%c%02d", t.year, insert, t.mon, insert, t.day, insert, t.hour, insert, t.min, insert, t.sec);
        break;
    case '*':
        sprintf(buf, "%04d%02d%02d%02d%02d%02d", t.year, t.mon, t.day, t.hour, t.min, t.sec);
        break;
    default: {
#ifdef Sichuan
        sprintf(buf, "%02d%02d%02d%02d%02d%02d", t.year, t.mon, t.day, t.hour, t.min, t.sec);
#else
        sprintf(buf, "%04d%02d%02d%02d%02d%02d", t.year, t.mon, t.day, t.hour, t.min, t.sec);
#endif
        break;
    }
    }

    return 0;
}

/*************************************************
  Description:  获取分钟为最小单位的时间
  Input:           空
  Return:         无
  *************************************************/
int mid_tool_time2string_min(int sec, char* buf)
{
    struct ind_time t;

    if(buf == NULL) {
        LogRunOperError("buf is null\n");
        return -1;
    }

    /* 屏蔽警告错误 */
    ind_time_local((unsigned int)sec, &t);

    sprintf(buf, "%04d%02d%02d%02d%02d", t.year, t.mon, t.day, t.hour, t.min);
    buf[12] = '\0';

    return 0;
}


int mid_tool_time2str(int sec, char* buf, char insert)
{
    struct ind_time t;

    if(buf == NULL) {
        LogRunOperError("buf is null\n");
        return -1;
    }

    ind_time_local((unsigned int)sec, &t);

    if(insert == 0) {
        sprintf(buf, "%02d%02d%02d", t.hour, t.min, t.sec);
        buf[6] = '\0';
    } else {
        sprintf(buf, "%02d%c%02d%c%02d", t.hour, insert, t.min, insert, t.sec);
        buf[8] = '\0';
    }

    return 0;
}

// hcc. 15:42 2010-7-31
// use new version by liujianhua.
#if 0
// #if !defined(USE_NEW_DNS_LIBRARY)
unsigned int mid_tool_resolvehost(char *buf)
{
    in_addr_t addr = inet_addr(buf);

    if(addr == INADDR_ANY || addr == INADDR_NONE) {
        struct hostent *host = gethostbyname(buf);
        if(host)
            addr = *((in_addr_t *)(host->h_addr_list[0]));
    }
    if(addr == INADDR_ANY || addr == INADDR_NONE) {
        LogRunOperError("addr = %x\n", addr);
        return INADDR_NONE;
    }

    return addr;
}
#else
int mid_tool_resolvehost(char *buf, unsigned int *hostip)
{
    int ret = 0;
    unsigned int ipaddr;

    ipaddr = inet_addr(buf);

    if(ipaddr == INADDR_ANY || ipaddr == INADDR_NONE)
        ret = mid_dns_gethost(buf, &ipaddr, 28);
    *hostip = ipaddr;

    return ret;
}
#endif

int mid_tool_addr2string(unsigned int addr, char *buf)
{
    unsigned char *p;

    if(buf == NULL)
        return -1;
    p = (unsigned char *)(&addr);
    return sprintf(buf, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
}

int mid_tool_checkURL(const char* url, char *p_addr, int *p_port)
{
    char *p, *p2;
    int len;

    if(url == NULL || strncasecmp(url, "http://", 7))
        return -1;

    url += 7;
    if(url[0] == '[') {
        p = (char *)strchr(url, ']'); //WZW modified to fix pc-lint Error 158
        if(p == NULL) {
            LogRunOperError("ipv6 addr error\n");
            return -1;
        }
        if(p_addr) {
            len = p - url + 1;
            strncpy(p_addr, url, len);
            p_addr[len] = '\0';
        } else {
            LogRunOperError("ipv6 p_addr null\n");
            return -1;
        }
        if(p_port) {
            p++;
            if(*p == ':') {
                *p_port = atoi(p + 1);
            } else {
                *p_port = 80;
            }
        } else {
            LogRunOperError("ipv6 port null\n");
            return -1;
        }
    } else {
        p = (char *)strchr(url, '/');  //WZW modified to fix pc-lint Error 158
        if(p == NULL)
            p = (char *)url + strlen(url);
        p2 = (char *)strchr(url, ':');  //WZW modified to fix pc-lint Error 158
        if(p2 == NULL || p < p2) {
            len = p - url;
            if(p_port)
                *p_port = 80;
        } else {
            len = p2 - url;
            if(p_port)
                *p_port = atoi(p2 + 1);
        }
        if(len <= 4 || len > 64) {
            LogRunOperError("len err\n");
            return -1;
        }
        if(p_addr) {
            strncpy(p_addr, url, len);
            p_addr[len] = '\0';
        }
    }

    return 0;
}


static int overLimit(const char *p1, char *p2, int deadline)
{
    if(deadline > 0) {
        if(abs(p1 - p2) > deadline)
            return 1;
    }
    return 0;
}

char* distillValuebyTag(const char *xml, char *tagName, int *tagValueLen, int deadline)
{
    char *p = NULL;
    char *tmp = NULL;
    char tname[32];
    memset(tname, 0, 32);
    sprintf(tname, "<%s>", tagName);
    p = (char *)strstr(xml, tname);     //WZW modified to fix pc-lint Error 158
    if(NULL == p || overLimit(xml, p, deadline) > 0)
        return NULL;
    p += strlen(tname);
    tmp = p;

    memset(tname, 0, 32);
    sprintf(tname, "</%s>", tagName);
    p = strstr(tmp, tname);
    if(NULL == p || overLimit(xml, p, deadline) > 0)
        return NULL;

    *tagValueLen = p - tmp;
    return tmp;
}

int mid_tool_check_RTSPURL(char* url, char *p_addr)
{
    char			*p = NULL;
    int				len;

    if(url == NULL || strncasecmp(url, "rtsp://", 7))
        return -1;

    url += 7;
    p = strchr(url, '/');
    if(p == NULL)
        return -1;

    len = p - url;

    if(len <= 4 || len > 64)
        return -1;

    if(p_addr) {
        strncpy(p_addr, url, len);

        p = strchr(p_addr, ':');
        if(p != NULL)
            len = p - p_addr;

        p_addr[len] = '\0';
    }

    return 0;
}

/**  为保持与原来时区机制兼容并支持半时区。lh 2010-6-19
**    采用半时区* 4 +100的方式来记录，如果时区大于52 认为是半时区
***/
int mid_tool_timezone2str(int pTimezone, char* buf)
{
    int hour, min;
    int  tmp1;

    strcpy(buf, "UTC ");
    buf += 4;

    if(pTimezone < 52) {
        if(pTimezone > 0) {
            hour = pTimezone;
            buf[0] = '+';
        } else {
            hour = (0 - pTimezone);
            buf[0] = '-';
        }
        min = 0;

        sprintf(buf + 1, "%02d:%02d", hour, min);
    } else {
        tmp1 = pTimezone - 100;
        if(tmp1 > 0) {
            hour = tmp1 / 4;
            min = (tmp1 % 4) * 15;
            buf[0] = '+';
        } else {
            hour = (0 - tmp1) / 4;
            min = ((0 - tmp1) % 4) * 15;
            buf[0] = '-';
        }
        sprintf(buf + 1, "%02d:%02d", hour, min);
    }
    return 0;
}

int mid_tool_str2timezone(char* str)
{
    int hour, min, positive, tTimezone;
    double tmp;
    hour = min = 0;

    if(*str == '+') {
        positive = 1;
    } else {
        positive = -1;
    }
    sscanf(str + 1, "%02d:%02d", &hour, &min);

    if(min == 0)
        tTimezone = positive * hour;
    else {
        tmp = (min * 4) / 60;
        tTimezone = positive * (hour * 4 + tmp) + 100;
    }

    return tTimezone;
}

int mid_tool_timezone2sec(int pTimezone)
{
    int sec, tmp, hour, min;

    if(pTimezone < 52) {
        sec = pTimezone * 60 * 60;
    } else {
        tmp = pTimezone - 100;
//	   if(tmp>0)
//	   {
        hour = tmp / 4;
        min = (tmp % 4) * 15;
        sec = hour * 60 * 60 + min * 60;
//	   }
//	   else
//	   {
//	   	hour=(0-tmp)/4;
//		min=((0-tmp)%4)*15;
//		sec=hour*60*60+min*60;
//		sec=0-sec;
//	   }
    }
    return sec;
}


int mid_tool_resolvehost2(char *buf, unsigned int *hostip)
{
    int ret = 0;
    unsigned int ipaddr;

    ipaddr = inet_addr(buf);

    if(ipaddr == INADDR_ANY || ipaddr == INADDR_NONE) {
        //PRINTF("\n************before ntp:%d****************\n",mid_time());
        ret = mid_dns_gethost(buf, &ipaddr, 20);
        //PRINTF("\n************after ntp:%d****************\n",mid_time());
    }
    *hostip = ipaddr;

    return ret;
}

int mid_tool_checksum(char *buf, int len)
{
    int i = 0;
    int lcheck = 0;
    int *ptr = (int *)buf;

    len >>= 2;
    for(i = 0; i < len; i ++) {
        lcheck += *ptr++;
    }
    return lcheck;
}

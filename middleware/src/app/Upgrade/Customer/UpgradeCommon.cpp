#include "UpgradeCommon.h"
#include "UpgradeAssertions.h"

#include "mid/mid_tools.h"
#include "libzebra.h"

extern "C" {

int ys_front_set_poweroff_enable(int value);
}

#include <stdlib.h>
#include <string.h>


namespace Hippo {

#define SKIP_SPACE( )	while(*p == '\r' || *p == '\n' || *p == '\t' || *p == ' ') p++

char *getItemValue(char* buf, const char* tag, char* value)
{
    int len = strlen(tag);
    char *p = buf;
    value[0] = 0;

    SKIP_SPACE();
    if (strncasecmp(p, tag, len)) {
        UpgradeLogDebug("not match %s\n", tag);
        if(!strncasecmp(tag, "MD5", len))
            strcpy(value , "no md5");
        goto Err;
    }
    p += len;
    SKIP_SPACE();
    if (*p != '=') {
        UpgradeLogDebug("*p = %d\n", *p);
        goto Err;
    }
    p++;
    SKIP_SPACE();
    len = mid_tool_line_len(p);
    if (len <= 0 || len > 64) {
        UpgradeLogDebug("len = %d\n", len);
        goto Err;
    }
    memcpy(value, p, len);
    value[len] = 0;
    return (p + len);
Err:
    return NULL;
}

int getVersionNumAndFileName(char* buf, const char* item, int* ver, char* file)
{
    char *p = strstr(buf, item);

    if(!p)
        return 0;

    p += strlen(item);
    p = getItemValue(p, "Version", file);
    if (!p) {
        UpgradeLogDebug("get %s version\n", item);
        return -1;
    }

    char *pver = NULL;
    pver = strchr(file, '.');
    if (pver) {
        pver += 1;
        pver = strchr(pver, '.');
        if (pver)
            pver += 1;
        if (pver)
            *ver = atoi(pver);
    } else
        *ver = atoi(file);

    p = getItemValue(p, "FileName", file);
    if (!p) {
        UpgradeLogDebug("get %s filename\n", item);
        return -1;
    }

    UpgradeLogDebug("Get version number(%d) from config.ini\n", *ver);
    return 1;
}

int getLogoVersionNumAndFileName(char* buf,  const char* item, int* ver, char* filename, char* logomd5)
{
    char *p = strstr(buf, item);

    if(!p)
        return 0;

    p += strlen(item);
    p = getItemValue(p, "Version", logomd5);
    if (!p) {
        UpgradeLogDebug("get %s version\n", item);
        return -1;
    }

    char *pver = NULL;
    pver = strchr(logomd5, '.');
    if (pver){
        pver += 1;
        pver = strchr(pver, '.');
        if (pver)
            pver += 1;
        if (pver)
            *ver = atoi(pver);
    } else
        *ver = atoi(logomd5);

    p = getItemValue(p, "FileName", logomd5);
    if(!p) {
        UpgradeLogDebug("get %s filename\n", item);
        return -1;
    }
    strcpy(filename, logomd5);

    p = getItemValue(p, "MD5", logomd5);
    if (!p)
        UpgradeLogDebug("get %s md5\n", item);
    UpgradeLogDebug("Get version number(%d) from config.ini\n", *ver);
    return 1;
}

void urlCheckSum(char* buf)
{
    char *p = NULL;
    unsigned int sum = 0;

    p = buf;
    sum = 0;
    while(*p != '\0') {
        sum += (unsigned int) * p++;
    }
    sum += 1;
    sprintf(p, "%d", sum);
    return ;
}

#if defined(hi3560e)
#else
int firmware_burn_ex(char *firmware, int len, BRUN_FUNC func)
{
    int sdk_len = -1;
    int begin = 0;
    int cur   = 0;
    int compress = 0;
    int real     = 0;
    int precent = 0;
    unsigned int cur_long = 0;
    unsigned int compress_long = 0;

    if(!firmware || !func)
        return -1;

    if(yhw_upgrade_checkSoftware(firmware, sdk_len) != 0)
        return -2;

    ys_front_set_poweroff_enable(0);
    usleep(1000000);
    if(yhw_upgrade_burnSoftware(firmware, sdk_len) != 0) {
        ys_front_set_poweroff_enable(1);
        return -3;
    }

    while(1) {
        yhw_upgrade_getStatus(&begin, &cur, &compress, &real);
        printf("now begin is %d, precent is %d, cur is %d, compress is %d\n", begin, precent, cur, compress);

        if(begin == 0 && (cur == compress)) {
            func(100);
            break;
        }

        if(begin == 0) {
            yos_ipc_msleep(100);
            continue;
        } else {
            cur_long = cur;
            compress_long = compress;
            precent = (int)(cur_long * 100.0 / compress_long);
            func(precent);
            yos_ipc_msleep(500);
        }
    }
    ys_front_set_poweroff_enable(1);
    return 1;
}

#endif

}


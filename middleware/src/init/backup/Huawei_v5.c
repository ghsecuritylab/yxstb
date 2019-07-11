
#include "dhcp.h"
#include "pppoe.h"

#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "NativeHandler.h"

#include "http/http_apple.h"
#include "Assertions.h"

#include "libzebra.h"


void app_stream_init_private_province(void)
{
	/*
	    by liujianhua
	    stream库已经做了调整，不用再调用mid_stream_addsize
	    参看mid_stream.c里mid_stream_set_size前面的注释
	 */
	//mid_stream_addsize(0, 4600 * 1316);
	mid_stream_nat(1); // C57 need
	mid_stream_rrs_timeout(2000); // C58 need
    mid_stream_set_arq(1);
    mid_stream_set_burst(1);
}

char *HwSoftwareVersion_aa(int type)
{
    char version[64] = {0};
if (1 == type)
    strncpy(version, "V100R002C20LCMC11B002", sizeof(version));
else
    strncpy(version, "IPTV STB V100R002C20LCMC11B002", sizeof(version)); //huawei V5 base

    static char huaweiVer[256] = {0};

#if (defined DEBUG_BUILD)
#if defined(BROWSER_INDEPENDENCE)
    snprintf(huaweiVer, 256, "%s", version);
#else //BROWSER_INDEPENDENCE
    snprintf(huaweiVer, 256, "%s", version);
#endif //BROWSER_INDEPENDENCE
#else //DEBUG_BUILD
    snprintf(huaweiVer, 256, "%s", version);
#endif //DEBUG_BUILD
    return huaweiVer;
}

int HardwareVersion(char *hard_type, int len)
{
    YX_HW_INFO info;

    ys_cfg_get_hardware_info(&info);
    if(strlen(info.product_id) >= len) {
        strncpy(hard_type, info.product_id, len - 1);
        hard_type[len - 1] = '\0';
    } else {
        strcpy(hard_type, info.product_id);
    }

    return 0;
}

char *SQAVersion(void)
{
    return NULL;
}

char *SQMVersion(void)
{
    return NULL;
}


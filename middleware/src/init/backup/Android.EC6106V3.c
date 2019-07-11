#include "string.h"
#include "dhcp.h"
#include "pppoe.h"

#include "mid_stream.h"
#include "libzebra.h"

#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "NativeHandler.h"

#include "http/http_apple.h"
#include "Assertions.h"


void app_stream_init_private_province(void)
{
	/*
	    by liujianhua
	    stream库已经做了调整，不用再调用mid_stream_addsize
	    参看mid_stream.c里mid_stream_set_size前面的注释
	 */
	//mid_stream_addsize(0, 4600 * 1316);
#if _HW_BASE_VER_ >= 57
	mid_stream_nat(1);
#endif
#if _HW_BASE_VER_ >= 58
	mid_stream_rrs_timeout(2000);
#endif
    mid_stream_set_arq(1);
    mid_stream_set_burst(1);
}

int mid_dhcp_option125_check(unsigned char *buf, int len)
{
	int opt125strlen;
	unsigned char *opt125str = NULL;
	unsigned char vendor0[] = "SCCTCIPTVDHCPAAA";
    unsigned char vendor1[] = "SCITVDHCP";
    int ret = -1;

	if (!buf) {
		ERR_OUT("vendor is NULL\n");
    }

	if ((len < strlen(vendor0)+3)
          && (len < strlen(vendor1)+3)) {
		ERR_OUT("len = %d\n", len);
    }

    buf += 6; // skip the enterprise number

    opt125strlen = *buf;

    opt125str = buf + 1; // skip the length number

	if (!memcmp(opt125str, vendor0, strlen(vendor0))
          || !memcmp(opt125str, vendor1, strlen(vendor1))) {
        PRINTF("option 125 check ok.\n");
        ret = 0;
    } else {
        unsigned char cnext;
        cnext = opt125str[opt125strlen];
        opt125str[opt125strlen]='\0';
		PRINTF("option 125 check err, vendor = %s\n", opt125str);
        opt125str[opt125strlen]=cnext;
	}

Err:
	return ret;
}

void mid_SN17toSN32(char *SN17,char *SN32)
{
	if( (SN17 == NULL) || (SN32 == NULL) )
		ERR_OUT("change sn from 17 to 32 error\n");

    memcpy(SN32, "00100199006000010000000000000000", 32);

#if defined(hi3716m)
    memcpy(SN32 + 16, SN17 + 4, 4);

    if (!strncasecmp(SN17 + 8, "HWC", 3)) { // HWC:EC2108(Hi3716M方案)统一生产版本标志
        memcpy(SN32 + 4,  "02",  2);
        memcpy(SN32 + 12, "201", 3);
        memcpy(SN32 + 20, "201", 3);
    } else if (!strncasecmp(SN17 + 8, "H2E", 3)) { // H2E:EC2108(Hi3716M方案)对接四川电信局点标志
        memcpy(SN32 + 12, "208", 3);
        memcpy(SN32 + 20, "208", 3);
    } else if (!strncasecmp(SN17 + 8, "HB7", 3)) { // HB7:EC2108(Brcm7405方案)对接四川电信局点标志
        memcpy(SN32 + 12, "210", 3);
        memcpy(SN32 + 20, "210", 3);
    } else if (!strncasecmp(SN17 + 8, "H5D", 3)) { // H5D:EC5108(Brcm7405方案)对接四川电信局点标志
        memcpy(SN32 + 4,  "02",  2);
        memcpy(SN32 + 12, "510", 3);
        memcpy(SN32 + 20, "510", 3);
    } else {
        memcpy(SN32 + 12, SN17, 1);
        memcpy(SN32 + 13, SN17 + 2, 2);
        memcpy(SN32 + 20, SN17, 1);
        memcpy(SN32 + 21, SN17 + 2, 2);
    }
    memcpy(SN32 + 23, SN17 + 8, 9);
#else
    memcpy(SN32 + 12, SN17, 3);
	memcpy(SN32 + 20, SN17, 3);
	memcpy(SN32 + 16, SN17 + 4, 4);
	memcpy(SN32 + 23, SN17 + 8, 9);
#endif

Err:
    return;
}

int HardwareVersion(char *hard_type, int len)
{
    strcpy(hard_type, "EC6106V3");
    hard_type[strlen("EC6106V3")] = '\0';
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


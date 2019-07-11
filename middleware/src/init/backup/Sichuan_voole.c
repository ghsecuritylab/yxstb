#include "string.h"
#include "dhcp.h"
#include "pppoe.h"

#include "mid_stream.h"

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

    opt125str = buf + 1;

	if ((strlen(vendor0) == opt125strlen)
          && !memcmp(opt125str, vendor0, strlen(vendor0))) {
		PRINTF("option 125 check ok.\n");
        ret = 0;
	} else if ((strlen(vendor1) == opt125strlen)
          && !memcmp(opt125str, vendor1, strlen(vendor1))) {
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

    //add...

Err:
    return;
}

char *HwSoftwareVersion(int type)
{
    return "";
}

int HardwareVersion(char *hard_type, int len)
{
    unsigned int hardwareVer = 0;

    yhw_board_getHWVersion(&hardwareVer);

    if (hardwareVer == 0x300)
       strcpy(hard_type, "Z118");
    else
       strcpy(hard_type, "Z113");
    return 0;
}


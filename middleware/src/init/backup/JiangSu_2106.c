
#include "dhcp.h"
#include "pppoe.h"

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
	mid_stream_nat(1); // C57 need
	mid_stream_rrs_timeout(2000); // C58 need
    mid_stream_set_arq(1);
    mid_stream_set_burst(1);
}

int mid_dhcp_option125_check(unsigned char *buf, int len)
{
	int opt125strlen;
	unsigned char *opt125str = NULL;
	unsigned char vonder[] = "SHCTCIPTVDHCPAAA";

	if (buf == NULL)
		ERR_OUT("vendor is NULL\n");

	if (len <= 7)
		ERR_OUT("len = %d\n", len);

	opt125str = buf + 5;
	opt125strlen = len - 5;

	if (!memcmp(opt125str, vonder, opt125strlen)) {
		PRINTF("opt125 check ok, vendor = %s\n", opt125str);
	} else {
		ERR_OUT("opt125 check err, vendor = %s, buf = %s\n", opt125str, buf);
	}

	return 0;
Err:
	return -1;
}
void mid_SN17toSN32(char *SN17,char *SN32)
{
	if( (SN17 == NULL) || (SN32 == NULL) )
		ERR_OUT("change sn from 17 to 32 error\n");

    memcpy(SN32, "00100299006020110000000000000000", 32);
    memcpy(SN32 + 16, SN17 + 4, 4);
    memcpy(SN32 + 20, "201", 3);
    memcpy(SN32 + 23, SN17 + 8, 9);
Err:
    return;
}

char *HwSoftwareVersion(int type)
{
    if(1 == type)
        return "V100R002C15LJSD32";
    else
        return "IPTV STB V100R002C15LJSD32";
}

int HardwareVersion(char *hard_type, int len)
{
#if defined(brcm7405)
    YX_HW_INFO info;

    yhw_env_getHardwareInfo(&info);
    if(strlen(info.product_id) >= len) {
        strncpy(hard_type, info.product_id, len - 1);
        hard_type[len - 1] = '\0';
    } else
        strcpy(hard_type, info.product_id);
#elif defined(hi3716m)
    unsigned int hardwareVer = 0;

    if(!hard_type)
        return -1;

    yhw_board_getHWVersion(&hardwareVer);
    if (hardwareVer == 0x300)
       strcpy(hard_type, "EC2108V3");
    else
       strcpy(hard_type, "EC2108");
#else
#error "Can't defined platform."
#endif

    return 0;
}


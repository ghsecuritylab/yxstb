
#include "dhcp.h"
#include "pppoe.h"
#include "app_include.h"

#include "mid_stream.h"

#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "NativeHandler.h"

#include "http/http_apple.h"

void app_stream_init_private_province(void)
{
	//mid_stream_addsize(0, 4600 * 1316);
	mid_stream_nat(1); // C57 need
	mid_stream_rrs_timeout(2000); // C58 need
    mid_stream_set_arq(1);
    mid_stream_set_burst(1);
    mid_stream_heartbit_standard(0);    // 用option心跳。
    mid_stream_standard(RTSP_STANDARD_UNICOM);  // Play 要加 Range:
}

int mid_dhcp_option125_check(unsigned char *buf, int len)
{
	int opt125strlen;
	unsigned char *opt125str = NULL;
	unsigned char vendor[] = "SHCTCIPTVDHCPAAA";
    int ret = -1;

	if (buf == NULL)
		ERR_OUT("vendor is NULL\n");

	if (len < strlen(vendor)+3)
		ERR_OUT("len = %d\n", len);

    buf += 2; // skip the enterprise number

    opt125strlen = *buf;

    opt125str = buf + 1;

	if (strlen(vendor)==opt125strlen && !memcmp(opt125str, vendor, strlen(vendor))) {
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
	return ret; // liuqun: should return ret here when build shanghai filed version
	               // but return 0 or ret under testing.
}

void mid_SN17toSN32(char *SN17,char *SN32)
{
	if( (SN17 == NULL) || (SN32 == NULL) )
		ERR_OUT("change sn from 17 to 32 error\n");

	strcpy(SN32, "00000100000821010000000000000000");
    strncpy(SN32 + 16, SN17 + 4, 4);
    strncpy(SN32 + 20, SN17, 3);
    strncpy(SN32 + 23, SN17 + 8, 9);

    *(SN32 + 32) = '\0';
Err:
    return;
}


char *HwSoftwareVersion(int type)
{
    if(1 == type)
        return "V100R002C15LNML11";
    else
        return "IPTV STB V100R002C15LNML11";
}

int HardwareVersion(char *hard_type, int len)
{
    unsigned int hardwareVer = 0;

    yhw_board_getHWVersion(&hardwareVer);
    if (hardwareVer == 0x300)
       strcpy(hard_type, "EC2108V3");
    else
       strcpy(hard_type, "EC2108");
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


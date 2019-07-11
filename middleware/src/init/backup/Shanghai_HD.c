
#include "dhcp.h"
#include "pppoe.h"

#include "app_include.h"
#include "mid_stream.h"
#include "Assertions.h"

static const int g_master = 0;

int test(unsigned char *buf, int len);
void app_stream_init_private_province(void);

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

void app_stream_init_private_province(void)
{
	mid_stream_statint_pklosts(g_master, 10);
	mid_stream_statint_bitrate(g_master, 5);
	mid_stream_timeshift_second(0);
	mid_stream_standard(RTSP_STANDARD_CTC_SHANGHAI);
	mid_stream_cache(-1, 0); 
}

void mid_SN17toSN32(char *SN17,char *SN32)
{
	if ((SN17 == NULL) || (SN32 == NULL))
		ERR_OUT("change sn from 17 to 32 error\n");
	
    memcpy(SN32, "00100299003011210000000000000000", 32);
    memcpy(SN32 + 16, SN17 + 4, 4);
    memcpy(SN32 + 20, SN17 + 1, 3);
    memcpy(SN32 + 23, SN17 + 8, 9);

Err:
    return;
}

char *HwSoftwareVersion(int type)
{
    if(1 == type)
        return "R002C01LJSD21";
    else
        return "IPTV STB R002C01LJSD21";
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

char *SQAVersion(void)
{
    return NULL;
}

char *SQMVersion(void)
{
    return NULL;
}
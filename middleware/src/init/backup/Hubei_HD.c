#include "dhcp.h"
#include "pppoe.h"

#include "app_include.h"

#include "Assertions.h"

#include "MessageValueNetwork.h"
#include "MessageTypes.h"

static const int g_master = 0;

int mid_dhcp_option125_check(unsigned char *buf, int len);


int mid_dhcp_option125_check(unsigned char *buf, int len)
{
	int opt125strlen;
	unsigned char *opt125str = NULL;
	unsigned char vonder[] = "HBCTCIPTVDHCPAAA";

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

void app_stream_init_private_province(void)
{
    mid_stream_voddelay(1000); 
}

void mid_SN17toSN32(char *SN17,char *SN32)
{
	if ((SN17 == NULL) || (SN32 == NULL))
		ERR_OUT("change sn from 17 to 32 error\n");
	
    memcpy(SN32, "00100199006000010000000000000000", 32);
    memcpy(SN32 + 16, SN17 + 4, 4);

    if (!strncasecmp(SN17 + 8, "HWC", 3)) { // HWC:EC2108(Hi3716M方案)统一生产版本标志
        memcpy(SN32 + 4,  "02",  2);
        memcpy(SN32 + 12, "201", 3);
        memcpy(SN32 + 20, "201", 3);
    } else if(!strncasecmp(SN17 + 8, "H5E", 3)) { // H5E:EC2108(Hi3716M方案)对接湖北电信局点
        memcpy(SN32 + 12, "208", 3);
        memcpy(SN32 + 20, "208", 3);
    } else {
        memcpy(SN32 + 12, "208", 3);
        memcpy(SN32 + 20, "208", 3);        
    }

    memcpy(SN32 + 23, SN17 + 8, 9);

Err:
    return;
}

char *HwSoftwareVersion(int type)
{
    if(1 == type)
        return "V100R023C15LHBD53";
    else
        return "IPTV STB V100R023C15LHBD53";
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
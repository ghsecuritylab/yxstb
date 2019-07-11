
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

    memcpy(SN32, "00100199006030810000000000000000", 32);
    memcpy(SN32 + 16, SN17 + 4, 4);
    memcpy(SN32 + 20, "308", 3);
    memcpy(SN32 + 23, SN17 + 8, 9);
Err:
    return;
}

char *HwSoftwareVersion(int type)
{
    if(1 == type)
        return "V100R002C15LJSD18";
    else
        return "IPTV STB V100R002C15LJSD18";
}

int HardwareVersion(char *hard_type, int len)
{
    char *hardtype = "M8048";

    if(!hard_type)
        return -1;

    if(strlen(hardtype) >= len) {
        strncpy(hard_type, hardtype, len - 1);
        hard_type[len - 1] = '\0';
    } else
        strcpy(hard_type, hardtype);

    return 0;
}


char *SQAVersion(void)
{
    return "IPTV SQA V100R001C01B059-SPC2";
}

char *SQMVersion(void)
{
    return "IPTV SQM V100R001C23B003";
}


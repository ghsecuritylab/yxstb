
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
        return "V100R002C15LGDDn1";
    else
        return "IPTV STB V100R002C15LGDDn1";
}

int HardwareVersion(char *hard_type, int len)
{
    if(!hard_type || len < 8)
        return -1;

    strcpy(hard_type, "EC2106V1");
    return 0;
}



#include "dhcp.h"
#include "pppoe.h"

#include "mid_network.h"

#include "MessageTypes.h"
#include "MessageValueNetwork.h"

#include "NativeHandler.h"

#include "http/http_apple.h"
#include "Assertions.h"


void app_stream_init_private_province(void)
{
	//mid_stream_addsize(0, 4600 * 1316);
    mid_stream_nat(1); // C57 need
    mid_stream_rrs_timeout(2000); // C58 need
    mid_stream_set_arq(1);
    mid_stream_set_burst(1);
}

/* 辽宁标清对接的为联通平台，没有17转32位串号规则，直接拷贝17位串号 */
void mid_SN17toSN32(char *SN17,char *SN32)
{
    if ((SN17 == NULL) || (SN32 == NULL))
        ERR_OUT("change sn from 17 to 32 error\n");

    memcpy(SN32, "00000000000813000000000000000000", 32);
    memcpy(SN32 + 16, SN17 + 4, 2);//生产年代
    memcpy(SN32 + 18, SN17 + 6, 2);//生产周
    network_tokenmac_get(SN32 + 20, 12, 0);
    Err:
        return;
}

char *HwSoftwareVersion(int type)
{
    if(1 == type)
        return "V100R002C12LHCL21";
    else
        return "IPTV STB V100R002C12LHCL21";
}

int HardwareVersion(char *hard_type, int len)
{
    char *hardtype = "ME8048";

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
    return NULL;
}

char *SQMVersion(void)
{
    return NULL;
}

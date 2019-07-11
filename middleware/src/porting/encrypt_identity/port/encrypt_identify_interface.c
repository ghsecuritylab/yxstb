#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef ENCRYPT_IDENTITY_VIA_HUAWEILIB

#include "encrypt_identify_interface.h"

#include "app_heartbit.h"
#include "cryptoFunc.h"
#include "charConvert.h"
#include "Account.h"

static char g_local_mac[20];
static char g_md5_out[35];
char* getShareKey()
{
	return AccountGetShareKey();
}
char* getMacAddress()
{
	memset(g_local_mac, 0, 20);
    network_tokenmac_get(g_local_mac, 20, 0);
	return g_local_mac;
}
char* getMD5(char* input)
{
	memset(g_md5_out, 0, 35);
    md5Encypt(&input, 1, g_md5_out, sizeof(g_md5_out), 0);
    data2Hex(g_md5_out, 16, g_md5_out, sizeof(g_md5_out));
    lower2Upper(g_md5_out, strlen(g_md5_out));
	return g_md5_out;
}

#endif

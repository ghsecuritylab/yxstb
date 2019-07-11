
#include "JseHWEncrypt.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "openssl/sha.h"

#include <string.h>

static int JseSHA256Read(const char *param, char *value, int len)
{
    LogJseDebug("SHA256 param=#%s#\n", param);
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, param, strlen(param));
    unsigned char buf1[32];
    SHA256_Final(buf1, &sha256);
    int i;
    for (i = 0; i < 32; i++)
        sprintf(&value[i * 2], "%02X", buf1[i]);
    value[64] = '\0';
    return 0;
}

/*************************************************
Description: 初始化并注册华为定义的接口 <Tools.AES.Encrypt.***> 
接口相关说明见 《IPTV 海外版本STB与EPG接口文档 STB公共能力(Webkit) V1.1》
Input: 无
Return: 无
 *************************************************/ 
JseHWEncrypt::JseHWEncrypt()
	: JseGroupCall("Encrypt")
{
    JseCall *call  = new JseFunctionCall("SHA256", JseSHA256Read, 0);
    regist(call->name(), call);
}

JseHWEncrypt::~JseHWEncrypt()
{
}



#include "JseHWSHA256.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "json_tokener.h"
#include "json_object.h"

#include "AppSetting.h"
#include "app_epg_para.h"
#include "ind_mem.h"
#include "sys_basic_macro.h"
#include "cryptoFunc.h"
#include "charConvert.h"
#include "Session.h"

#include "openssl/sha.h"

#include <stdio.h>
#include <string.h>

static char signatureKey[32] = { 0 };

static int JseSignatureKeyWrite(const char *param, char *value, int len)
{
    char passwdmd5[USER_LEN + 4];
    char user[USER_LEN + 4];
    char buf[256];

    LogJseDebug("value=%s\n", value);
    struct json_object *object  = NULL;

    object = json_tokener_parse(value);
    if (!object) {
        LogJseError("ERROR\n");
        return -1;
    }
    const char *random = json_object_get_string(json_object_object_get(object, "random"));
    if (!random) {
        LogJseError("ERROR\n");
        return -1;
    }
    LogJseDebug("random = [%s]\n", random);

    // SHA256(UserKey+InitVector+UserID+ random+cnonce)

    char cont[USER_LEN] = { 0 };
    char* p = cont;
    appSettingGetString("ntvAESpasswd", cont, USER_LEN, 0);
    md5Encypt(&p, 1, passwdmd5, sizeof(passwdmd5), 1);
    data2Hex(passwdmd5, 4, passwdmd5, sizeof(passwdmd5));
    appSettingGetString("ntvuser", user, USER_LEN, 0);
    sprintf(buf , "%s99991231%s%s%s", passwdmd5, user, random, session().getCnonce());
    LogJseDebug("[%s]\n", buf);

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, buf, strlen(buf));
    SHA256_Final((unsigned char*)signatureKey, &sha256);

    json_object_put(object);
    return 0;
}

static int JseSensitiveDataEncryptRead(const char *param, char *value, int len)
{
    char databuf[256] = {0};
    struct json_object *object  =  json_tokener_parse(param);
    char *sensitiveData = (char *)json_object_get_string(json_object_object_get(object, "sensitiveData"));

    if (aesEcbEncrypt(sensitiveData, strlen(sensitiveData), signatureKey, databuf, sizeof(databuf)) < 0) {
        json_object_put(object);
        LogJseError("ERROR\n");
        return -1;
    }

    if (EVP_EncodeBlock(value, databuf, strlen(databuf)) < 0) {//base64 encode
        LogJseError("ERROR, EVP_EncodeBlock\n");
        json_object_put(object);
        return -1;
    }
    json_object_put(object);
    return 0;
}

static int JseSensitiveDataDecryptRead(const char *param, char *value, int len)
{
    struct json_object *object  =  json_tokener_parse(param);
    char *sensitiveData = (char *)json_object_get_string(json_object_object_get(object, "sensitiveData"));
    char result[4096] = {0};

    if (EVP_DecodeBlock(result, sensitiveData, strlen(sensitiveData)) == -1 ) {
        LogJseError("ERROR, EVP_DecodeBlock\n");
        json_object_put(object);
        return -1;
    }

    if (aesEcbDecrypt(result, strlen(result), signatureKey, value, len)) < 0) {
        json_object_put(object);
        return -1;
    }

    json_object_put(object);
    return 0;
}

//TODO
static int JseAntiTamperDataDigestRead(const char *param, char *value, int len)
{
    return 0;
}


/*************************************************
Description: 初始化华为SHA256模块配置定义的接口，由JseModules.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWSHA256Init()
{
#ifdef HUAWEI_C20
    JseCall* call;

    call = new JseFunctionCall("signatureKey", 0, JseSignatureKeyWrite);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("sensitiveDataEncrypt", JseSensitiveDataEncryptRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("sensitiveDataDecrypt", JseSensitiveDataDecryptRead, 0);
    JseRootRegist(call->name(), call);

    call = new JseFunctionCall("antiTamperDataDigest", JseAntiTamperDataDigestRead, 0);
    JseRootRegist(call->name(), call);
#endif
    return 0;
}


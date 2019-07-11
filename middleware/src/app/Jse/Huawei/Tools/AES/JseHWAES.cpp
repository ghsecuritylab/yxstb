
#include "JseHWAES.h"

#include "JseFunctionCall.h"
#include "JseAssertions.h"

#include "json/json_public.h"
#include "cryptoFunc.h"
#include "charConvert.h"

#include <string.h>

#define huawei_key_suffix   "OkM9"
#define huawei_vector_suffix   "QaZ1"

static const char * huawei_default_key =  "HuaweiiewauHOkM9";
static const char * huawei_default_vector = "HuaweiiewauHQaZ1";

struct huawei_aes_param{
	char encryptMode; //0:ecb; 1:cbc
	int cipherSize;
	char paddingMode; //0:pkcs5; other unknown.
	char cipherPrefix[32 + 1]; //default is 128(16B),extents 256;
	char IVPrefix[32 + 1];//default is 128(16B),extents 256;
	char text[4096 + 1];//plainText or  cipherText
};

static int huaweiAESJsonParse(char *param_json, char *result_buf, int result_buf_len, int encrypt)
{
    struct json_object* object = 0;
    struct json_object* sub_object = 0;
    struct huawei_aes_param param;
    int ret = 0;
    const char *result = 0;
    unsigned char result_text[4096 + 1] = {0};
    unsigned char temp[4096 + 1] = {0};
    int result_len = 4096;
    char buf[8192 + 1];

    if (*param_json == '"')
        param_json ++;
    strcpy(buf, param_json);//去掉第一个引号
    if (buf[strlen(buf) - 1] == '"')//去掉后面的引号
        buf[strlen(buf) - 1] = 0;

    if (!strlen(buf)) {
        LogJseError("Tools.AES.Encrypt or Tools.AES.Decryptparam is NULL\n");
        return -1;
    }

    object = json_tokener_parse_string(buf);
    if (object){
        memset(&param, 0, sizeof(struct huawei_aes_param));
        sub_object = json_object_get_object_bykey(object, "encryptMode");
        if (sub_object) {
            result = json_get_object_string(sub_object);
            if (result) {
                if (!strcmp(result, "CBC")) {
                    param.encryptMode = 1;
                } else {
                    param.encryptMode = 0;
                }
            }
        } else {
            param.encryptMode = 0;
        }
        sub_object = json_object_get_object_bykey(object, "cipherSize");
        if (sub_object) {
            param.cipherSize = json_get_object_int(sub_object);
            if (param.cipherSize  != 128) {
                LogJseDebug("cipherSize is invalid, should be 128 bit\n");
                ret = -1;
                goto end_object_process;
            }
        } else {
            param.cipherSize = 128;
        }
        sub_object = json_object_get_object_bykey(object, "paddingMode");
        if (sub_object) {
            result = json_get_object_string(sub_object);
            if (result) {
                if(!strcmp(result, "PKCS5Padding"))
                    param.paddingMode = 0;
                else {
                    LogJseDebug("paddingMode is unsupported:%s\n", result);
                    ret = -1;
                    goto end_object_process;
                }
            }
        } else {
            param.paddingMode = 0;
        }
        sub_object = json_object_get_object_bykey(object, "cipherPrefix");
        if (sub_object) {
            result = json_get_object_string(sub_object);
            if (result) {
                strncpy(param.cipherPrefix, result, 12);
            }
        }
        if (param.encryptMode == 1) { //cbc
            sub_object = json_object_get_object_bykey(object, "IVPrefix");
            if (sub_object) {
                result = json_get_object_string(sub_object);
                if (result) {
                    strncpy(param.IVPrefix, result, 12);
                }
            }
        }
        if (encrypt) {
            sub_object = json_object_get_object_bykey(object, "plainText");
            if (sub_object) {
                result = json_get_object_string(sub_object);
                if (result) {
                    strncpy(param.text, result, 4096);
                }
            } else {
                LogJseDebug("plainText is NULL\n");
                ret = -1;
                goto end_object_process;
            }
        } else {
            sub_object = json_object_get_object_bykey(object, "cipherText");
            if (sub_object) {
                result = json_get_object_string(sub_object);
                if (result) {
                    strncpy(param.text, result, 4096);
                }
            } else {
                LogJseDebug("cipherText is NULL\n");
                ret = -1;
                goto end_object_process;
            }
        }

    end_object_process:
        json_object_delete(object);
        if (ret != 0) //fail
            return ret;
    } else {
        LogJseDebug("Tools.AES.Encrypt param or Tools.AES.Decryptparam is invalid\n");
        return -1;
    }
    if (param.cipherPrefix[0] != 0) {
        strcat(param.cipherPrefix, huawei_key_suffix);
    } else {
        strcpy(param.cipherPrefix, huawei_default_key);
    }
    if (param.encryptMode == 0) {
        if (encrypt) {
            ret = aesEcbEncrypt(param.text, strlen(param.text), param.cipherPrefix, (char *)temp, result_len);
            ret = data2Hex((char*)temp, ret, (char*)result_text, result_len);
            ret = lower2Upper((char*)result_text, ret);
        } else {
            upper2Lower(param.text, strlen(param.text));
            ret = hex2Data(param.text, strlen(param.text), (char*)temp, sizeof(temp));
            ret = aesEcbDecrypt((char *)temp, ret, param.cipherPrefix, (char *)result_text, result_len);
        }
    } else if  (param.encryptMode == 1) {
        if(param.IVPrefix[0] != 0){
            strcat(param.IVPrefix, huawei_vector_suffix);
        } else {
            strcpy(param.IVPrefix, huawei_default_vector);
        }
        if (encrypt) {
            ret = aesCbcEncrypt(param.text, strlen(param.text), param.cipherPrefix, param.IVPrefix, (char *)result_text, &result_len);
            ret = data2Hex((char*)result_text, result_len, (char*)result_text, sizeof(result_text));
            ret = lower2Upper((char*)result_text, ret);
        } else {
            upper2Lower(param.text, strlen(param.text));
            ret = hex2Data(param.text, strlen(param.text), (char*)temp, sizeof(temp));
            ret = aesCbcDecrypt(param.text, ret, param.cipherPrefix, param.IVPrefix, (char *)result_text, &result_len);
        }
    }
    ret = (ret >= 0) ? 0 : 1;   //0:success;1:fail
    if (ret == 1) {
        sprintf(result_buf, "{\"status\":%d}", ret);
    } else {
        if (encrypt) {
            sprintf(result_buf, "{\"status\":%d, \"cipherText\":\"%s\"}", ret, result_text);
        } else {
            sprintf(result_buf, "{\"status\":%d, \"plainText\":\"%s\"}", ret, result_text);
        }
    }
    return 0;
}

static int JseAESEncrypt(const char *param, char *value, int len)
{
    LogJseDebug("Tools.AES.Encrypt in jse read param[%s]\n", param);
    huaweiAESJsonParse((char*)param, value, len, 1);
    return 0;
}

static int JseAESDecrypt(const char *param, char *value, int len)
{
    LogJseDebug("Tools.AES.Decrypt in jse read param[%s]\n", param);
    huaweiAESJsonParse((char*)param, value, len, 0);
    return 0;
}

/*************************************************
Description: 初始化并注册华为定义的接口 <Tools.AES.***>
接口相关说明见 《IPTV 海外版本STB与EPG接口文档 STB公共能力(Webkit) V1.1》
Input: 无
Return: 无
 *************************************************/
JseHWAES::JseHWAES()
	: JseGroupCall("AES")
{
    JseCall *call;

    call = new JseFunctionCall("Encrypt", JseAESEncrypt, 0);
    regist(call->name(), call);

    call = new JseFunctionCall("Decrypt", JseAESDecrypt, 0);
    regist(call->name(), call);
}

JseHWAES::~JseHWAES()
{
}


#include "UDiskConfig.h"
#include "UDiskDetect.h"
#include "UDiskAssertions.h"

#include "AppSetting.h"
#include "SysSetting.h"
#include "mid_sys.h"

#include "cryptoFunc.h"
#include "charConvert.h"
#include "openssl/des.h"
#include "openssl/evp.h"
#include "openssl/md5.h"
#include "sys_msg.h"
#include "app_sys.h"

#include "Tr069.h"


#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace Hippo {

static int gUserNum = 0;
static int gCommonParserFlag = 0;
static int gParseAccountInited = 0;

static AccountConfig_s* gAccountCfgLinkHead = NULL;
static CommonConfig_s gCommonCfg;
static UserConfig_s gUserCfg;

static int _Decrypt3DES(char *in, char *out);
static void _CopyRightValue(char *out, char *in, unsigned int len);
static int _ParseAccountConfigData(const char* filepath);
static int _ParseCommonConfigData(const char* filepath);
static int _ParseUserConfigData(const char* filepath);
extern "C" int pkcs5PaddingRemove(char* plain_data, int plain_data_len);

static void _CopyRightValue(char* out, char* in, unsigned int outlen)
{/*{{{*/
    if(out == NULL || in == NULL) {
        LogUDiskDebug("_CopyRightValue error\n");
        return;
    }
    int len = 0;
    len = strlen(in);
    if(outlen < strlen(in))
        len = outlen;
    memcpy(out, in, len);
    out[len] = '\0';
}/*}}}*/

static int _Decrypt3DES(char* in, char* out)
{
    static int init_decrypt_key = 0;
    static DES_key_schedule ks1, ks2, ks3;

    //init 3des key
    if(init_decrypt_key == 0) {
        int len, i;
        char key[37] = {0};
        DES_cblock key1, key2, key3;
        MD5_CTX ctx;

        MD5_Init(&ctx);
        MD5_Update(&ctx, (unsigned char *)"huaweiuser", 10);
        MD5_Update(&ctx, (unsigned char *)"99991231", 8); //»ªÎªË½ÓÐ
        MD5_Final((unsigned char*)key, &ctx);
        len = data2Hex(key, 12, key, sizeof(key));
        lower2Upper(key, len);

        memcpy(key1, key, 8);
        memcpy(key2, key + 8, 8);
        memcpy(key3, key + 16, 8);
        //set Key
        DES_set_key_unchecked(&key1, &ks1);
        DES_set_key_unchecked(&key2, &ks2);
        DES_set_key_unchecked(&key3, &ks3);
        init_decrypt_key = 1;
    }
    int inlen = 0;
    int outlen = 0;
    int ret = 0;
    char buf[1024] = {0};
    char *temp = NULL;

    inlen = EVP_DecodeBlock((unsigned char*)buf, (unsigned char*)in, strlen(in));
    while(buf[inlen - 1] == '\0')  inlen --;//EVP_DecodeBlock·µ»ØÖµ²»ÄÜÖ±½ÓÓÃ¡£º¯Êý½âÂëÊ±Î´È¥µôÌî³ä×Ö½Ú
    if(inlen < 0) {
        LogUDiskDebug("the buffer is not a base64encoder string\n");
        return -1;
    }

    DES_cblock plain_text[40], output[40];
    int text_len, text_remainder;
    int i, j;

    text_len = inlen / 8  ;
    text_remainder = inlen % 8;

    if(text_remainder != 0) {
        LogUDiskDebug("the string is not a decrypte by 3des\n");
        return -1;
    }
    for(i = 0; i < text_len; i++)
        for(j = 0; j < 8; j++)
            plain_text[i][j] = buf[ i * 8 + j ];

    for(i = 0; i < text_len; i++)
        DES_ecb3_encrypt(&plain_text[i], &output[i], &ks1, &ks2, &ks3, 0);

    int len = 0;
    for(i = 0; i < text_len; i++) {
        for(j = 0; j < 8; j++) {
            len += sprintf(out + len, "%c", output[i][j]);
        }
    }
    outlen = pkcs5PaddingRemove(out, inlen);
    *(out + outlen) = '\0';

    temp = strstr(out, "\r");
    if(temp != NULL) {
        *temp = '\0';
    }
    LogUDiskDebug("the decrpt buffer =%s\n", out);
    return 0;
}

static int _ParseAccountConfigData(const char* filepath)
{
    LogUDiskDebug("parse account data\n");
    FILE* fAccountCfg = NULL;
    char  wLocalBuffer[528] = { 0 };
    char *pRightValue = NULL;
    char *pStr = NULL;
    short wParseSign = 0;
    AccountConfig_s* pAccountNode = NULL;
    AccountConfig_s* tAccountNode = NULL;
    fAccountCfg = fopen(filepath, "rb");
    if(!fAccountCfg) {
        LogUDiskDebug("open account file failed\n");
        return -1;
    }
    gAccountCfgLinkHead = (AccountConfig_s*)calloc(1, sizeof(AccountConfig_s)); /* head node */
    if(!gAccountCfgLinkHead) {
        LogUDiskWarn("create head node.\n");
        goto Err;
    }

    while(true) {
        pAccountNode = (AccountConfig_s*)calloc(1, sizeof(AccountConfig_s));
        if(!pAccountNode)
            goto Err;
        for(wParseSign = 0; wParseSign < 3; ++wParseSign) {
            if(fgets(wLocalBuffer, 528, fAccountCfg)) {
                // be compatible with sign '\n'
                pStr = strstr(wLocalBuffer, "\r\n");
                if(!pStr) {
                    pStr = strstr(wLocalBuffer, "\n");
                    if(!pStr)
                        continue;
                    else
                        *pStr = '\0';
                } else
                    *pStr = '\0';

                switch(wParseSign) {
                    case 0:
                        pRightValue = strstr(wLocalBuffer, "[");
                        if(!pRightValue)
                            goto Err;
                        _CopyRightValue(pAccountNode->nAccount, pRightValue + 1, 31);
                        pRightValue = strstr(pAccountNode->nAccount, "]");
                        if(pRightValue)
                            *pRightValue = '\0';
                        break;
                    case 1:
                        pRightValue = strstr(wLocalBuffer, "=");
                        if(!pRightValue)
                            goto Err;
                        _CopyRightValue(pAccountNode->nUser, pRightValue + 1, 31);
                        break;
                    case 2:
                        pRightValue = strstr(wLocalBuffer, "=");
                        if(!pRightValue)
                            goto Err;
                        pAccountNode->nIsInitialzed = atoi(pRightValue + 1);
                        break;
                    default:
                        break;
                }
            } else {
                LogUDiskDebug("file end\n");
                free(pAccountNode);
                fclose(fAccountCfg);
                return 0;
            }
            memset(wLocalBuffer, 0, 528);
        }
        //Use tail insert
        tAccountNode = gAccountCfgLinkHead;
        while (tAccountNode->pNext)
            tAccountNode = tAccountNode->pNext;
        tAccountNode->pNext = pAccountNode;
        //pAccountNode->pNext = gAccountCfgLinkHead->pNext;
        //gAccountCfgLinkHead->pNext = pAccountNode;
        gUserNum++;
    }
Err:
    fclose(fAccountCfg);
    if(pAccountNode)
        free(pAccountNode);
    return -1;
}

static int _ParseCommonConfigData(const char* filepath)
{/*{{{*/
    LogUDiskDebug("parse common data\n");
    FILE* fCommonCfg = NULL;
    char* pStr = NULL;
    char* pRightValue = NULL;
    char wLocalBuffer[528];
    char wTempBuffer[528];
    int wModFlag = 0;
    int wFullErr = 1;
    char tempstr[64] = {0};
    int ret = -1;
    if(gCommonParserFlag) {
        LogUDiskDebug("parse common config is parse already\n");
        return 0;
    }
    fCommonCfg = fopen(filepath, "rb");
    if(!fCommonCfg) {
        LogUDiskDebug("open config info file failed\n");
        return -1;
    }
    memset(&gCommonCfg, 0, sizeof(CommonConfig_s));
    while(fgets(wTempBuffer, 528, fCommonCfg)) {
        // be compatible with sign '\n'
        pStr = strstr(wTempBuffer, "\r\n");
        if(!pStr) {
            pStr = strstr(wTempBuffer, "\n");
            if(!pStr)
                continue;
            else
                *pStr = '\0';
        } else
            *pStr = '\0';

        memset(wLocalBuffer, 0, 528);
        ret = _Decrypt3DES(wTempBuffer, wLocalBuffer);
        if(ret == -1)
            continue;

        LogUDiskDebug("fgets = [%s]\n", wLocalBuffer);
        pRightValue = strstr(wLocalBuffer, "=");
        if(!pRightValue)
            continue;
        int str_len_right = pStr - pRightValue;

        pRightValue += 1;
        str_len_right -= 1;
        if(str_len_right == 0) {
            wModFlag = -1;
            continue;
        }

        while(*(pRightValue) == 0x20 || *(pRightValue) == 0) {
            pRightValue += 1;
            str_len_right -= 1;
            if(str_len_right == 0 || *(pRightValue) == 0) {
                wModFlag = -1;
                continue;
            }
        }
        LogUDiskDebug(" pRightValue=%s\n", pRightValue);

        if(!strncmp("netuseraccount", wLocalBuffer, strlen("netuseraccount"))) {
            _CopyRightValue(gCommonCfg.nNetuseraccount, pRightValue, 32);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("netuserpassword", wLocalBuffer, strlen("netuserpassword"))) {
            _CopyRightValue(gCommonCfg.nNetuserpassword, pRightValue, 31);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("defContAcc", wLocalBuffer, strlen("defContAcc"))) {
            _CopyRightValue(gCommonCfg.nDefContAcc, pRightValue, 31);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("defContPwd", wLocalBuffer, strlen("defContPwd"))) {
            _CopyRightValue(gCommonCfg.nDefContPwd, pRightValue, 31);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("ntvuseraccount", wLocalBuffer, strlen("ntvuseraccount"))) {
            _CopyRightValue(gCommonCfg.nNtvuseraccount, pRightValue, 31);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("ntvuserpassword", wLocalBuffer, strlen("ntvuserpassword"))) {
            _CopyRightValue(gCommonCfg.nNtvuserpassword, pRightValue, 31);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("stbIP", wLocalBuffer, strlen("stbIP"))) {
            _CopyRightValue(gCommonCfg.nStbIP, pRightValue, 15);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("netmask", wLocalBuffer, strlen("netmask"))) {
            _CopyRightValue(gCommonCfg.nNetmask, pRightValue, 15);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("gateway", wLocalBuffer, strlen("gateway"))) {
            _CopyRightValue(gCommonCfg.nGateway, pRightValue, 15);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("dns2", wLocalBuffer, strlen("dns2"))) {
            _CopyRightValue(gCommonCfg.nDns2, pRightValue, 15);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("dns", wLocalBuffer, strlen("dns"))) {
            _CopyRightValue(gCommonCfg.nDns, pRightValue, 15);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("edsAddr2", wLocalBuffer, strlen("edsAddr2"))) {
            _CopyRightValue(gCommonCfg.nEdsAddr2, pRightValue, 255);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("edsAddr", wLocalBuffer, strlen("edsAddr"))) {
            _CopyRightValue(gCommonCfg.nEdsAddr, pRightValue, 255);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("ntpServer2", wLocalBuffer, strlen("ntpServer2"))) {
            _CopyRightValue(gCommonCfg.nNtpServer2, pRightValue, 31);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("ntpServer", wLocalBuffer, strlen("ntpServer"))) {
            _CopyRightValue(gCommonCfg.nNtpServer, pRightValue, 31);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("IGMPVersion", wLocalBuffer, strlen("IGMPVersion"))) {
            gCommonCfg.nIGMPVersion = atoi(pRightValue);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("DirectPlay", wLocalBuffer, strlen("DirectPlay"))) {
            gCommonCfg.nDirectPlay = atoi(pRightValue);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("WatchDogSwitch", wLocalBuffer, strlen("WatchDogSwitch"))) {
            gCommonCfg.nWatchDogSwitch = atoi(pRightValue);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
#ifdef INCLUDE_TR069
        } else if(!strncmp("TMSUsername", wLocalBuffer, strlen("TMSUsername"))) {
            _CopyRightValue(gCommonCfg.nTMSUsername, pRightValue, 256);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("TMSPassword", wLocalBuffer, strlen("TMSPassword"))) {
            _CopyRightValue(gCommonCfg.nTMSPassword, pRightValue, 256);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("TMSHeartBitInterval", wLocalBuffer, strlen("TMSHeartBitInterval"))) {
            gCommonCfg.nTMSHeartBitInterval = atoi(pRightValue);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("TMSHeartBit", wLocalBuffer, strlen("TMSHeartBit"))) {
            gCommonCfg.nTMSHeartBit = atoi(pRightValue);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
#endif
        } else if(!strncmp("TransportProtocol", wLocalBuffer, strlen("TransportProtocol"))) {
            gCommonCfg.nTransportProtocol = atoi(pRightValue);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("ChannelSwitch", wLocalBuffer, strlen("ChannelSwitch"))) {
            gCommonCfg.nChannelSwitch = atoi(pRightValue);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("VideoOutput", wLocalBuffer, strlen("VideoOutput"))) {
            gCommonCfg.nVideoOutput = atoi(pRightValue);
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("standardMode", wLocalBuffer, strlen("standardMode"))) {
            _CopyRightValue(tempstr, pRightValue, 63);
            if(!strcasecmp(tempstr, "pal")) {
                gCommonCfg.nStandardMode = VideoFormat_PAL;
            } else if(!strcasecmp(tempstr, "ntsc")) {
                gCommonCfg.nStandardMode = VideoFormat_NTSC ;
            }
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("HDMode", wLocalBuffer, strlen("HDMode"))) {
            _CopyRightValue(tempstr, pRightValue, 63);
            if(!strcasecmp(tempstr, "pal")) {
                gCommonCfg.nHDMode = VideoFormat_PAL;
            } else if(!strcasecmp(tempstr, "ntsc")) {
                gCommonCfg.nHDMode = VideoFormat_NTSC ;
            } else if(!strcasecmp(tempstr, "480p")) {
                gCommonCfg.nHDMode = VideoFormat_480P ;
            } else if(!strcasecmp(tempstr, "576p")) {
                gCommonCfg.nHDMode = VideoFormat_576P ;
            } else if(!strcasecmp(tempstr, "720p-60hz")) {
                gCommonCfg.nHDMode = VideoFormat_720P60HZ ;
            } else if(!strcasecmp(tempstr, "720p-50hz")) {
                gCommonCfg.nHDMode = VideoFormat_720P50HZ ;
            } else if(!strcasecmp(tempstr, "1080i-60hz")) {
                gCommonCfg.nHDMode = VideoFormat_1080I60HZ ;
            } else if(!strcasecmp(tempstr, "1080i-50hz")) {
                gCommonCfg.nHDMode = VideoFormat_1080I50HZ ;
            } else if(!strcasecmp(tempstr, "1080p-30hz")) {
                gCommonCfg.nHDMode = VideoFormat_1080P30HZ ;
            } else if(!strcasecmp(tempstr, "1080p-25hz")) {
                gCommonCfg.nHDMode = VideoFormat_1080P25HZ ;
            }

            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        } else if(!strncmp("connecttype", wLocalBuffer, strlen("connecttype"))) {
            int	nNetMode =  atoi(pRightValue);
            gCommonCfg.nNetMode = nNetMode;
            memset(wLocalBuffer, 0, 528);
            wFullErr = 0;
            continue;
        }else {
            LogUDiskDebug("this is param is unkown\n");
        }
        memset(wLocalBuffer, 0, 528);
    }
    gCommonCfg.nFlag = 1;
    fclose(fCommonCfg);
    gCommonParserFlag = 1;
    LogUDiskDebug("wFullErr[%d] wModFlag[%d]\n", wFullErr, wModFlag);
    if(wFullErr)
        return -1;
    else if(wModFlag == -1)
        return -2;
    else
        return 0;
}/*}}}*/

static int _ParseUserConfigData(const char* filepath)
{/*{{{*/
    LogUDiskDebug("parse user encrypt data\n");
    FILE *fUserCfg = NULL;
    char wLocalBuffer[528];
    char wTempBuffer[528];
    char *pRightValue = NULL;
    char *p = NULL;
    int flag = 0;
    int ret = 0;
    UserConfig_s* pUserCfg = &gUserCfg;

    fUserCfg = fopen(filepath, "rb");
    if(!fUserCfg) {
        LogUDiskDebug("open config info file failed\n");
        return -2;
    }
    memset(pUserCfg, 0, sizeof(pUserCfg));
    while(fgets(wTempBuffer, 528, fUserCfg)) {
        //åŽ»æŽ‰fgetsçš„æœ€åŽä¸€ä¸ªæ¢è¡Œç¬¦ "\r\n", å…¼å®¹\n
        p = strstr(wTempBuffer, "\r\n");
        if(p == NULL) {
            p = strstr(wTempBuffer, "\n");
            if(p == NULL)
                continue;
            if(p != NULL)
                *p = '\0';
        } else {
            *p = '\0';
        }

        memset(wLocalBuffer, 0, 528);
        ret = _Decrypt3DES(wTempBuffer, wLocalBuffer);
        if(ret == -1)
            continue;

        pRightValue = strstr(wLocalBuffer, "=");
        if(pRightValue != NULL)
            pRightValue++;
        else {
            memset(wTempBuffer, 0, 528);
            continue;
        }

        if(*(pRightValue) == 0x20)
            pRightValue++;

        if(!strncmp("ntvuseraccount", wLocalBuffer, sizeof("ntvuseraccount") - 1))
            _CopyRightValue(pUserCfg->nNtvUserAccount, pRightValue, USER_LEN - 1);
        else if(!strncmp("ntvuserpassword", wLocalBuffer, sizeof("ntvuserpassword") - 1))
            _CopyRightValue(pUserCfg->nNtvUserPassword, pRightValue, USER_LEN + 4 - 1);
        else if(!strncmp("netuseraccount", wLocalBuffer, sizeof("netuseraccount") - 1))
            _CopyRightValue(pUserCfg->nNetUserAccount, pRightValue, USER_LEN - 1);
        else if(!strncmp("netuserpassword", wLocalBuffer, sizeof("netuserpassword") - 1))
            _CopyRightValue(pUserCfg->nNetUserPassword, pRightValue, USER_LEN - 1);
        else if(!strncmp("connecttype", wLocalBuffer, sizeof("connecttype") - 1))
            pUserCfg->nConnectType = atoi(pRightValue);
        else if(!strncmp("stbIP", wLocalBuffer, sizeof("stbIP") - 1))
            _CopyRightValue(pUserCfg->nStbIP, pRightValue, 15);
        else if(!strncmp("netmask", wLocalBuffer, sizeof("netmask") - 1))
            _CopyRightValue(pUserCfg->nNetmask, pRightValue, 15);
        else if(!strncmp("gateway", wLocalBuffer, sizeof("gateway") - 1))
            _CopyRightValue(pUserCfg->nGateway, pRightValue, 15);
        else if(!strncmp("dns2", wLocalBuffer, sizeof("dns2") - 1))
            _CopyRightValue(pUserCfg->nDns2, pRightValue, 15);
        else if(!strncmp("dns", wLocalBuffer, sizeof("dns") - 1))
            _CopyRightValue(pUserCfg->nDns, pRightValue, 15);
        else
            LogUDiskDebug("error, this localbuf =%s=is unknow\n", wLocalBuffer);
        memset(wTempBuffer, 0, 528);
    }

    LogUDiskDebug("connecttype=%d, ntvuseraccount=%s, ntvuserpassword=%s, netuseraccount=%s, netuserpassword=%s\n",
        pUserCfg->nConnectType, pUserCfg->nNtvUserAccount, pUserCfg->nNtvUserPassword, pUserCfg->nNetUserAccount, pUserCfg->nNetUserPassword);

    if(pUserCfg->nConnectType < 1 || pUserCfg->nConnectType > 3)
        flag = -1;
    if(strlen(pUserCfg->nNtvUserAccount) <= 0)
        flag = -1;
    if(pUserCfg->nConnectType == 1 && strlen(pUserCfg->nNetUserAccount) <= 0)
        flag = -1;

    fclose(fUserCfg);
    return flag;
}/*}}}*/

int UDiskReadAccountConfigData(void)
{/*{{{*/
    LogUDiskDebug("read account.ini file.\n");
    if(!gParseAccountInited) {
        char wAccountCfg[64] = { 0 };
        sprintf(wAccountCfg, "/mnt/usb%d/usbconfig/account.ini", UDiskGetMountNumber());
        if(!access(wAccountCfg, R_OK | F_OK)) {
            if(!_ParseAccountConfigData(wAccountCfg))
                gParseAccountInited = 1;
        }
    }
    LogUDiskDebug("user account num : [%d] \n", gUserNum);
    return gUserNum;
}/*}}}*/

int UDiskReadCommonConfigData(void)
{/*{{{*/
    LogUDiskDebug("read common.cfg file.\n");
    if(!gCommonCfg.nFlag) {
        char wCommonCfg[64] = { 0 };
        sprintf(wCommonCfg, "/mnt/usb%d/usbconfig/common.cfg", UDiskGetMountNumber());
        if(access(wCommonCfg, R_OK | F_OK) < 0) {
            LogUDiskDebug("no this file! : COMON_FILE \n");
            return -1;
        }
        return _ParseCommonConfigData(wCommonCfg);
    }
    return 0;
}/*}}}*/

int UDiskReadUserConfigData(const char* pUserAccount)
{/*{{{*/
    int ret = 0;
    AccountConfig_s* pAccountCfg = (NULL != gAccountCfgLinkHead) ? gAccountCfgLinkHead->pNext : NULL;
    if(!pUserAccount || !pAccountCfg || !(pAccountCfg->pNext)) {
        LogUDiskDebug("error : pAccountCfg->pNext is NULL\n");
        return -2;
    }
    LogUDiskDebug("account is %s\n", pUserAccount);
    while (pAccountCfg) {
        LogUDiskDebug("account = %s, user = %s, filenameLen =%d\n", pAccountCfg->nAccount, pAccountCfg->nUser, strlen(pUserAccount));
        if((strlen(pAccountCfg->nAccount)) && (!strcmp(pUserAccount, pAccountCfg->nAccount))) {
            char wUserCfg[64] = { 0 };
            sprintf(wUserCfg, "/mnt/usb%d/usbconfig/%s.cfg", UDiskGetMountNumber(), pAccountCfg->nAccount);
            LogUDiskDebug("read %s.cfg file\n", pAccountCfg->nAccount);
            ret = _ParseUserConfigData(wUserCfg);
            if(-1 == ret) {
                LogUDiskWarn("this key file is error\n");
                return -1;
            } else if(-2 == ret) {
                LogUDiskWarn("there is no this file\n");
                return -2;
            }
            gUserCfg.nFlag = 1;
            break;
        }
        pAccountCfg = pAccountCfg->pNext;
    }
    return 0;
}/*}}}*/

AccountConfig_s* UDiskGetUserConfigByIndex(int idx)
{/*{{{*/
    AccountConfig_s* pAccountCfg = gAccountCfgLinkHead;
    if(idx >= gUserNum || idx < 0)
        return NULL;
    LogUDiskDebug("idx [%d]\n", idx);
    idx++;
    for(int i = 0; i < idx && pAccountCfg; i++) {
        pAccountCfg = pAccountCfg->pNext;
        LogUDiskDebug("account[%d] = [%s]\n", i, pAccountCfg->nAccount);
    }

    return pAccountCfg;
}/*}}}*/

int UDiskGetUserConfigByUserID(const char* pUserAccount)
{/*{{{*/
    if(!gAccountCfgLinkHead)
        return -1;

    AccountConfig_s* pAccountCfg = (NULL != gAccountCfgLinkHead) ? gAccountCfgLinkHead->pNext : NULL;
    int wIdx = 0;

    while (pAccountCfg) {
        if (!strcmp(pAccountCfg->nAccount, pUserAccount)) {
            LogUDiskDebug("DO find UserAccount, Index:%d, Account:%s\n", wIdx, pAccountCfg->nAccount);
            return wIdx;
        }
        wIdx++;
        pAccountCfg = pAccountCfg->pNext;
    }

    LogUDiskError("DON'T find UserAccount(%s)\n", pUserAccount);

    return -1;
}/*}}}*/

int UDiskChanageUserStatus(const char* account)
{/*{{{*/
    if(!account)
        return -1;
    char wAccountField[64] = { 0 };
    snprintf(wAccountField, 63, "[%s]", account);
    int  wRcount = 0, wLength = 0;
    FILE* fAccountCfg = NULL;
    char wLocalBuffer[4 * 1024];
    char *pTmp = NULL;
    //åœ¨è¿™é‡Œé‡æ–°æ‰“å¼€æ–‡ä»¶,ç›®çš„æ˜¯ä¿®æ”¹Uç›˜ä¸­account.iniä¸­çš„initializeéŸ¿
    //sleep(1);
    char wAccountCfg[64] = { 0 };
    sprintf(wAccountCfg, "/mnt/usb%d/usbconfig/account.ini", UDiskGetMountNumber());
    fAccountCfg = fopen(wAccountCfg, "r+");
    if(!fAccountCfg)
        return -1;

    int flag = 0;
    memset(wLocalBuffer, 0, 4 * 1024);
    while((wRcount = fread(wLocalBuffer, sizeof(char), 4 * 1024, fAccountCfg)) > 0) {
        wLength = wRcount;
        pTmp = strstr(wLocalBuffer, wAccountField);
        if(!pTmp) {
            LogUDiskDebug("should continue to read more data.");
            memset(wLocalBuffer, 0, 4 * 1024);
            continue;
        } else {
            flag = 1;
            LogUDiskDebug("ok,find this accout in account.ini\n");
            break;
        }
    }
    if(!flag) {
        LogUDiskDebug("error,there is not ths accout in accout.ini");
        fclose(fAccountCfg);
        return -1;
    }

    if(pTmp)
        pTmp = strstr(pTmp, "initialize");
    if(!pTmp) {
        LogUDiskDebug("account.ini file error!\n");
        fclose(fAccountCfg);
        return -1;
    }

    *(pTmp + 11) = '1';
    fseek(fAccountCfg, 0 - wLength, SEEK_CUR);
    fwrite(wLocalBuffer, sizeof(char), wLength, fAccountCfg);
    fclose(fAccountCfg);
    sync();
    sleep(1);
    return 0;
}/*}}}*/

void UDiskSetUserConfigData()
{
    LogUDiskDebug("set and save user config data\n");
    if(gUserCfg.nFlag == 0) {
        LogUDiskDebug("the ***.cfg file is not know\n");
        return ;
    }
    int len = 0;
    //Below Must
    sysSettingSetInt("connecttype", gUserCfg.nConnectType);
    appSettingSetString("ntvuser", gUserCfg.nNtvUserAccount);
    appSettingSetString("ntvAESpasswd", gUserCfg.nNtvUserPassword);

    len = strlen(gUserCfg.nNetUserAccount);
    if (0 < len && len < 33) {
        sysSettingSetString("netuser", gUserCfg.nNetUserAccount);
        if (2 == gUserCfg.nConnectType)
            sysSettingSetString("netAESpasswd", gUserCfg.nNetUserPassword);
    }
    len = strlen(gUserCfg.nNetUserPassword);
    if (0 < len && len < 32) {
        sysSettingSetString("netAESpasswd", gUserCfg.nNetUserPassword);
        if (2 == gUserCfg.nConnectType)
            sysSettingSetString("ipoeAESpasswd", gUserCfg.nNetUserPassword);
    }

    //Below Optional
    len = strlen(gUserCfg.nStbIP);
    if (0 < len && len < 16)
        sysSettingSetString("ip", gUserCfg.nStbIP);
    len = strlen(gUserCfg.nNetmask);
    if (0 < len && len < 16)
        sysSettingSetString("netmask", gUserCfg.nNetmask);
    len = strlen(gUserCfg.nGateway);
    if (0 < len && len < 16)
        sysSettingSetString("gateway", gUserCfg.nGateway);
    len = strlen(gUserCfg.nDns);
    if (0 < len && len < 16)
        sysSettingSetString("dns", gUserCfg.nDns);
    len = strlen(gUserCfg.nDns2);
    if (0 < len && len < 16)
        sysSettingSetString("dns1", gUserCfg.nDns2);
}

void UDiskSetCommonConfigData()
{/*{{{*/
    LogUDiskDebug("set and save common config data\n");
    if(gCommonCfg.nFlag != 1) {
        char wCommonCfg[64] = { 0 };
        sprintf(wCommonCfg, "/mnt/usb%d/usbconfig/common.cfg", UDiskGetMountNumber());
        if(access(wCommonCfg, R_OK | F_OK) < 0) {
            LogUDiskDebug("no this file! : COMON_FILE \n");
            return;
        }
        _ParseCommonConfigData(wCommonCfg);
    }

    if(gCommonCfg.nFlag != 1) {
        LogUDiskDebug("the common file is not init\n");
        return;
    }
    if('\0' != gCommonCfg.nEdsAddr[0] && (strncmp(gCommonCfg.nEdsAddr, "http://", 7) == 0))
        sysSettingSetString("eds", gCommonCfg.nEdsAddr);
    if('\0' != gCommonCfg.nEdsAddr2[0] && (strncmp(gCommonCfg.nEdsAddr, "http://", 7) == 0))
        sysSettingSetString("eds1", gCommonCfg.nEdsAddr2);
    if('\0' != gCommonCfg.nNtpServer[0])
        sysSettingSetString("ntp", gCommonCfg.nNtpServer);
    if('\0' != gCommonCfg.nNtpServer2[0])
        sysSettingSetString("ntp1", gCommonCfg.nNtpServer2);
    if(gCommonCfg.nStandardMode != 0)
	    sysSettingSetInt("videoformat", gCommonCfg.nStandardMode);
    if(gCommonCfg.nHDMode != 0)
	    sysSettingSetInt("hd_video_format", gCommonCfg.nHDMode);
    if(gCommonCfg.nNetMode >= 0)
        sysSettingSetInt("connecttype", gCommonCfg.nNetMode);
    if('\0' != gCommonCfg.nStbIP[0])
        sysSettingSetString("ip", gCommonCfg.nStbIP);
    if('\0' != gCommonCfg.nNetmask[0])
        sysSettingSetString("netmask", gCommonCfg.nNetmask);
    if('\0' != gCommonCfg.nGateway[0])
        sysSettingSetString("gateway", gCommonCfg.nGateway);
    if('\0' != gCommonCfg.nDns[0])
        sysSettingSetString("dns", gCommonCfg.nDns);
    if('\0' != gCommonCfg.nDns2[0])
        sysSettingSetString("dns1", gCommonCfg.nDns2);
    if('\0' != gCommonCfg.nNetuseraccount[0])
        sysSettingSetString("netuser", gCommonCfg.nNetuseraccount);
    if('\0' != gCommonCfg.nNetuserpassword[0])
        sysSettingSetString("netAESpasswd", gCommonCfg.nNetuseraccount);
    if('\0' != gCommonCfg.nDefContAcc[0])
        sysSettingSetString("netuser", gCommonCfg.nDefContAcc);
    if('\0' != gCommonCfg.nDefContPwd[0])
        sysSettingSetString("netAESpasswd", gCommonCfg.nDefContPwd);
    if('\0' != gCommonCfg.nNtvuseraccount[0])
        appSettingSetString("ntvuser", gCommonCfg.nNtvuseraccount);
    if('\0' != gCommonCfg.nNtvuserpassword[0])
        appSettingSetString("ntvAESpasswd", gCommonCfg.nNtvuserpassword);
    if(gCommonCfg.nWatchDogSwitch >= 0)
        ;
    if(gCommonCfg.nIGMPVersion >= 0)
        appSettingSetInt("igmpversion", gCommonCfg.nIGMPVersion);
    if(gCommonCfg.nDirectPlay >= 0)
        sysSettingSetInt("lastChannelPlay", gCommonCfg.nDirectPlay);
    if(gCommonCfg.nTransportProtocol >= 0)
        sysSettingSetInt("TransportProtocol", gCommonCfg.nTransportProtocol);
    if(gCommonCfg.nChannelSwitch >= 0)
        sysSettingSetInt("changevideomode", gCommonCfg.nChannelSwitch);
    if(gCommonCfg.nVideoOutput >= 0)
        appSettingSetInt("hd_aspect_mode", gCommonCfg.nVideoOutput);
#ifdef INCLUDE_TR069
    if(gCommonCfg.nTMSHeartBit >= 0)
        TR069_API_SETVALUE("Param.PeriodicInformEnable", NULL, gCommonCfg.nTMSHeartBit);
    if(gCommonCfg.nTMSHeartBitInterval >= 0)
        TR069_API_SETVALUE("Param.PeriodicInformInterval", NULL, gCommonCfg.nTMSHeartBitInterval);
    if('\0' != gCommonCfg.nTMSUsername[0])
        TR069_API_SETVALUE("Param.Username", gCommonCfg.nTMSUsername, 0);
    if('\0' != gCommonCfg.nTMSPassword[0])
        TR069_API_SETVALUE("Param.Password", gCommonCfg.nTMSPassword, 0);
#endif
}

} // End Hippo



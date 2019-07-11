
#include "Business.h"
#include "Assertions.h"
#include "ntp/mid_ntp.h"
#include "mid/mid_tools.h"
#include "mid_sys.h"
#include "ind_mem.h"
#include "cryptoFunc.h"
#include "charConvert.h"
#include "sys_basic_macro.h"
#include <openssl/des.h>
#include "Account.h"

#include "NetworkFunctions.h"
#include "AppSetting.h"
#include <string.h>
#include <stdlib.h>

#if defined (HUAWEI_C10)
int Business::s_leftrightkeyFlag = 0;
#elif defined (HUAWEI_C20)
int Business::s_leftrightkeyFlag = 1;
#else
should define s_leftrightkeyFlag.
#endif

int Business::s_joinFlag = 0;
char Business::s_serviceMode[10] = "";
int Business::s_EPGReadyFlag = 0; // 1:ready;0:no ready
char Business::s_encryToken[36] = {0};

static Business g_business;

Business::Business()
{
}

Business::~Business()
{
}

int
Business::changeUrlFormatFromDomainToIp(char *doMainUrl, int maxLen)
{
#ifdef Huawei_v5
    char url[LARGE_URL_MAX_LEN + 4] = {0};
    char tmp[LARGE_URL_MAX_LEN + 4] = {0};
    unsigned int hostip = INADDR_NONE;
    char *pstr = NULL, *pstr1 = NULL;

    if(strlen(doMainUrl) && maxLen <= LARGE_URL_MAX_LEN + 4) {
        strncpy(url, doMainUrl, maxLen);
        pstr = strchr(url, '/');//http://
        if(pstr == NULL) {
            LogSafeOperError("url format error: url =%s\n", url);
            return -1;
        }

        while(*pstr == '/') {//http://
            pstr++;
        }
        if(pstr == NULL) {
            LogSafeOperError("url =%s\n", url);
            return -1;
        }

        pstr1 = strchr(pstr, ':');//http://domain:port/path
        if(pstr1 == NULL) {
            pstr1 = strchr(pstr, '/');//http://domain/path
            if(pstr1 == NULL)
                pstr1 = url + strlen(url);//http://domain
        }
        strncpy(tmp, pstr, pstr1 - pstr);
        hostip = inet_addr(tmp);
        if (hostip == INADDR_ANY || hostip == INADDR_NONE) {
            if (!mid_dns_gethost(tmp, &hostip, 10)) {
                memset(tmp, 0, LARGE_URL_MAX_LEN + 4);
                mid_tool_addr2string(hostip, tmp);
                LogSafeOperDebug("tmp ip=[%s]\n", tmp);
                snprintf(doMainUrl, maxLen, "http://%s%s", tmp, pstr1);
                LogSafeOperDebug("ip url=[%s]\n", doMainUrl);
                return 0;
            } else {
                LogSafeOperError("change domain to ip error: url =%s\n", url);
                return -1;
            }
        }
        LogSafeOperDebug("the url is ip format[%s], don't need change!\n", doMainUrl);
        return -1;
    }
    LogSafeOperError("url length is 0[%d] or maxlen[%d] is too long[%d]!\n", strlen(doMainUrl), maxLen, LARGE_URL_MAX_LEN + 4);

#endif
    return -1;
}

int
Business::setEncryToken(const char *buf)
{
    int len = 0;

    if(NULL == buf)
        ERR_OUT("EncryToken is NULL.\n");
    len = strlen(buf);
    len = ((len > 35) ? 35 : len);
    strncpy(s_encryToken , buf , len);
    s_encryToken[len] = '\0';
    return 0;
Err:
    return -1;

}

char*
Business::getEncryToken()
{
    return s_encryToken;
}

/**
 * @brief getCTCAuthInfo get encrypto authentication information ( China Telecom Corporation )
 *
 * @note  Modified from 'getCTCAuthInfo' since subversion 1635
 *
 * @param nOutStr pointer to encrypto string of auth
 * @param nSize the size of nOutStr
 *
 * @return pointer of nOut.  Mod.by Michael
 */
char*
Business::getCTCAuthInfo(char* nOutStr, int nSize)
{
    if(!nOutStr || !nSize)
        LogSafeOperDebug("param invalid\n");

    // 1. concat the authentication string
    int  lLength = 0;
    char lStrText[EPG_PASSWD_LEN] = { 0 };
    char lRandom[9] = { '\n' };
    char lNtvID[USER_LEN]  = { 0 };
    char lStbID[USER_LEN]  = { 0 };
    char lMacAdr[USER_LEN] = { 0 };

    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);

    LogSafeOperDebug("stb read the auth info ,begin to auth\n");
    srand(time(NULL));
    sprintf(lRandom, "%u", rand() % 100000000);

    if(appSettingGetString("ntvuser", lNtvID, USER_LEN, 0) < 0) {
        LogSafeOperDebug("get ntv user id\n");
        return NULL;
    }
    mid_sys_serial(lStbID);

    LogSafeOperDebug("auth info concatenation\n");
    lLength = snprintf(lStrText, EPG_PASSWD_LEN, "%s$%s$%s$%s$%s$%s$$CTC", lRandom, getEncryToken(), lNtvID, lStbID,
        network_address_get(ifname, ifaddr, URL_LEN),
        network_tokenmac_get(lMacAdr, USER_LEN, ':'));

    if(lLength > nSize) {
        LogSafeOperDebug("buffer size is too small\n");
        return NULL;
    }

    // 2. encrypto the authentication string
    LogSafeOperDebug("auth info encrypto:%s\n", lStrText);
    char lPassword[USER_LEN] = {0};
    char encyptData[USER_LEN] = {0};
    char* cont = (char*)IND_MALLOC(USER_LEN);
    int lCodeLength = 0, lTextLength = 0, lTextRemainder = 0, lSegregation = 1, i = 0, j = 0;
    char lICKey[ ] = "unknown ICKey00000000000";
    DES_cblock lKey1, lKey2, lKey3;
    DES_key_schedule lKs1, lKs2, lKs3;
    DES_cblock lPlainText[40], lEncryptOut[40];

    char* lStrEncrypType = account().getEncryptionType();
    appSettingGetString("ntvAESpasswd", cont, USER_LEN, 0);

    if(lStrEncrypType) {
        if(!strncmp(lStrEncrypType, "0001", 4)) {
            LogSafeOperDebug("I will use shanghai to token\n");
            if(32 == strlen(cont)) {
                LogSafeOperDebug("The NTV password is 32 bytes, may be md5 !");
            }
            strcpy(lPassword, cont);
        }
        else if(!strncmp(lStrEncrypType, "0002", 4)) {
            LogSafeOperDebug("I will use huawei MD5 to token cont = %s, len = %d\n", cont, strlen(cont));
            if(32 == strlen(cont)) {
                strncpy(lPassword, cont, 8);  // get the first 8 characters of the md5 digest
            } else {
                md5Encypt(&cont, 1, encyptData, sizeof(encyptData), 1);
                data2Hex(encyptData, 16, encyptData, sizeof(encyptData));
                strncpy(lPassword, encyptData, 8);  // get the first 8 characters of the md5 digest
            }
        }
        else if(!strncmp(lStrEncrypType, "0003", 4)) {
            LogSafeOperDebug("I will use 163 MD5 to token\n");
            if(32 == strlen(cont)) {
                strncpy(lPassword , cont, 24); // get the first 24 characters of the md5 digest
            } else {
                md5Encypt(&cont, 1, encyptData, sizeof(encyptData), 0);
                data2Hex(encyptData, 16, encyptData, sizeof(encyptData));
                lower2Upper(encyptData, strlen(encyptData));
                strncpy(lPassword , encyptData, 24); // get the first 24 characters of the md5 digest
            }
        }
        else {
            LogSafeOperDebug("platformat is error, will use plain text token.\n");
            strcpy(lPassword, cont);
        }
    }
    else {
        LogSafeOperDebug("platformat is error, will use plain text token.\n");
        strcpy(lPassword, cont);
    }
    IND_FREE(cont);

    LogSafeOperDebug("Padding key to 24 Byte, if less than 24 bytes, must be filled 0.\n");
    lCodeLength = 24 - strlen(lPassword);
    if(lCodeLength > 0)
        for(i = 0; i < lCodeLength; ++i)
            strcat(lPassword, "0");

    LogSafeOperDebug("make original password string divided into tree groups.\n");
    if(lSegregation) {
        for(i = 0; i < 24; ++i) {
            switch(i / 8) {
                case 0: /* 0 - 7 */
                    lKey1[i % 8] = (unsigned char)lPassword[i];
                    break;
                case 1: /* 8 - 15 */
                    lKey2[i % 8] = (unsigned char)lPassword[i];
                    break;
                case 2: /* 16 - 23 */
                    lKey3[i % 8] = (unsigned char)lPassword[i];
                    break;
                default:
                    break;
            }
        }
    }
    else {
        for(i = 0; i < 24; ++i) {
            switch(i / 8) {
                case 0: /* 0 - 7 */
                    lKey1[i % 8] = (unsigned char)lICKey[i];
                    break;
                case 1: /* 8 - 16 */
                    lKey2[i % 8] = (unsigned char)lICKey[i];
                    break;
                case 2: /* 16 - 23 */
                    lKey3[i % 8] = (unsigned char)lICKey[i];
                    break;
                default:
                    break;
            }
        }
    }

    LogSafeOperDebug("PKCS5 Padding\n");
    lTextLength    = strlen(lStrText) / 8 + 1 ;
    lTextRemainder = strlen(lStrText) % 8;
    for(i = 0; i < lTextLength; ++i) {
        for(j = 0; j < 8; ++j)
            lPlainText[i][j] = 0;
    }
    for(i = 0; i < lTextLength; ++i) {
        for(j = 0; j < 8; ++j) {
            if(i == lTextLength - 1 && j >= lTextRemainder)
                lPlainText[i][j] = 0x08 - lTextRemainder; /* Padding bytes must be the multiple of 8 */
            else
                lPlainText[i][j] = lStrText[i * 8 + j];
        }
    }

    LogSafeOperDebug("Set key\n");
    DES_set_key_unchecked(&lKey1, &lKs1);
    DES_set_key_unchecked(&lKey2, &lKs2);
    DES_set_key_unchecked(&lKey3, &lKs3);

    LogSafeOperDebug("Encrypt start\n");
    for(i = 0; i < lTextLength; ++i)
        DES_ecb3_encrypt(&lPlainText[i], &lEncryptOut[i], &lKs1, &lKs2, &lKs3, DES_ENCRYPT);

    lLength = 0;
    for(i = 0; i < lTextLength; i++) {
        for(j = 0; j < 8; ++j)
            lLength += sprintf(nOutStr + lLength, "%02X", lEncryptOut[i][j]);
    }

    LogSafeOperDebug("Encrypt success\n");
    return nOutStr;
}

/**
 * @brief getCUAuthInfo get encrypto authentication information (China Unicom)
 *
 * @param nOutStr pointer to encrypto string of auth
 * @param nSize the size of nOutStr
 *
 * @return pointer of nOut.  Add. by Michael
 */
char*
Business::getCUAuthInfo(char* nOutStr, int nSize)
{
    if(!nOutStr || !nSize)
        LogSafeOperDebug("param invalid\n");

    // concat the authentication string
    int  lLength = 0;
    char lStrText[EPG_PASSWD_LEN] = { 0 };
    char lRandom[9] = { '\n' };
    char lNtvID[USER_LEN]  = { 0 };
    char lStbID[USER_LEN]  = { 0 };
    char lMacAdr[USER_LEN] = { 0 };

    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);

    LogSafeOperDebug("stb read the auth info ,begin to auth\n");
    srand(time(NULL));
    sprintf(lRandom, "%u", rand() % 100000000);

    if(appSettingGetString("ntvuser", lNtvID, USER_LEN, 0) < 0) {
        LogSafeOperDebug("get ntv user id\n");
        return NULL;
    }
    mid_sys_serial(lStbID);

    LogSafeOperDebug("auth info concatenation\n");
    lLength = snprintf(lStrText, EPG_PASSWD_LEN, "%s$%s$%s$%s$%s$%s$$CU", lRandom, getEncryToken(), lNtvID, lStbID,
        network_address_get(ifname, ifaddr, URL_LEN),
        network_tokenmac_get(lMacAdr, 32, ':'));

    if(lLength > nSize) {
        LogSafeOperDebug("buffer size is too small\n");
        return NULL;
    }

    // 2. encrypto the authentication string
    LogSafeOperDebug("auth info encrypto:%s\n", lStrText);
    char lPassword[USER_LEN] = {0};
    char encyptData[USER_LEN] = {0};
    char *cont = (char*)IND_MALLOC(USER_LEN);
    int lCodeLength = 0, lTextLength = 0, lTextRemainder = 0, lSegregation = 1, i = 0, j = 0;
    char lICKey[ ] = "unknown ICKey00000000000";
    DES_cblock lKey1, lKey2, lKey3;
    DES_key_schedule lKs1, lKs2, lKs3;
    DES_cblock lPlainText[40], lEncryptOut[40];

    char* lStrEncrypType = account().getEncryptionType();
    appSettingGetString("ntvAESpasswd", cont, USER_LEN, 0);

    if(lStrEncrypType) {
        if(!strncmp(lStrEncrypType, "0001", 4)) {
            LogSafeOperDebug("I will use shanghai to token\n");
            if(32 == strlen(cont)) {
                LogSafeOperDebug("The NTV password is 32 bytes, may be md5 !");
            }
            strcpy(lPassword, cont);
        }
        else if(!strncmp(lStrEncrypType, "0002", 4)) {
            LogSafeOperDebug("I will use huawei MD5 to token cont = %s, len = %d\n", cont, strlen(cont));
            if(32 == strlen(cont)) {
                strncpy(lPassword, cont, 8);  // get the first 8 characters of the md5 digest
            } else {
                md5Encypt(&cont, 1, encyptData, sizeof(encyptData), 1);
                data2Hex(encyptData, 16, encyptData, sizeof(encyptData));
                strncpy(lPassword, encyptData, 8);  // get the first 8 characters of the md5 digest
            }
        }
        else if(!strncmp(lStrEncrypType, "0003", 4)) {
            LogSafeOperDebug("I will use 163 MD5 to token\n");
            if(32 == strlen(cont)) {
                strncpy(lPassword , cont, 24); // get the first 24 characters of the md5 digest
            } else {
                md5Encypt(&cont, 1, encyptData, sizeof(encyptData), 0);
                data2Hex(encyptData, 16, encyptData, sizeof(encyptData));
                lower2Upper(encyptData, strlen(encyptData));
                strncpy(lPassword , encyptData, 24); // get the first 24 characters of the md5 digest
            }
        }
        else {
            LogSafeOperDebug("platformat is error, will use plain text token.\n");
            strcpy(lPassword, cont);
        }
    }
    else {
        LogSafeOperDebug("platformat is error, will use plain text token.\n");
        strcpy(lPassword, cont);
    }
    IND_FREE(cont);

    LogSafeOperDebug("Padding key to 24 Byte, if less than 24 bytes, must be filled 0.\n");
    lCodeLength = 24 - strlen(lPassword);
    if(lCodeLength > 0)
        for(i = 0; i < lCodeLength; ++i)
            strcat(lPassword, "0");

    LogSafeOperDebug("make original password string divided into tree groups.\n");
    if(lSegregation) {
        for(i = 0; i < 24; ++i) {
            switch(i / 8) {
                case 0: /* 0 - 7 */
                    lKey1[i % 8] = (unsigned char)lPassword[i];
                    break;
                case 1: /* 8 - 15 */
                    lKey2[i % 8] = (unsigned char)lPassword[i];
                    break;
                case 2: /* 16 - 23 */
                    lKey3[i % 8] = (unsigned char)lPassword[i];
                    break;
                default:
                    break;
            }
        }
    }
    else {
        for(i = 0; i < 24; ++i) {
            switch(i / 8) {
                case 0: /* 0 - 7 */
                    lKey1[i % 8] = (unsigned char)lICKey[i];
                    break;
                case 1: /* 8 - 16 */
                    lKey2[i % 8] = (unsigned char)lICKey[i];
                    break;
                case 2: /* 16 - 23 */
                    lKey3[i % 8] = (unsigned char)lICKey[i];
                    break;
                default:
                    break;
            }
        }
    }

    LogSafeOperDebug("PKCS5 Padding\n");
    lTextLength    = strlen(lStrText) / 8 + 1 ;
    lTextRemainder = strlen(lStrText) % 8;
    for(i = 0; i < lTextLength; ++i) {
        for(j = 0; j < 8; ++j)
            lPlainText[i][j] = 0;
    }
    for(i = 0; i < lTextLength; ++i) {
        for(j = 0; j < 8; ++j) {
            if(i == lTextLength - 1 && j >= lTextRemainder)
                lPlainText[i][j] = 0x08 - lTextRemainder; /* Padding bytes must be the multiple of 8 */
            else
                lPlainText[i][j] = lStrText[i * 8 + j];
        }
    }

    LogSafeOperDebug("Set key\n");
    DES_set_key_unchecked(&lKey1, &lKs1);
    DES_set_key_unchecked(&lKey2, &lKs2);
    DES_set_key_unchecked(&lKey3, &lKs3);

    LogSafeOperDebug("Encrypt start\n");
    for(i = 0; i < lTextLength; ++i)
        DES_ecb3_encrypt(&lPlainText[i], &lEncryptOut[i], &lKs1, &lKs2, &lKs3, DES_ENCRYPT);

    lLength = 0;
    for(i = 0; i < lTextLength; i++) {
        for(j = 0; j < 8; ++j)
            lLength += sprintf(nOutStr + lLength, "%02X", lEncryptOut[i][j]);
    }

    LogSafeOperDebug("Encrypt success\n");
    return nOutStr;
}

void
Business::setEDSJoinFlag(const int value)
{
    s_joinFlag = value;
    return;
}

/*************************************************
Description:获取joinflag标志位
Input:       无
output:     无
Return:     标志位
 *************************************************/
int
Business::getEDSJoinFlag(void)
{
    return s_joinFlag;
}

int
Business::setKeyCtrl(int flag)
{
    if(0 != flag && 1 != flag)
        ERR_OUT("The leftrightkey is Error!\n");

    if(s_leftrightkeyFlag != flag)
        s_leftrightkeyFlag = flag;
    return 0;
Err:
    return -1;
}

int
Business::getKeyCtrl()
{
    return s_leftrightkeyFlag;
}

void
Business::setEPGReadyFlag(int flag)
{
    s_EPGReadyFlag = flag;
}

int
Business::getEPGReadyFlag()
{
    return s_EPGReadyFlag;
}

void
Business::setServiceMode(char *newServiceMode)
{
    if(strcmp(s_serviceMode, newServiceMode) != 0) {
        strcpy(s_serviceMode, newServiceMode);
    }
}

char*
Business::getServiceMode()
{
    return s_serviceMode;
}


Business &business()
{
    return g_business;
}

/*************************************************
Description:设置joinflag标志
Input:       value:标志位
output:     无
Return:     无
 *************************************************/
void BusinessSetEDSJoinFlag(const int value)
{
    business().setEDSJoinFlag(value);
    return;
}

int BusinessGetKeyCtrl()
{
    return business().getKeyCtrl();
}

int BusinessGetEPGReadyFlag()
{
    return business().getEPGReadyFlag();
}

char* BusinessGetServiceMode()
{
    return business().getServiceMode();
}


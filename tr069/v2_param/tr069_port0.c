
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<sys/stat.h>

#include "tr069_port.h"
#include "tr069_define.h"
//#include "pathConfig.h"

#include "tr069_debug.h"

#include "app_aes.h"

static char *gInfoPath = NULL;


#ifdef ANDROID
/*------------------------------------------------------------------------------
    从flash中分配给TR069的区域读取参数配置信息
    目前 len 的长度恒定为 16K
 ------------------------------------------------------------------------------*/
int tr069_port_infoLoad(char *buf, int len)
{
    int l;
    FILE *fp;

    if (!gInfoPath)
        return -1;

    fp = fopen(gInfoPath, "rb");
    if (fp == NULL) {
        TR069Error("fopen\n");
        return -1;
    }
    l = fread(buf, 1, len, fp);
    fclose(fp);
    return l;
}

/*------------------------------------------------------------------------------
    将参数配置信息写到flash中分配给TR069的区域
    目前 len 的长度恒定为 16K
 ------------------------------------------------------------------------------*/
int tr069_port_infoSave(char *buf, int len)
{
    FILE *fp = NULL;

    if (!gInfoPath)
        return -1;

    fp = fopen(gInfoPath, "wb");
    if (fp == NULL) {
        TR069Error("fopen\n");
        return -1;
    }
    fwrite(buf, 1, len, fp);
    fclose(fp);
    sync();
    return 0;
}
#else
static int TmsChageAesPasswdToPlaintextPasswd(char *buf, int bufLen)
{
    FILE *fp = fopen(gInfoPath, "rb");
    if (!fp){
        TR069Debug("fopen error\n");
        return -1;
    }
    char lineBuf[1024] = {0};
    char *ptr = NULL;
    int infoLen = 0;
    int changeFlag = 0;
    while(1) {
        memset(lineBuf, 0, sizeof(lineBuf));
        if (!fgets(lineBuf, 1024, fp)) {
            TR069Debug("fread finish\n");
            break;
        }
        if (strstr(lineBuf, "<Param>"))
            changeFlag = 1;
        else if (strstr(lineBuf, "</Param>"))
            changeFlag = 0;

        if (changeFlag) {
            if (strstr(lineBuf, "AESPassword")){//AESPassword to Password
                ptr = lineBuf + strlen("AESPassword") + 1;
                char* enterPtr = strrchr(lineBuf, '\n');
                if (enterPtr)
                    *enterPtr = 0; //remove '\n' for app_aes_decrypt (input:32bit or n*32bit)
                char plaintext[128] = {0};
                if (strlen(ptr))
                    app_aes_decrypt(ptr, plaintext);
                memset(lineBuf, 0, sizeof(lineBuf));
                snprintf(lineBuf, 1024, "Password=%s\n", plaintext);
            }
            else if (strstr(lineBuf, "AESConnectionRequestPassword")){//AESConnectionRequestPassword to ConnectionRequestPassword
                ptr = lineBuf + strlen("AESConnectionRequestPassword") + 1;
                char* enterPtr = strrchr(lineBuf, '\n');
                if (enterPtr)
                    *enterPtr = 0; //remove '\n' for app_aes_decrypt (input:32bit or n*32bit)
                char plaintext[128] = {0};
                if (strlen(ptr))
                    app_aes_decrypt(ptr, plaintext);
                memset(lineBuf, 0, sizeof(lineBuf));
                snprintf(lineBuf, 1024, "ConnectionRequestPassword=%s\n", plaintext);
            }
            else if (strstr(lineBuf, "AESSTUNPassword")){//AESSTUNPassword to STUNPassword
                ptr = lineBuf + strlen("AESSTUNPassword") + 1;
                char* enterPtr = strrchr(lineBuf, '\n');
                if (enterPtr)
                    *enterPtr = 0; //remove '\n' for app_aes_decrypt (input:32bit or n*32bit)
                char plaintext[128] = {0};
                if (strlen(ptr))
                    app_aes_decrypt(ptr, plaintext);
                memset(lineBuf, 0, sizeof(lineBuf));
                snprintf(lineBuf, 1024, "STUNPassword=%s\n", plaintext);
            } else {

                if(!strncmp(lineBuf, "Password", 8)){
                    ptr = lineBuf + strlen("Password") + 1;
                    char* enterPtr = strrchr(lineBuf, '\n');
                    if (enterPtr)
                        *enterPtr = 0; //remove '\n' for app_aes_decrypt (input:32bit or n*32bit)
                    char plaintext[128] = {0};
                    if(strlen(ptr) > 16 && strlen(ptr) % 16 == 0)
                        app_aes_decrypt(ptr, plaintext);
                    else
                        memcpy(plaintext, ptr, 128);
                    memset(lineBuf, 0, sizeof(lineBuf));
                    snprintf(lineBuf, 1024, "Password=%s\n", plaintext);
                }
                else if(!strncmp(lineBuf, "ConnectionRequestPassword", 25)){
                    ptr = lineBuf + strlen("ConnectionRequestPassword") + 1;
                    char* enterPtr = strrchr(lineBuf, '\n');
                    if (enterPtr)
                        *enterPtr = 0; //remove '\n' for app_aes_decrypt (input:32bit or n*32bit)
                    char plaintext[128] = {0};
                    if(strlen(ptr) > 16 && strlen(ptr) % 16 == 0)
                        app_aes_decrypt(ptr, plaintext);
                    else
                        memcpy(plaintext, ptr, 128);
                    memset(lineBuf, 0, sizeof(lineBuf));
                    snprintf(lineBuf, 1024, "ConnectionRequestPassword=%s\n", plaintext);
                }
                else if(!strncmp(lineBuf, "STUNPassword", 12)){
                    ptr = lineBuf + strlen("STUNPassword") + 1;
                    char* enterPtr = strrchr(lineBuf, '\n');
                    if (enterPtr)
                        *enterPtr = 0; //remove '\n' for app_aes_decrypt (input:32bit or n*32bit)
                    char plaintext[128] = {0};
                    if(strlen(ptr) > 16 && strlen(ptr) % 16 == 0)
                        app_aes_decrypt(ptr, plaintext);
                    else
                        memcpy(plaintext, ptr, 128);
                    memset(lineBuf, 0, sizeof(lineBuf));
                    snprintf(lineBuf, 1024, "STUNPassword=%s\n", plaintext);
                }

            }
        }

        if (infoLen + strlen(lineBuf) < bufLen) {
            sprintf(buf + infoLen, "%s", lineBuf);
            infoLen = strlen(buf);
        } else {
            perror("error, bufLen is not large enough\n");
            break;
        }
    }// while
    fclose(fp);
    return infoLen;
}

static int TmsChangePlaintextPasswdToAesPasswd(char *buf, int bufLen)
{
    FILE *fp = fopen(gInfoPath, "rb");
    if (!fp) {
        TR069Debug("fopen error\n");
        return -1;
    }
    char lineBuf[1024] = {0};
    char *ptr = NULL;
    int infoLen = 0;
    int changeFlag = 0;
    while(1) {
        memset(lineBuf, 0, sizeof(lineBuf));
        if (!fgets(lineBuf, 1024, fp)) {
            TR069Debug("fread finish\n");
            break;
        }

        if (strstr(lineBuf, "<Param>"))
            changeFlag = 1;
        else if (strstr(lineBuf, "</Param>"))
            changeFlag = 0;

        if (changeFlag) {
            if (strstr(lineBuf, "STUNPassword")){//STUNPassword -> AESSTUNPassword
                ptr = lineBuf + strlen("STUNPassword") + 1;
                char* enterPtr = strrchr(lineBuf, '\n');
                if (enterPtr)
                    *enterPtr = 0; //remove '\n' for app_aes_encrypt (input:32bit or n*32bit)
                char ciphertext[128] = {0};
                if (strlen(ptr) > 0 && strlen(ptr) < 32)
                    app_aes_encrypt(ptr, ciphertext);
                else
                    memcpy(ciphertext, ptr, 128);
                memset(lineBuf, 0, sizeof(lineBuf));
                snprintf(lineBuf, 1024, "AESSTUNPassword=%s\n", ciphertext);
            }
            else if (strstr(lineBuf, "ConnectionRequestPassword")){//ConnectionRequestPassword -> AESConnectionRequestPassword
                ptr = lineBuf + strlen("ConnectionRequestPassword") + 1;
                char* enterPtr = strrchr(lineBuf, '\n');
                if (enterPtr)
                    *enterPtr = 0; //remove '\n' for app_aes_encrypt (input:32bit or n*32bit)
                char ciphertext[128] = {0};
                if (strlen(ptr) > 0 && strlen(ptr) < 32)
                    app_aes_encrypt(ptr, ciphertext);
                else
                    memcpy(ciphertext, ptr, 128);
                memset(lineBuf, 0, sizeof(lineBuf));
                snprintf(lineBuf, 1024, "AESConnectionRequestPassword=%s\n", ciphertext);
            }
            else if (strstr(lineBuf, "Password")){//Password -> AESPassword
                ptr = lineBuf + strlen("Password") + 1;
                char* enterPtr = strrchr(lineBuf, '\n');
                if (enterPtr)
                    *enterPtr = 0; //remove '\n' for app_aes_encrypt (input:32bit or n*32bit)
                char ciphertext[128] = {0};
                if (strlen(ptr) > 0 && strlen(ptr) < 32 )
                    app_aes_encrypt(ptr, ciphertext);
                else
                    memcpy(ciphertext, ptr, 128);
                memset(lineBuf, 0, sizeof(lineBuf));
                snprintf(lineBuf, 1024, "AESPassword=%s\n", ciphertext);
            }
        }

        if (infoLen + strlen(lineBuf) < bufLen) {
            sprintf(buf + infoLen, "%s", lineBuf);
            infoLen = strlen(buf);
        } else {
            perror("2error, bufLen is not large enough\n");
            break;
        }
    }//while
    fclose(fp);
    return infoLen;
}

int TmsPlaintextPasswdRestore()
{
    if (!gInfoPath)
        return -1;

    char buf[10240] = {0};
    int infoLen = TmsChageAesPasswdToPlaintextPasswd(buf, 10240);
    if (infoLen == -1) {
        perror("TmsChangePlaintextPasswdToAesPasswd error\n");
        return -1;
    }

    mode_t oldMask = umask(0077);
    FILE *fp = fopen(gInfoPath, "w");
    umask(oldMask);
    if (!fp){
        perror("fopen11 error\n");
        return -1;
    }
    fwrite(buf, infoLen, 1, fp);
    //fsync(fp); 2013-8-2 11:50:28 by liujianhua fsync的参数是int不是FILE
    fclose(fp);

    return 0;
}

static int TmsPlaintextPasswdClear()
{
    char buf[10240] = {0};
    int infoLen = TmsChangePlaintextPasswdToAesPasswd(buf, 10240);
    if ( infoLen == -1) {
        perror("TmsPlaintextPasswdClear error\n");
        return -1;
    }
    mode_t oldMask = umask(0077);
    FILE *fp = fopen(gInfoPath, "w");
    umask(oldMask);
    if (!fp){
        perror("fopen11 error\n");
        return -1;
    }
    fwrite(buf, infoLen, 1, fp);
    //fsync(fp); 2013-8-2 11:50:28 by liujianhua fsync的参数是int不是FILE
    fclose(fp);

    return 0;
}

int tr069_port_infoLoad(char *buf, int len)
{
    if (!gInfoPath)
        return -1;

    return TmsChageAesPasswdToPlaintextPasswd(buf, len);
}

int tr069_port_infoSave(char *buf, int len)
{
    if (!gInfoPath)
        return -1;

    mode_t oldMask = umask(0077);
    FILE *fp = fopen(gInfoPath, "wb");
    umask(oldMask);
    if (!fp) {
        TR069Debug("fopen error\n");
        return -1;
    }
    int writeLen = fwrite(buf, 1, len, fp);
    if (writeLen != len) {
        TR069Debug("fopen error\n");
        fclose(fp);
        return -1;
    }
    //fsync(fp); 2013-8-2 11:50:28 by liujianhua fsync的参数是int不是FILE
    fclose(fp);
    TmsPlaintextPasswdClear();
    return 0;
}
#endif//ANDROID

void tr069_port_infoPath(char *path)
{
    if (!path)
        return;
    if (gInfoPath)
        free(gInfoPath);
    gInfoPath = strdup(path);
}

/*------------------------------------------------------------------------------
    获取TR069默认配置信息
 ------------------------------------------------------------------------------*/
int tr069_port_getDefault(char *buf, int len)
{
    int l;
    unsigned int size;

    l = 0;
    char stbid[33] = {0};

    size = 32;
    tr069_port_getValue("Device.X_00E0FC.STBID", stbid, size);
    //"URL=http://116.213.94.142:80/entry_digest/node1/tr069\n"
    //"URL=http://116.213.94.142:80/entry_digest/node1/tr069\n"
    //http://192.168.13.60:9090/ACS-server/ACS
#if defined(STBTYPE_QTEL)
    l += snprintf(buf + l, len - l,
        "URL=http://tms.qteliptv.net.qa:31800/comserver/node1/tr069\n"\
        "Username=testcpe\n"\
        "Password=ac5entry\n"\
        "ConnectionRequestUsername=admin\n"\
        "ConnectionRequestPassword=admin\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=60\n");//86400
#elif defined(SHANGHAI_HD)
    l += snprintf(buf + l, len - l,
        "URL=http://110.1.1.118:8080/acs_simple/SoapServlet\n"\
        "Username=STBAdmin\n"\
        "Password=STBAdmin\n"\
        "ConnectionRequestUsername=STBAdmin\n"\
        "ConnectionRequestPassword=STBAdmin\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=3600\n");//86400
#elif defined(SHANGHAI_SD)
    l += snprintf(buf + l, len - l,
        "URL=http://110.1.1.118:8080/acs_simple/SoapServlet\n"\
        "Username=STBAdmin\n"\
        "Password=STBAdmin\n"\
        "ConnectionRequestUsername=STBAdmin\n"\
        "ConnectionRequestPassword=STBAdmin\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=3600\n");//86400
#elif defined(Jiangsu)
    l += snprintf(buf + l, len - l,
        "URL=http://180.100.134.11:5050/web/tr069\n"\
        "Username=cpe\n"\
        "Password=cpe\n"\
        "ConnectionRequestUsername=cpe\n"\
        "ConnectionRequestPassword=cpe\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=3600\n");//86400

#elif defined(LIAONING_HD)||defined(LIAONING_SD)

    l += snprintf(buf + l, len - l,
        "URL=http://devacs.edatahome.com:9090/ACS-server/ACS\n"\
        "URLBackup=http://devacs.edatahome.com:9090/ACS-server/ACS\n"\
        "URLModifyFlag=15\n"\
        "Username=testcpe\n"\
        "Password=ac5entry\n"\
        "ConnectionRequestUsername=%s\n"\
        "ConnectionRequestPassword=STBAdmin\n"\
        "STUNUsername=%s\n"\
        "STUNPassword=STBAdmin\n"\
        "STUNMaximumKeepAlivePeriod=170\n"\
        "STUNMinimumKeepAlivePeriod=170\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=3600\n", stbid,stbid);

#elif defined(C15_HD) || defined(C15_SD)
    l += snprintf(buf + l, len - l,
        "URL=http://devacs.edatahome.com:9090/ACS-server/ACS\n"\
        "URLBackup=http://devacs.edatahome.com:9090/ACS-server/ACS\n"\
        "URLModifyFlag=15\n"\
        "Username=testcpe\n"\
        "Password=ac5entry\n"\
        "ConnectionRequestUsername=admin\n"\
        "ConnectionRequestPassword=admin\n"\
        "STUNUsername=%s\n"\
        "STUNPassword=STBAdmin\n"\
        "STUNMaximumKeepAlivePeriod=170\n"\
        "STUNMinimumKeepAlivePeriod=170\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=3600\n", stbid);//86400
#elif defined(GUANGDONG)
    l += snprintf(buf + l, len - l,
        "URL=\n"\
        "URLModifyFlag=15\n"\
        "Username=testcpe\n"\
        "Password=ac5entry\n"\
        "ConnectionRequestUsername=admin\n"\
        "ConnectionRequestPassword=admin\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=3600\n");
#elif defined(SICHUAN_HD)// || _HW_BASE_VER_ >= 58
    /*四川电信调试itms*/
    l += snprintf(buf + l, len - l,
        "URL=http://devacs.edatahome.com:9090/ACS-server/ACS\n"\
        "Username=STB\n"\
        "Password=STB\n"\
        "ConnectionRequestUsername=ITMS\n"\
        "ConnectionRequestPassword=ITMS\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=3600\n");//86400
#elif defined(NEIMENGGU_HD)
    l += snprintf(buf + l, len - l,
        "URL=http://1.25.202.10:8090/ACSServer\n"\
        "URLModifyFlag=15\n"\
        "Username=STB\n"\
        "Password=STB\n"\
        "ConnectionRequestUsername=ITMS\n"\
        "ConnectionRequestPassword=ITMS\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=3600\n");//86400
#elif defined(Cameroon_v5)
     l += snprintf(buf + l, len - l,
        "URL=\n"\
        "URLModifyFlag=15\n"\
        "Username=testcpe\n"\
        "Password=ac5entry\n"\
        "ConnectionRequestUsername=admin\n"\
        "ConnectionRequestPassword=admin\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=60\n");//86400
#elif defined(GUANGDONG)
    l += snprintf(buf + l, len - l,
        "URL=\n"\
        "URLModifyFlag=15\n"\
        "Username=testcpe\n"\
        "Password=ac5entry\n"\
        "ConnectionRequestUsername=admin\n"\
        "ConnectionRequestPassword=admin\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=3600\n");

#else
    l += snprintf(buf + l, len - l,
        "URL=http://110.1.1.81:8082/tr069\n"\
        "URLModifyFlag=15\n"\
        "Username=testcpe\n"\
        "Password=ac5entry\n"\
        "ConnectionRequestUsername=admin\n"\
        "ConnectionRequestPassword=admin\n"\
        "PeriodicInformEnable=1\n"\
        "PeriodicInformInterval=60\n");//86400
#endif

    return l;
}

int tr069_port_fault2string(int faultcode, char *faulstring, int maxlen)
{
    char string[256] = {0};
    switch(faultcode) {
    case UPGRADE_CONNECT_SERVER_FAIL:
        strcpy(string, "Connect to server failed!");
        break;
    case UPGRADE_NET_DISCONNECT:
        strcpy(string, "Disconnect from server!");
        break;
    case UPGRADE_ILLEGAL_VERSION:
        strcpy(string, "Illegal software Version, cannot upgrade!");
        break;
    case UPGRADE_WRITE_FLASH_FAIL:
        strcpy(string, "Upgrade failed, stb will restart ,please make sure network is OK!");
        break;
    case UPGRADE_SAME_VERSION_AS_SERVERS:
        strcpy(string, "Upgrade failed, cannot upgrade to same software version!");
        break;
    case UPGRADE_IS_RUNNING_ALREADY:
        strcpy(string, "Upgrade process is running!");
        break;
    default:
        strcpy(string, "Unknow fault");
        break;
    }
    if (strlen(string) < maxlen - 1) {
        strcpy(faulstring, string);
    } else {
        strncpy(faulstring, string, maxlen - 1);
        faulstring[maxlen - 1] = 0;
     }
    return 0;
}



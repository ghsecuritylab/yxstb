#include <map>
#include <fstream>
#include <sys/stat.h>

#include "Hippo_ContextHWC20.h"
#include "Hippo_Debug.h"
#include <Hippo_api.h>

#include <openssl/des.h>

#include "middle/mid_http.h"
#include <json/json_public.h>

#include "app/app_heartbit.h"
#include "cryptoFunc.h"
#include "charConvert.h"

#include "AppSetting.h"
#include "KeyTableParser.h"
#include "mid/mid_timer.h"

#include "NetworkFunctions.h"

#include "JseRoot.h"

namespace Hippo
{

HippoContextHWC20::HippoContextHWC20()
{
    m_pMediaPlayerMgr = &m_MediaPlayerMgr;
}

HippoContextHWC20::~HippoContextHWC20()
{
}

int HippoContextHWC20::JseRegister(const char *ioName, JseIoctlFunc &rfunc, JseIoctlFunc &wfunc, ioctl_context_type_e eChnl)
{
    if(eChnl != IoctlContextType_eHWBaseC10)
        m_ioctlCustomMap[ioName] = ioctlMapNode<JseIoctlFunc>(rfunc, wfunc);
    return 0;
}

int HippoContextHWC20::UnJseRegister(const char *ioName, ioctl_context_type_e eChnl)
{
    if(eChnl != IoctlContextType_eHWBaseC10)
        m_ioctlCustomMap.erase(ioName);
    return 0;
}

int HippoContextHWC20::ioctlRead(HString &aField, HString &aValue/*out*/)
{
    HString param;
    int ret = -1;
    char buf[4096] = { 0 };
    const char *start = NULL;
    const char *cmdend = NULL;
    const char *parastart = NULL;
    const char* str;
    char c;

    HIPPO_DEBUG("run here field=%s.\n", aField.c_str());
    //TODO: 准备运行参数
    HString newField;
    str = aField.c_str();
    start = str;
    GET_CHAR(c, str);
    while(c != '\0') {
        switch(c) {
        case ',':
            cmdend = str - 1;
            parastart = str;
            goto end;
        case '^':
            cmdend = str - 1;
            parastart = str;
            goto end;
        case ':':
            GET_CHAR(c, str);
            if(c == ':') {
                cmdend = str - 2;
                parastart = str;
            } else {
                cmdend = str - 2;
                parastart = str - 1;
            }
            goto end;
        default:
            break;
        }
        GET_CHAR(c, str);
    }
end:
    if(cmdend != NULL && parastart != NULL) {
        char newFieldstr[4096];
        char parastr[4096];

        strncpy(newFieldstr, start, cmdend - start);
        newFieldstr[cmdend - start] = '\0';

        strncpy(parastr, parastart, 4096);
        newField = newFieldstr;
        param = parastr;
    } else {
        newField = aField;
    }

    ret = JseRootRead(newField.c_str(), param.c_str(), buf, 4096);
    if(!ret){
       aValue = buf;
       return ret;
    }

    //TODO: 准备运行参数
    do {
        //搜索客户注册列表
        //JseIoctlFunc pCustomFunc = 0;
        ioctlCustomMap::const_iterator itc;
        itc = m_ioctlCustomMap.find(newField.c_str());

        if(itc != m_ioctlCustomMap.end()) {
            const ioctlMapNode<JseIoctlFunc>& ioctlNode = (itc->second);
            if(ioctlNode.m_ioctlRead != NULL) {
                ret = (ioctlNode.m_ioctlRead)(newField.c_str(), param.c_str(), buf, 4096);
                break;
            }
        }
        ret = HippoContextHWBase::ioctlRead(aField, aValue);
    } while(0);

    aValue = buf;
    HIPPO_DEBUG("aValue=%s.\n", aValue.c_str());
    return ret;
}

int HippoContextHWC20::ioctlWrite(HString &aField, HString &aValue/*in*/)
{
    HString param;
    int ret = -1;

    HIPPO_DEBUG("run here field=%s,value=%s.\n", aField.c_str(), aValue.c_str());

    ret = JseRootWrite(aField.c_str(), NULL, (char *)aValue.c_str(), 0);
    if(!ret)
        return ret;

    do {
        //JseIoctlFunc pCustomFunc = 0;
        ioctlCustomMap::const_iterator itc;
        itc = m_ioctlCustomMap.find(aField.c_str());

        if(itc != m_ioctlCustomMap.end()) {
            const ioctlMapNode<JseIoctlFunc>& ioctlNode = (itc->second);
            if(ioctlNode.m_ioctlWrite != NULL) {
                ret = (ioctlNode.m_ioctlWrite)(aField.c_str(), param.c_str(), (char*)aValue.c_str(), HippoContext::s_ioctlWriteFlag);
                break;
            }
        }
        ret = HippoContextHWBase::ioctlWrite(aField, aValue);
    } while(0);
    return ret;
}

int HippoContextHWC20::AuthenticationCTCGetAuthInfo(HString& aToken, HString& aResult)
{
    HString desKey ;
    HString plainText;
    // 对密码MD5去hashvalue,
    char check[40] = {0};
    char passwd[USER_LEN] = { 0 };
    char* p = passwd;
    appSettingGetString("ntvAESpasswd", passwd, USER_LEN, 0);

    md5Encypt(&p, 1, check, sizeof(check), 1);
    data2Hex(check, 4, check, sizeof(check));
    desKey = check;
    if(desKey.length() > 24)
        desKey.erase(24, desKey.length());
    else {
        for(int i = desKey.length(); i < 24 ; i ++) {
            desKey += "0";
        }
    }

    /* 根据规格, 拼接加密字串; */
    srand((unsigned int)time(NULL)); // get random
    char bb[64] = {0};
    snprintf(bb, 64, "%u", rand() % 100000000);
    plainText += bb;
    plainText += "$";

    // encry token.
    plainText += aToken;
    plainText += "$";

    //ntvUserId,
    char userId[64] = {0};
    appSettingGetString("ntvuser", userId, 64, 0);
    plainText += userId;
    plainText += "$";

    //terminal ID
    char macAdd[64] = {0};
    //TODO 这个地方是个标识, 串号生成的mac地址, 不考虑无线
    network_tokenmac_get(macAdd, 64, ':');
    plainText += macAdd;
    plainText += "$";

    //IP
    char ifname[URL_LEN] = { 0 };
    char ifaddr[URL_LEN] = { 0 };
    network_default_ifname(ifname, URL_LEN);
    plainText += network_address_get(ifname, ifaddr, URL_LEN);
    plainText += "$";

    //MAC
    plainText += macAdd;
    plainText += "$";

    //Reserved string
    plainText += "Reserved";
    plainText += "$";

    //固定为CTC
    plainText += "CTC";

    char buf[1025] = { 0 };
    char Password[28] = {0};
    int i, j, text_len, text_remainder;

    DES_key_schedule ks1, ks2, ks3;
    DES_cblock key1, key2, key3;
    DES_cblock plain_text[40], output[40];

    strncpy(Password, desKey.c_str(), 28);
    for(i = 0; i < 24; i++) {
        if(i < 8)
            key1[i] = Password[i];
        else if(i < 16)
            key2[i - 8] = Password[i];
        else
            key3[i - 16] = Password[i];
    }

    //PKCS5Padding
    text_len = (int) strlen(plainText.c_str()) / 8 + 1 ;
    text_remainder = strlen(plainText.c_str()) % 8;

    for(i = 0; i < text_len; i++) {
        for(j = 0; j < 8; j++) {
            plain_text[i][j] = 0;
        }
    }

    for(i = 0; i < text_len; i++) {
        for(j = 0; j < 8; j++) {
            if(i == text_len - 1 && j >= text_remainder) {
                switch(text_remainder) {
                case 0:
                    plain_text[i][j] = 0x08;
                    break;

                case 1:
                    plain_text[i][j] = 0x07;
                    break;

                case 2:
                    plain_text[i][j] = 0x06;
                    break;

                case 3:
                    plain_text[i][j] = 0x05;
                    break;

                case 4:
                    plain_text[i][j] = 0x04 ;
                    break;

                case 5:
                    plain_text[i][j] = 0x03;
                    break;

                case 6:
                    plain_text[i][j] = 0x02;
                    break;

                case 7:
                    plain_text[i][j] = 0x01;
                    break;
                }
            } else {
                plain_text[i][j] = plainText.c_str()[ i * 8 + j ];
            }
        }
    }

    //set Key
    DES_set_key_unchecked(&key1, &ks1);
    DES_set_key_unchecked(&key2, &ks2);
    DES_set_key_unchecked(&key3, &ks3);

    //encrypt
    for(i = 0; i < text_len; i++) {
        DES_ecb3_encrypt(&plain_text[i], &output[i], &ks1, &ks2, &ks3, DES_ENCRYPT);
    }

    for(i = 0; i < text_len; i++) {
        for(j = 0; j < 8; j++) {
            snprintf(buf + (i * 16) + (2 * j), 3, "%02X", output[i][j]);
        }
    }
    aResult = buf;
    return true;
}

}

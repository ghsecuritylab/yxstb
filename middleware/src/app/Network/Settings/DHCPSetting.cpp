#include "DHCPSetting.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <openssl/des.h>

DHCPSetting::DHCPSetting()
    : mUsername("")
    , mPassword("")
    , mVendorClass("")
    , mClientID("") 
    , mRetryTimes(0)
    , mRetryInterval(0)
    , mReTransSeq("")
    , mLeaseTimeSeq("")
    , mEnterpriseNumber(0)
    , mSuboptCode(-1)
    , mVerify("")
{
}

DHCPSetting::~DHCPSetting()
{

}

DHCPSetting&
DHCPSetting::operator = (const DHCPSetting& rhs)
{
    mUsername = rhs.getUsername();
    mPassword = rhs.getPassword();
    mVendorClass = rhs.getVendorClass();
    mClientID = rhs.getClientID();
    mRetryTimes = rhs.getRetryTimes();
    mRetryInterval = rhs.getRetryInterval();
    mReTransSeq = rhs.getReTransSeq();
    mLeaseTimeSeq = rhs.getLeaseTimeSeq();
    mEnterpriseNumber = rhs.getEnterpriseNumber();
    mSuboptCode = rhs.getSuboptCode(); 
    mVerify = rhs.getVerify();
    return *this;
}

void
DHCPSetting::setUsername(const char* username)
{
    if (username && strlen(username) > 0)
        mUsername = username;
}

const char*
DHCPSetting::getUsername() const
{
    return mUsername.c_str();
}

void
DHCPSetting::setPassword(const char* password)
{
    if (password && strlen(password) > 0)
        mPassword = password;
}

const char*
DHCPSetting::getPassword() const
{
    return mPassword.c_str();
}

void
DHCPSetting::setClientID(const char* clientid)
{
    if (clientid && strlen(clientid) > 0)
        mClientID = clientid;
}

const char*
DHCPSetting::getClientID() const
{
    return mClientID.c_str();
}

void
DHCPSetting::setVendorClass(const char* vendor)
{
    if (vendor && strlen(vendor) > 0)
        mVendorClass = vendor;
}

const char*
DHCPSetting::getVendorClass() const
{
    return mVendorClass.c_str();
}

void 
DHCPSetting::setRetryTimes(int times)
{
    mRetryTimes = times;
}

const int 
DHCPSetting::getRetryTimes() const
{
    return mRetryTimes;
}

void 
DHCPSetting::setRetryInterval(int interval)
{
    mRetryInterval = interval;
}

const int 
DHCPSetting::getRetryInterval() const
{
    return mRetryInterval;
}

void 
DHCPSetting::setReTransSeq(const char* seq)
{
    if (seq && strlen(seq) > 0)
        mReTransSeq = seq;
}

const char* 
DHCPSetting::getReTransSeq() const
{
    return mReTransSeq.c_str();
}

void 
DHCPSetting::setLeaseTimeSeq(const char* seq)
{
    if (seq && strlen(seq) > 0)
        mLeaseTimeSeq = seq;
}

const char* 
DHCPSetting::getLeaseTimeSeq() const
{
    return mLeaseTimeSeq.c_str();
}

void 
DHCPSetting::setEnterpriseNumber(int number)
{
    mEnterpriseNumber = number;
}

const int DHCPSetting::getEnterpriseNumber() const
{
    return mEnterpriseNumber;
}

void 
DHCPSetting::setSuboptCode(int code)
{
    mSuboptCode = code;
}

const int 
DHCPSetting::getSuboptCode() const
{
    return mSuboptCode;
}

void 
DHCPSetting::setVerify(const char* verify)
{
    if (verify && strlen(verify) > 0)
        mVerify = verify;
}

const char*
DHCPSetting::getVerify() const
{
    return mVerify.c_str();
}

char* EncryOption60(char* value, const char* userid /*max 128bytes*/, const char* password, int enterprise)
{
    if (!value || !userid || !password)
        return 0;
    const int kTxtLength = strlen(userid) / 8 + 1;
    const int kRemainder = strlen(userid) % 8;
    if (kTxtLength > 16)
        return 0;

    unsigned char R[9] = { 0 };
    unsigned char TS[9] = { 0 };
    unsigned char C[kTxtLength * 8];
    unsigned char Key[16] = { 0 };
    unsigned char O[1] = { 1 };
    unsigned char Message[512] = { 0 };

    int i = 0, j = 0, len = 0;
    DES_cblock key1, key2, key3;
    DES_cblock in[kTxtLength], out[kTxtLength];
    DES_key_schedule ks1, ks2, ks3;
    MD5_CTX md5;
    unsigned char* p = &Message[0];
    unsigned char head[4] = { 0 };

    srand(time(0));
    //1. generate R (8Bytes)
    snprintf((char*)R, 9, "%08u", (unsigned int)(rand() % 100000000));
    memcpy(key1, R, 8);

    //2. generate TS (8Bytes)
    snprintf((char*)TS, 9, "%08u", (unsigned int)(time(0) % 100000000));
    memcpy(key2, TS, 8);

    //3. generate C = EnCry(R+TS+64Bit, Login), here Login = userid
    memset(key3, 0, 8);
    DES_set_key_unchecked(&key1, &ks1);
    DES_set_key_unchecked(&key2, &ks2);
    DES_set_key_unchecked(&key3, &ks3);
    for(i = 0; i < kTxtLength; ++i) {
        for(j = 0; j < 8; ++j)
            in[i][j] = 0;
    }
    for(i = 0; i < kTxtLength; ++i) {
        for(j = 0; j < 8; ++j) {
            if(i == kTxtLength - 1 && j >= kRemainder)
                in[i][j] = 0x08 - kRemainder;
            else
                in[i][j] = userid[i * 8 + j];
        }
    }
    for(i = 0; i < kTxtLength; ++i)
        DES_ecb3_encrypt(&in[i], &out[i], &ks1, &ks2, &ks3, DES_ENCRYPT);
    memcpy(C, out, kTxtLength * 8);

    //4. generate Key = Hash(R+Password+TS) (16Bytes), here Hash use MD5 instead.
    MD5_Init(&md5);
    MD5_Update(&md5, R, strlen((char*)R));
    MD5_Update(&md5, password, strlen(password));
    MD5_Update(&md5, TS, strlen((char*)TS));
    MD5_Final(Key, &md5);

    //5. send Message = O+R+TS+Key+C
    p = (unsigned char*)memcpy(p, &O, 1) + 1; //Android not support mempcpy, so mod.
    p = (unsigned char*)memcpy(p, &R, 8) + 8;
    p = (unsigned char*)memcpy(p, &TS, 8) + 8;
    p = (unsigned char*)memcpy(p, &Key, 16) + 16;
    p = (unsigned char*)memcpy(p, &C, kTxtLength * 8) + kTxtLength * 8;

    //6. encapsulate option60 infomation
    head[0] = (enterprise & 0x0000FF00) >> 8;
    head[1] = enterprise & 0x000000FF;
    head[2] = 31; //fix value 31
    head[3] = p - Message;
    len = 0;
    for (i = 0; i < 4; ++i) 
        len += sprintf(value + len, "%02x:", head[i]);
    for (i = 0; i < head[3]; ++i)
        len += sprintf(value + len, "%02x:", Message[i]);
    value[len - 1] = 0;
    return value;
}

void InitDhcpConfigureFile(const char* path, DHCPSetting* conf)
{
    const int kBuffLength = 1024;
    char buff[kBuffLength] = { 0 }; 
    int len = 0;
    FILE* fp = fopen(path, "w");
    if (!fp)
        return;

    //include common configure file;
    len = snprintf(buff, kBuffLength, "include \"%s\";\n", DEFAULT_DHCP_CONFIG_PATH);
    fwrite(buff, 1, len, fp);

    if (conf->getRetryTimes() > 0) {
        len = snprintf(buff, kBuffLength, "hw-retry-times %d;\n", conf->getRetryTimes());
        fwrite(buff, 1, len, fp);
    }
    if (conf->getRetryInterval() > 0) {
        len = snprintf(buff, kBuffLength, "hw-retry-interval %d;\n", conf->getRetryInterval());
        fwrite(buff, 1, len, fp);
    }
    if (strlen(conf->getReTransSeq()) > 0) {
        len = snprintf(buff, kBuffLength, "hw-retrans-seq \"%s\";\n", conf->getReTransSeq());
        fwrite(buff, 1, len, fp);
    }

    if (strlen(conf->getLeaseTimeSeq()) > 0) {
        len = snprintf(buff, kBuffLength, "hw-leasetime-seq \"%s\";\n", conf->getLeaseTimeSeq());
        fwrite(buff, 1, len, fp);
    }

    //send option60
    if (strlen(conf->getVendorClass()) > 0) {
        /*
         *    0                 1                 2                 3
         *    0 1 2 3 4 5 6 7 8 0 1 2 3 4 5 6 7 8 0 1 2 3 4 5 6 7 8 0 1 2 3 4 5 6 7 8
         *   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
         *   |    Code(60)     |     Length      |            Enterprise Code         |
         *   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
         *   |  Field Type(31) |  Field Length   |  Filed Value(Message=O+R+TS+Key+C) |
         *   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
         */
        len = snprintf(buff, kBuffLength, "send vendor-class-identifier %s;\n", conf->getVendorClass()); 
        fwrite(buff, 1, len, fp);
    } else {
        //TODO: more complicated, must generate by dhclient progress because of random xid is needed.
        if (strlen(conf->getUsername()) > 0) {
            len = snprintf(buff, kBuffLength, "hw-username \"%s\";\n", conf->getUsername());
            fwrite(buff, 1, len, fp);
        }
        if (strlen(conf->getPassword()) > 0) {
            len = snprintf(buff, kBuffLength, "hw-password \"%s\";\n", conf->getPassword());
            fwrite(buff, 1, len, fp);
        }
    }

    //send optin61
    if (strlen(conf->getClientID()) > 0) { //TODO
        len = snprintf(buff, kBuffLength, "send dhcp-client-identifier \"%s\";\n", conf->getClientID());
        fwrite(buff, 1, len, fp);
    }

    //require option125
    if (strlen(conf->getVerify()) > 0) {
        /*
         *       0                 1                 
         *       0 1 2 3 4 5 6 7 8 0 1 2 3 4 5 6 7 8 
         *      ++++++++++++++++++++++++++++++++++++
         *      |    Code(125)    |     Length     |
         *      ++++++++++++++++++++++++++++++++++++
         *      |            Enterprise Code       |
         *      ++++++++++++++++++++++++++++++++++++
         *      |   Data-Len1     |                |
         *      |------------------                |
         *  +---|-     vendor-class-data1          |
         *  |   ++++++++++++++++++++++++++++++++++++  vendor-class-data can contain suboptions (ShangHai Telcom)
         *  |
         *  |   subopt125 = -1 : no suboption,  vendor-class-data is verify string.
         *  |   subopt125 > 0 : contain suboptions. 
         *  |
         *  |    0 1 2 3 4 5 6 7 8 0 1 2 3 4 5 6 7 8 
         *  |   ++++++++++++++++++++++++++++++++++++
         *  |   | subopt-code(125) | subopt-length |
         *  +-->++++++++++++++++++++++++++++++++++++
         *      |         sub-option-data          |
         *      ++++++++++++++++++++++++++++++++++++
         */
        len = snprintf(buff, kBuffLength, "also request vivso;\n");
        fwrite(buff, 1, len, fp);
        len = snprintf(buff, kBuffLength, "option hw-opt125 code 125 = string;\n");
        fwrite(buff, 1, len, fp);
        len = snprintf(buff, kBuffLength, "hw-subopt125 %d;\n", conf->getSuboptCode());
        fwrite(buff, 1, len, fp);
        len = snprintf(buff, kBuffLength, "hw-verify \"%s\";\n", conf->getVerify());
        fwrite(buff, 1, len, fp);
    }
    if (conf->getEnterpriseNumber() >= 0) {
        len = snprintf(buff, kBuffLength, "hw-enterprise %d;\n", conf->getEnterpriseNumber());
        fwrite(buff, 1, len, fp);
    }
    fclose(fp);
}



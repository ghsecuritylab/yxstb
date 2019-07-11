#ifndef __DHCPSetting__H_
#define __DHCPSetting__H_

#ifdef __cplusplus
#include <string>

#define DEFAULT_DHCP_CONFIG_PATH "/home/hybroad/share/dhclient.conf" //TODO
class DHCPSetting {
public:
    DHCPSetting();
    ~DHCPSetting();

    DHCPSetting& operator = (const DHCPSetting& rhs);

    void setUsername(const char* username);
    const char* getUsername() const;

    void setPassword(const char* password);
    const char* getPassword() const;

    void setVendorClass(const char* vc);
    const char* getVendorClass() const;

    void setClientID(const char* clientid);
    const char* getClientID() const;

    void setRetryTimes(int times);
    const int getRetryTimes() const;

    void setRetryInterval(int interval);
    const int getRetryInterval() const;

    void setReTransSeq(const char* seq);
    const char* getReTransSeq() const;

    void setLeaseTimeSeq(const char* seq);
    const char* getLeaseTimeSeq() const;

    void setEnterpriseNumber(int number);
    const int getEnterpriseNumber() const;

    void setSuboptCode(int code);
    const int getSuboptCode() const;

    void setVerify(const char* verify);
    const char* getVerify() const;

private:
    std::string mUsername;
    std::string mPassword;
    std::string mVendorClass;
    std::string mClientID;
    int mRetryTimes;
    int mRetryInterval;
    std::string mReTransSeq;
    std::string mLeaseTimeSeq;
    int mEnterpriseNumber;
    int mSuboptCode; 
    std::string mVerify;
};

char* EncryOption60(char* value, const char* userid /*max 128bytes*/, const char* password, int enterprise);
void InitDhcpConfigureFile(const char* path, DHCPSetting* conf);
#endif

#endif

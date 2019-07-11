#ifndef __IPv4Setting__H_
#define __IPv4Setting__H_

#ifdef __cplusplus
#include <string>

class IPv4Setting {
public:
    IPv4Setting();
    ~IPv4Setting();

    IPv4Setting& operator = (const IPv4Setting& rhs);

    void clear();

    void setAddress(const char* addr);
    const char* getAddress() const;

    void setNetmask(const char* mask);
    const char* getNetmask() const;

    void setGateway(const char* gate);
    const char* getGateway() const;

    void setDns0(const char* dns0);
    const char* getDns0() const;

    void setDns1(const char* dns1);
    const char* getDns1() const;

private:
    std::string mAddress;
    std::string mNetmask;
    std::string mGateway;
    std::string mDns0;
    std::string mDns1;
};

#endif

#endif

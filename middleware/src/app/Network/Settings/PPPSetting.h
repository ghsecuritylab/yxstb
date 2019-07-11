#ifndef __PPPSetting__H_
#define __PPPSetting__H_

#ifdef __cplusplus
#include <string>

class PPPSetting {
public:
    PPPSetting();
    ~PPPSetting();

    PPPSetting& operator = (const PPPSetting& rhs);

    void setUsername(const char* username);
    const char* getUsername() const;

    void setPassword(const char* psssword);
    const char* getPassword() const;

    void setRetryTimes(int times);
    const int getRetryTimes() const;
private:
    std::string mUsername;
    std::string mPassword;
    int mRetryTimes;
};

#endif

#endif

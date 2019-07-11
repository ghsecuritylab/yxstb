#ifndef __NetworkErrorCode__H_
#define __NetworkErrorCode__H_

#include "net_errorcode_def.h"

#define SHOW_MINI_ALERT  0
#define OPEN_ERROR_PAGE  1
#define SEND_ERROR_EVENT 2

#define to_positive_sign(x) ((x) > 0 ? x : (-1) * (x))

#ifdef __cplusplus
#include <string>

class NetworkManager;

class NetworkErrorCode {
public:
    NetworkErrorCode(int codeid, int showmode, const char* message, int promptid = -1);
    NetworkErrorCode();
    ~NetworkErrorCode();

    void setCodeId(int codeid);
    int getCodeId() const;

    void setShowMode(int showmode);
    int getShowMode() const;

    void setMessage(const char* message);
    const char* getMessage() const;

    void setPromptId(int promptid);
    int getPromptId() const;

    static bool isAuthFail(int hycode);
private:
    int mCodeId;
    int mShowMode;
    int mPromptId;
    std::string mMessage;
};

void NetworkErrorCodeRegister(NetworkManager&);
#endif

#endif

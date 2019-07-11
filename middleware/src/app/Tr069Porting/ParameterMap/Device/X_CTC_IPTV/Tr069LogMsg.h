#ifndef Tr069LogMsg_h
#define Tr069LogMsg_h

#include "Tr069GroupCall.h"

#ifdef __cplusplus

class Tr069LogMsg : public Tr069GroupCall {
public:
    Tr069LogMsg();
    ~Tr069LogMsg();
};


extern "C"
{
void tr069LogMsgStatisticInfo(int flag, char *info);
int tr069LogMsgInit(void);
int tr069LogMsgPost(char *tm, int type, char *info);
unsigned int tr069LogMsgGetEnable(void);

void tr069LogMsgPostHTTPInfo(char *info_buf, int info_len);
void tr069LogMsgPostRTSPInfo(int multflg, char *info_buf, int info_len);
}

#endif // __cplusplus

enum {
    LOG_MSG_RTSPINFO = 0,
    LOG_MSG_HTTPINFO,
    LOG_MSG_IGMPINFO,
    LOG_MSG_PKGTOTALONESEC,
    LOG_MSG_BYTETOTALONESEC,
    LOG_MSG_PKGLOSTRATE,
    LOG_MSG_AVARAGERATE,
    LOG_MSG_BUFFER,
    LOG_MSG_ERROR,
    LOG_MSG_VENDOREXT,
    LOG_MSG_MAX
};

#endif // Tr069LogMsg_h

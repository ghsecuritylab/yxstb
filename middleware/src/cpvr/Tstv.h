#ifndef __TSTV_H__
#define __TSTV_H__

#ifdef __cplusplus

#include "UserManager.h"

namespace Hippo {

typedef enum {
    RECORD_UNINITIALIZED = -1,  /*?��3?��??��*/
    RECORD_START_SUCCESS,   /*3��1|??��?��?��?������?*/
    RECORD_NO_CHANNEL,
    RECORD_NO_DISK,
    RECORD_DISK_WRITE_ERR,
    RECORD_DISK_REMOVED,
    RECORD_DISK_FIETYPE_ERR,
    RECORD_NO_FREE_SPACE,   /*��??��????2?��?��??T����???����?��?������?*/
    RECORD_COLLIDE,         /*�̡�?�㨮DcPVR?����?��?������?��??����??T��?1?��??������?���?2??��???����?��?������?*/
} RECORDSTATUS;

class Tstv {
public:
    int mChanKey;
    int mStartFlag;
    std::string mChanDomain;
    RECORDSTATUS mRecordStatus;
    std::string mStartTime;

    Tstv();
    ~Tstv();
    int startRecord();
    int stopRecord();
    int releaseRecordResource();
    int buildEvent(RECORDSTATUS recordStatus);

protected:
    UserManager tstvUserMng;

private:
} ;

}

extern "C" {
#endif // __cplusplus

int TstvStartFlagGet();
int TstvDealRecordMsg(int msg);
int TstvJseRegister(int type);
int TstvUnJseRegister(int type);

#ifdef __cplusplus
};
#endif // __cplusplus

#endif // __TSTV_H__


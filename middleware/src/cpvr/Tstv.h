#ifndef __TSTV_H__
#define __TSTV_H__

#ifdef __cplusplus

#include "UserManager.h"

namespace Hippo {

typedef enum {
    RECORD_UNINITIALIZED = -1,  /*?¡ä3?¨º??¡¥*/
    RECORD_START_SUCCESS,   /*3¨¦1|??¨¨?¡À?¦Ì?¨º¡À¨°?*/
    RECORD_NO_CHANNEL,
    RECORD_NO_DISK,
    RECORD_DISK_WRITE_ERR,
    RECORD_DISK_REMOVED,
    RECORD_DISK_FIETYPE_ERR,
    RECORD_NO_FREE_SPACE,   /*¡ä??¨¬????2?¡Á?¡ê??T¡¤¡§???¡¥¡À?¦Ì?¨º¡À¨°?*/
    RECORD_COLLIDE,         /*¦Ì¡À?¡ã¨®DcPVR?¨°¡À?¦Ì?¨º¡À¨°?¡ä??¨²¡ê??T¡Á?1?¡ä??¨ª¡Á¨º?¡ä¡ê?2??¨¹???¡¥¡À?¦Ì?¨º¡À¨°?*/
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


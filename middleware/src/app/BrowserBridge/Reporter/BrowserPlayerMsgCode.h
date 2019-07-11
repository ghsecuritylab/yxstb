#ifndef _BrowserPlayerMsgCode_H_
#define _BrowserPlayerMsgCode_H_

#include <map>

#ifdef __cplusplus

namespace Hippo {
    
enum PrintErrorType {
    Print_ErrCmd,
    Print_ErrUrl,
    Print_All,
    Print_None    
};

enum EventType {
    Event_MediaError = 0,
    Event_MediaBegin,
    Event_MediaEnd,
    Event_MediaADSBegin,
    Event_MediaADSEnd,
    Event_MediaBuffer,
    Event_PlayModeChange,
    Event_PLTVModeChange,
    Event_GOTOChannel,
    Event_STBError,
    Event_STBRestore
};
        
class PlayerCode {
public:    
    int mEventType;
    int mStreamMsg;
    int mStreamCode;
    int mEPGCodeC10;
    int mEPGCodeC20;
    const char *mMessageC10;
    const char *mMessageC20;
    int mPrintType;
    const char *mPrintMessage;
};

class BrowserPlayerMsgCode {
public:
    BrowserPlayerMsgCode();
    ~BrowserPlayerMsgCode();
    
    int addMsgCode(PlayerCode *playCode);
    PlayerCode* getMsgCode(int message, int code);
    
private:
    std::map<int, PlayerCode*> mPlayerMsgCode;  
    
};

BrowserPlayerMsgCode &browserPlayerMsgCode();
    
}

#endif // __cplusplus

#endif


#ifndef _PPVListInfo_H_
#define _PPVListInfo_H_

#ifdef __cplusplus

#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"
#include "PPVProgram.h"
#include "DataSink.h"
    
#include <map>

namespace Hippo {

class HttpDataSource;

class PPVListInfo : public DataSink, public MessageHandler {
public:
    PPVListInfo();
    ~PPVListInfo();
    
    enum {
        PPV_Delay,
        PPV_Reminder,
        PPV_DataArrive,
        PPV_End,
        PPV_Error
    };
     
    virtual bool onDataArrive();
    virtual bool onError();
    virtual bool onEnd();
        
    std::string& PPVVersion() { return m_PPVVersion; }
    void refreshPPVList();    
    PPVAuthorizedProgram* checkPPVChannel(int);
       
protected:
    virtual void handleMessage(Message * msg);
    void parsePPVProgram();
    void parseAuthorizedProgram(PPVProgram*, std::string);
    bool addPPVProgram(PPVProgram*);
    void PPVReminder();
    void receiveData();
    void receiveError();
    void receiveEnd();
             
private:
    bool m_isParsing;
    char* m_memory;
    std::string m_PPVVersion;
    std::string m_PPVProgram;
    HttpDataSource* m_PPVSource;    
    std::map<int, PPVProgram*> m_ppvList;
};

PPVListInfo* ppvListInfo();

}

#endif

#endif
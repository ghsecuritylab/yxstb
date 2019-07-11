#ifndef _PPVProgram_H_
#define _PPVProgram_H_

#ifdef __cplusplus

#include <map>
#include <string>

namespace Hippo {

class PPVAuthorizedProgram {
public:
    int m_status;
    unsigned int m_startTimeNum;
    unsigned int m_stopTimeNum;
    std::string m_startTime;
    std::string m_stopTime;
    std::string m_programID;        
    std::string m_programCode;
    bool m_upgradeFlag;
      
};

class PPVProgram {
public:
    PPVProgram();
    ~PPVProgram();
    
    enum {
        PPV_Start,
        PPV_Range,
        PPV_End
    };
    
    void setChanID(int chanID) { m_chanID = chanID; }
    int chanID() { return m_chanID; }
    
    bool addAuthorizedProgram(PPVAuthorizedProgram*);
    bool removeAuthorizedProgram(std::string);
    PPVAuthorizedProgram* findPPVAuthorizedProgram(std::string);    
    bool hasAuthorizedProgram() { return !m_Authorized.empty(); }
    PPVAuthorizedProgram* getPPVProgram(int, int);

public:
    int m_chanID;
    bool m_upgradeFlag;
    
private:
    std::map<std::string, PPVAuthorizedProgram*> m_Authorized;
                
};

}
#endif

#endif
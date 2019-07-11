#ifndef _UpgradeIPChecker_H_
#define _UpgradeIPChecker_H_


#ifdef __cplusplus

#include "UpgradeChecker.h"
#include <string>

namespace Hippo {
    
class UpgradeSource;
class UpgradeManager;

class UpgradeIPChecker : public UpgradeChecker {
public:
    UpgradeIPChecker(UpgradeManager*);
    ~UpgradeIPChecker();
    
    enum CheckStep {
        Request_Upgrade_Address = 0x01,
        Get_Config_File,
        Parse_Config_File,
        Check_End        
    };
    
    virtual bool start();
    virtual bool stop();
    virtual bool reset();

    void setRealUpgradeIP(char* ip) { m_realUpgradeIP = ip; }
    void setRealUpgradePort(int port) { m_realUpgradePort = port; }
    void upgradeAddressGet(char*, int);
    void setUpgradeConfigInfo(char* fileInfo) { m_fileInfo = fileInfo; }
        
protected:
    virtual void handleMessage(Message* msg);
    
    void requestRealUpgradeAddr();
    void downLoadConfigFile();
    void parseConfigFile();
    void onCheckEnd();
    
private:
    std::string m_fileInfo;
    std::string m_realUpgradeIP;    
    int m_realUpgradePort;
    bool m_isUseBakAddr;
        
};
    
} //namespace Hippo


#endif //__cplusplus


#endif 
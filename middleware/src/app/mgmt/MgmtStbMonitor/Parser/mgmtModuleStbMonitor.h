#ifndef __MGMTSTBMONITOR_H__
#define __MGMTSTBMONITOR_H__

#include "mgmtModuleTr069.h"

typedef std::map<std::string, ParamMapFunc> StbMonitorParamMap;

class MgmtModuleStbMonitor 
{
public:
    MgmtModuleStbMonitor();
    ~MgmtModuleStbMonitor();
    int StbMonitorParamReg(const char *param, void *rfunc, void *wfunc, int type);
    StbMonitorParamMap m_StbMonitorParamMap;
private:

};

MgmtModuleStbMonitor* getStbMonitorMgmtInstance();
int mgmtReadConfig(const char *szParm, char *pBuf, int iLen);
int mgmtWriteConfig(const char *szParm, char *pBuf, int iLen);
int StbMonitorParamRegister(const char *param, void *rfunc, void *wfunc, int type);
int mgmtModuleStbMonitorParamRegist();
int mgmtModuleStbMonitorInit();

#endif

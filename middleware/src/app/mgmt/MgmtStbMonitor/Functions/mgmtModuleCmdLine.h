#ifndef _MgmtModuleCmdLine_H_
#define _MgmtModuleCmdLine_H_

#ifdef __cplusplus
#include <map>
#include <string>
#endif

typedef void (* mgmtCmdLineCallBack)(int argc, char *argv[]);
#ifdef __cplusplus
namespace Hippo {

class MgmtModuleCmdLine 
{
public:
    MgmtModuleCmdLine();
    ~MgmtModuleCmdLine();
    int mgmtModuleCmdLineRegister(const char *cmd, void *exefunc);

//	MgmtModuleCmdLine*  m_next; 
    std::map<std::string, mgmtCmdLineCallBack> m_itemMap;

};
MgmtModuleCmdLine* mgmtModuleCmdLineAgent();

}    //Hippo
#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif
void mgmtCmdLineManagerInit();
int mgmtCliCommandRegist();
int mgmtCmdParamRegist(const char *cmd, void *exefunc);
void shellMapExecuteFunc();
void mgmtCliCmdPrint(char *);


#ifdef __cplusplus
}
#endif

#endif

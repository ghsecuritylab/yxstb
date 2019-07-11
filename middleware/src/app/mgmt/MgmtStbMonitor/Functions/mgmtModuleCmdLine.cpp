#include "mgmtModuleCmdLine.h"
#include <iostream>
#include "hmw_mgmtlib.h"
#include "libcli.h"
#include "mgmtCliSelfDefineCmd.h"
static struct cli_def *g_cliMsg = NULL;

namespace Hippo {

static MgmtModuleCmdLine *gMgmtCmdLineManager = NULL;

MgmtModuleCmdLine::MgmtModuleCmdLine()
{
}

MgmtModuleCmdLine::~MgmtModuleCmdLine()
{
}

MgmtModuleCmdLine* mgmtModuleCmdLineAgent()
{
    return  gMgmtCmdLineManager;
}

int MgmtModuleCmdLine::mgmtModuleCmdLineRegister(const char *cmd, void *wfunc)
{
    m_itemMap[cmd] = (mgmtCmdLineCallBack)wfunc;
    return 0;
}

}

int mgmtCmdParamRegist(const char *cmd, void *exefunc)
{
    Hippo::mgmtModuleCmdLineAgent()->mgmtModuleCmdLineRegister(cmd, exefunc);
    return 0;
}

extern "C" {
void mgmtCmdLineManagerInit()
{
    Hippo::gMgmtCmdLineManager = new Hippo::MgmtModuleCmdLine();
}

int mgmtCmdLine(struct cli_def *pstCliMng,  char *command, char *argv[], int argc)
{
    std::map<std::string, mgmtCmdLineCallBack>::iterator it;

    it = Hippo::mgmtModuleCmdLineAgent()->m_itemMap.find(command);
    if (it == Hippo::mgmtModuleCmdLineAgent()->m_itemMap.end()){
        // INCLUDE_HMWMGMT ╡есп
        // cli_print(pstCliMng, "sorry, %s: exefunc is not found!\n", command);
        return -1;
    }
    mgmtCmdLineCallBack executeFunc = it->second;
    g_cliMsg = pstCliMng;
    executeFunc(argc, argv);
    return 0;
}

void shellMapExecuteFunc()
{
    shellSelfDefinedInit();
}

void mgmtCliCmdPrint(char *buffer)
{
#ifdef INCLUDE_HMWMGMT
    cli_print(g_cliMsg, "%s", buffer);
#endif
}

int mgmtCliCommandRegist()
{
#ifdef INCLUDE_HMWMGMT

//sample for regist a child command!  parent cmd :ls  child cmd :ls -l
    struct cli_command *pstParent = NULL;
    void *ls[] = {NULL, (void *)"ls", (void *)mgmtCmdLine, (void *)"Explanation :list all the files!", NULL};
    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, ls);
    pstParent = (cli_command *)ls[4];
    if (NULL != pstParent){
        void *lsl[] = {(void *)pstParent, (void *)"-l", (void *)mgmtCmdLine, (void *)"Explanation : list the files detail information!", NULL};
        hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, lsl);
    }

	void *logMgmtOut[] = {NULL, (void *)"logmgmtopen", (void *)mgmtCmdLine, (void *)"Explanation : Huawei libMgmt debug info ,Only for serial print!", NULL};
    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, logMgmtOut);

	void *logSet[] = {NULL, (void *)"logset", (void *)mgmtCmdLine, (void *)"Explanation : Set Module degbug level for logout!", NULL};
    void *logGet[] = {NULL, (void *)"logget", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
    void *logOut[] = {NULL, (void *)"logout", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
    void *logHelp[] = {NULL, (void *)"loghelp", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
    void *logStyle[] = {NULL, (void *)"logstyle", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};

    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, logSet);
    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, logGet);
    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, logOut);
    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, logHelp);
    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, logStyle);

#if defined(Huawei_v5) //template test macrovision and cgms-a
    void *logIpc[] = {NULL, (void *)"logipc", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
    void *vodSetma[] = {NULL, (void *)"vodsetma", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
    void *vodSetcg[] = {NULL, (void *)"vodsetcg", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
    void *vodSethd[] = {NULL, (void *)"vodsethd", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
    void *vodReset[] = {NULL, (void *)"vodreset", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
    void *vodHelp[] = {NULL, (void *)"vodhelp", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};

    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, logIpc);
    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, vodSetma);
    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, vodSetcg);
    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, vodSethd);
    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, vodReset);
    hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, vodHelp);
#endif

//something unknow
   void *openUrl[] = {NULL, (void *)"openurl", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *volume[] = {NULL, (void *)"volume", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *mute[] = {NULL, (void *)"mute", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *avd[] = {NULL, (void *)"avd", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *caWait[] = {NULL, (void *)"ca_wait", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *errLevel[] = {NULL, (void *)"err_level", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *cchTime[] = {NULL, (void *)"cch_time", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *size[] = {NULL, (void *)"size", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *cache[] = {NULL, (void *)"cache", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *audioChannel[] = {NULL, (void *)"audiochannel", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *getAudioChnnl[] = {NULL, (void *)"getaudiochnnl", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *hdcp[] = {NULL, (void *)"hdcp", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};

   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, openUrl);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, volume);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, mute);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, avd);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, caWait);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, errLevel);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, cchTime);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, size);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, cache);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, audioChannel);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, getAudioChnnl);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, hdcp);

//shell play!!!
   void *csave[] = {NULL, (void *)"csave", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *vod[] = {NULL, (void *)"vod", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *avod[] = {NULL, (void *)"avod", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *hls[] = {NULL, (void *)"hls", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *iptv[] = {NULL, (void *)"iptv", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *aiptv[] = {NULL, (void *)"aiptv", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *http[] = {NULL, (void *)"http", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *tstv[] = {NULL, (void *)"tstv", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *dvbs[] = {NULL, (void *)"dvbs", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *piptv[] = {NULL, (void *)"piptv", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *hpcm[] = {NULL, (void *)"hpcm", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *hmp3[] = {NULL, (void *)"hmp3", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *hmpa[] = {NULL, (void *)"hmpa", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};

   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, csave);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, vod);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, avod);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, hls);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, iptv);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, aiptv);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, http);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, tstv);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, dvbs);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, piptv);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, hpcm);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, hmp3);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, hmpa);

#ifdef INCLUDE_DVBS
   //void *pdvbs[] = {NULL, (void *)"pdvbs", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   //hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, );

#endif

#if SUPPORTE_HD == 1
   void *zebra[] = {NULL, (void *)"zebra", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *mosaic[] = {NULL, (void *)"mosaic", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *mset[] = {NULL, (void *)"mset", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *mget[] = {NULL, (void *)"mget", (void *)mgmtCmdLine, (void *)"Explanation: ", NULL};
   void *msave[] = {NULL, (void *)"msave", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};

   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, zebra);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, mosaic);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, mset);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, mget);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, msave);
#endif

#ifdef INCLUDE_PVR

   void *pvrInit[] = {NULL, (void *)"pvr_init", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *pvrList[] = {NULL, (void *)"pvr_list", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *fInfo[] = {NULL, (void *)"finfo", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *pvr[] = {NULL, (void *)"pvr", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *rcEnc[] = {NULL, (void *)"rc_enc", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *rcIptv[] = {NULL, (void *)"rc_iptv", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *rcClose[] = {NULL, (void *)"rc_close", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};

   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, pvrInit);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, pvrList);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, fInfo);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, pvr);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, rcEnc);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, rcIptv);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, rcClose);
#endif

   void *pause[] = {NULL, (void *)"pause", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *resume[] = {NULL, (void *)"resume", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *fast[] = {NULL, (void *)"fast", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *seek[] = {NULL, (void *)"seek", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *stop[] = {NULL, (void *)"stop", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *shift[] = {NULL, (void *)"shift", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *close[] = {NULL, (void *)"close", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *pClose[] = {NULL, (void *)"pclose", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *mpcm[] = {NULL, (void *)"mpcm", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *trans[] = {NULL, (void *)"trans", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *browser[] = {NULL, (void *)"browser", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *strmArq[] = {NULL, (void *)"strmarq", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *strmFcc[] = {NULL, (void *)"strmfcc", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *strmSqm[] = {NULL, (void *)"strmsqm", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *strmMsg[] = {NULL, (void *)"strmmsg", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *strmTrs[] = {NULL, (void *)"strmtrs", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};
   void *strmBst[] = {NULL, (void *)"strmbst", (void *)mgmtCmdLine, (void *)"Explanation : ", NULL};

   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, pause);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, resume);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, fast);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, seek);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, stop);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, shift);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, close);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, pClose);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, mpcm);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, trans);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, browser);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, strmArq);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, strmFcc);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, strmSqm);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, strmMsg);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, strmTrs);
   hmw_mgmtCliRegCmd(MGMT_CLI_PARM_NUM_REG, strmBst);

#endif //#ifdef INCLUDE_HMWMGMT
   return 0;
}

}

#ifndef MonitorCmd_h
#define MonitorCmd_h

#include "ParseCmd.h"
#include "MonitorManager.h"


class ParseCmd;
class MonitorManager;

class MonitorCmd{

public:
	MonitorCmd();
	virtual ~MonitorCmd();

    int CmdSort(const moni_buf_t pBuf);
	static MonitorCmd* GetInstance();

private:
	int InitCmdMap();
	int MonitorInitializeStateGet();
	void MonitorInitializeStateSet(int state);
	int MonitorIdentifyCheck(char *pIdentify);

private:
	typedef int (MonitorCmd::*cmdfunc)(moni_buf_t, int);
	//√¸¡Ó∫Ø ˝”≥…‰πÿœµ
	std::map<std::string, ParseCmd*> m_cmdMap;
	static MonitorCmd *m_monitorCmd;

};
#endif //MonitorCmd_h


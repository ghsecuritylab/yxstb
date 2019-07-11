#ifndef ParseCmd_h
#define ParseCmd_h

#include "MonitorTraceroute.h"
#include "mgmtDef.h"

#include <map>
#include <string>
extern "C" {
#include <stdio.h>
#include <string.h>
}


/***************************************
*ParseCmdClass
****************************************/
class ParseCmd {
public:
	ParseCmd();
	virtual ~ParseCmd();

	char* GetFuncCall();
	int Parse(char* pCmd, moni_buf_t pBuf);

protected:
	virtual int Exec(moni_buf_t pBuf)=0;

	char* m_szCmdInfo[4];
};

/***************************************
*ParseRead Class
****************************************/
class ParseRead : public ParseCmd {
public:
	ParseRead();
	~ParseRead();

private:
	int Exec(moni_buf_t pBuf);
};

/***************************************
*ParseWrite Class
****************************************/
class ParseWrite : public ParseCmd {
public:
	ParseWrite();
	~ParseWrite();

private:
	int Exec(moni_buf_t pBuf);
};

/***************************************
*ParseIoctl Class
****************************************/
class ParseIoctl : public ParseCmd {
public:
    ParseIoctl();
    ~ParseIoctl();

private:
    int Exec(moni_buf_t pBuf);
    static void* MonitorUpgrade(void* pSocketPort);

private:
    int m_tUpgradeReceivePort;
    FILE* m_pUpgradefile;
};

/***************************************
*ParseInform Class
****************************************/
class ParseInform : public ParseCmd {
public:
    ParseInform();
    ~ParseInform();

private:
    int Exec(moni_buf_t pBuf);
    int SetPara(HMW_MgmtMsgType &eMsgType, int &argc, void **argv);
    int stb_pcap_maxfilesize(moni_buf_t buf, int len);
    unsigned long GetFreeDiskBytes(const char* path);
    unsigned long GetFreeVarSize(void);
};

/***************************************
*ParseConnect Class
****************************************/
class ParseConnect : public ParseCmd {
public:
    ParseConnect();
    ~ParseConnect();

private:
    int Exec(moni_buf_t pBuf);
    int SetPara(HMW_MgmtMsgType &eMsgType, int &argc, void **argv);
};

/***************************************
*ParseInitialize Class
****************************************/
class ParseInitialize : public ParseCmd {
public:
    ParseInitialize();
    ~ParseInitialize();
    void sendLogErrorMsg(moni_buf_t pBuf);

private:
    int Exec(moni_buf_t pBuf);
    int Connection(unsigned char* key, unsigned char* output);
    int SetSessionID();
    int MonitorInitializeStateGet();
    void MonitorInitializeStateSet(int state);
    int MonitorIdentifyCheck(char* pIdentify);
    int get_monitor_aes_keys(unsigned char* buf, int len);
    int random_number_encrypt(unsigned char* key, unsigned char* output);
    int RSA_rndom_number_encrypt(char* input, unsigned char* output);

private:
    typedef int (ParseInitialize::*func)(unsigned char*, unsigned char*);
    std::map<std::string, func> m_initMap;
    std::map<std::string, func>::iterator m_iter;
};
#endif // ParseCmd_h


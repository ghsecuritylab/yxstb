#ifndef __MulticastDiagnose__H_
#define __MulticastDiagnose__H_

#include "NetworkDiagnose.h"

#ifdef __cplusplus

#include <string>

enum {
    eIGMP_NON,
    eIGMP_V2,
    eIGMP_V3,
};


class MulticastDiagnose : public NetworkDiagnose::DiagnoseProcess {
public:
    MulticastDiagnose();
    ~MulticastDiagnose(); 

    enum {
        eMulti_ADDRERR = -300,
        eMulti_SOCKERR,
        eMulti_TIMEOUT,
        eMulti_STOP,
        eMulti_OK = 0,
    };

    virtual int type() { return eTypeMulticastDiagnose; }
    virtual int start(NetworkDiagnose* netdiag);
    virtual int stop();

    void setMultiAddr(const char* addr) { mMultiAddr = addr; }
    void setLocalAddr(const char* addr) { mLocalAddr = addr; }
    void setSourceAddr(const char* addr) { mSourceAddr = addr; }
    void setMultiPort(int port) { mMultiPort = port; }
    void setIGMPVersion(int version) { mIGMPVersion = version; }

    const char* getMultiAddr() { return mMultiAddr.c_str(); }
    const char* getLocalAddr() { return mLocalAddr.c_str(); }
    const char* getSourceAddr() { return mSourceAddr.c_str(); }
    int getMultiPort() { return mMultiPort; }
    int getIGMPVersion() { return mIGMPVersion; }

private:
    int _MulticastV4();
    int _MulticastV6() { return -1; }

private:
    std::string mMultiAddr;
    std::string mLocalAddr;
    std::string mSourceAddr;
    int mMultiPort;
    int mIGMPVersion;
    int mTimeout;
};

#endif

#endif

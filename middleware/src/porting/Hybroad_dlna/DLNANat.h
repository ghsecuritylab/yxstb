
#pragma once

#ifndef __DLNANat_H__
#define __DLNANat_H__

#include <string>
#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"

namespace Hippo {

class DLNARegister;
class DLNANat : public MessageHandler {
public:
    virtual void handleMessage(Message* msg);
    void Start(const char * ip, int port, int interval);
    void sendHeartBit(int opType);

protected:
    friend class DLNARegister;
    static DLNANat* GetInstance(void);

private:
    DLNANat();
    virtual ~DLNANat();

    void handleHeartbitResult(void);
    int handle_tcp_Register( char* message );
    void stop_tcp(void);
    void Stop(void);
    void handleUrgConnectResult(int Result );

    std::string m_hbIp;
    int         m_hbPort;
    int         m_hbInterval;
    int         m_hbSock;
    int         m_inSock;
    int         m_outSock;
    int         firsttcpflag;
    int         UrgResultlen;
    int         serialNo;
    int         Heartbitflag;
    char	   stbAccount[32];
    char 	   stbId[64];
    char      UrgResult[2048];
    char      method[16];
    enum {
        NATTIMER_HEARTBIT = 0,
        NATTIMER_DATA,
    };
};

} // namespace Hippo


#endif // __DLNANat_H__


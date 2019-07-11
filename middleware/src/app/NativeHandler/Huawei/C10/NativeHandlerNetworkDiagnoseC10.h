#ifndef _NativeHandlerNetworkDiagnoseC10_H_
#define _NativeHandlerNetworkDiagnoseC10_H_

#include "NativeHandlerPublicC10.h"

#ifdef __cplusplus

namespace Hippo {

class NativeHandlerNetworkDiagnoseC10 : public NativeHandlerPublicC10{
public:
    NativeHandlerNetworkDiagnoseC10();
    ~NativeHandlerNetworkDiagnoseC10();

    virtual bool handleMessage(Message *msg);
    virtual void onActive(void);
    virtual void onUnactive(void);
    virtual NativeHandler::State state() { return NativeHandler::NetworkDiagnose; }
	virtual bool doNetworkProtocolConflict(int errcode, const char* ifname);
    virtual bool doNetworkConnectOk(int errcode, const char* ifname);
	virtual bool doNetworkDhcpError(int errcode, const char* ifname);
	virtual bool doNetworkPppoeError(int errcode, const char* ifname);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _NativeHandlerConfigC10_H_

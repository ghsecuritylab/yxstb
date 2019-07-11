#ifndef _MessageValueNetwork_H_
#define _MessageValueNetwork_H_

#define MV_Network_PhysicalUp           0x9000 // 1 NETWORK_CONNECT
#define MV_Network_PhysicalDown         0x9001 // 2 NETWORK_DISCONNECT
#define MV_Network_IpUnconflict         0x9002 // NETWORK_IP_UNCONFLICT
#define MV_Network_IpConflict           0x9003 // NETWORK_IP_CONFLICT
#if defined(Jiangsu)
#define MV_Network_PhysicalDown_OnOpenError         0x9004
#define MV_Network_PhysicalDown_DoOpenError         0x9005
#endif
#define MV_Network_DataLinkProbe        3
#define MV_Network_DataLinkTimeout      4
#define MV_Network_DataLinkBeRefused    5
#define MV_Network_DataLinkNameError    6
#define MV_Network_DataLinkEstablish    7
#define MV_Network_DataLinkLost         8
#define MV_Network_ProtocolUp           9
#define MV_Network_ProtocolDown         10
#define MV_Network_ProtocolConflict     11
#define MV_Network_DNSTimeout           12
#define MV_Network_DNSError             13
#define MV_Network_DhcpError            14
#define MV_Network_PppoeError           15
#define MV_Network_RefreshNetLED        16

#define MV_Network_UnlinkMonitor        100
#define MV_Network_WifiJoinFail         101
#define MV_Network_WifiJoinSuccess      102
#define MV_Network_WifiCheckSignal      103

#define MV_Network_PppoePassError       0x9583 // PPPOE_PASSWORD_ERROR

#define MV_Network_ConnectOk 20 //_NEW_NETWORK used templatly 

#ifdef __cplusplus

namespace Hippo {


} // namespace Hippo

#endif // __cplusplus

#endif // _MessageValueNetwork_H_

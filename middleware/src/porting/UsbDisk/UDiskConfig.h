#ifndef _UDISKCONFIG_H
#define _UDISKCONFIG_H

#include "sys_basic_macro.h"

#ifdef __cplusplus

namespace Hippo {

typedef struct _UserConfigNode {
	char 	nNtvUserAccount[USER_LEN];
	char 	nNtvUserPassword[USER_LEN+4];
	int 	nConnectType;
	char 	nNetUserAccount[USER_LEN];
	char 	nNetUserPassword[USER_LEN];
	char    nStbIP[NET_LEN];
	char    nNetmask[NET_LEN];
	char    nGateway[NET_LEN];
	char    nDns[NET_LEN];
	char    nDns2[NET_LEN];
	int     nFlag;
}UserConfig_s;

typedef struct _CommonConfigNode {
	 char nNetuseraccount[USER_LEN];
    char nNetuserpassword[USER_LEN];
    char nDefContAcc[USER_LEN];
    char nDefContPwd[USER_LEN];
    char nNtvuseraccount[USER_LEN];
    char nNtvuserpassword[USER_LEN];
    char nEdsAddr[URL_LEN];
    char nEdsAddr2[URL_LEN];
    char nNtpServer[USER_LEN];
    char nNtpServer2[USER_LEN];
    int  nStandardMode;
    int  nHDMode;
    int  nNetMode;
    int  nWatchDogSwitch;
    int  nIGMPVersion;
    int  nDirectPlay;
    int  nTMSHeartBit;
    int  nTMSHeartBitInterval;
    int  nTransportProtocol;
    int  nChannelSwitch;
    int  nVideoOutput;
    char nTMSUsername[URL_LEN];
    char nTMSPassword[URL_LEN];
    char nStbIP[NET_LEN];
    char nNetmask[NET_LEN];
    char nGateway[NET_LEN];
    char nDns[NET_LEN];
    char nDns2[NET_LEN];
    int  nFlag;
}CommonConfig_s;

typedef struct _AccountConfigNode {
	char nAccount[USER_LEN];
	char nUser[32]; // peaple name
	int  nIsInitialzed;
	UserConfig_s *nConfig;
	struct _AccountConfigNode *pNext;
}AccountConfig_s;

AccountConfig_s* UDiskGetUserConfigByIndex(int idx);
void UDiskSetUserConfigData(void);
void UDiskSetCommonConfigData(void);
int UDiskGetUserConfigByUserID(const char* pUserAccount);
int UDiskReadAccountConfigData(void);
int UDiskReadCommonConfigData(void);
int UDiskReadUserConfigData(const char* pAccount);
int UDiskChanageUserStatus(const char* account);

}
#endif

#endif

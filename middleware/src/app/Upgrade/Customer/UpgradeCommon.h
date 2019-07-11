#ifndef _UpgradeCommon_H_
#define _UpgradeCommon_H_

#include "UpgradeSource.h"

namespace Hippo {

void urlCheckSum(char* buf);
char *getItemValue(char* buf, const char* tag, char* value);
int getVersionNumAndFileName(char* buf, const char* item, int* ver, char* file);
int getLogoVersionNumAndFileName(char* buf,  const char* item, int* ver, char* filename, char* logomd5);
int constructRequestUpgradeAddrUrl(const char* ip, int port, char* upgradeUrl, char* requestUrl);
int constructRequestConfigUrl(const char* ip, int port, bool isTr069, int provider, char* upgradeUrl, char* requestUrl);
int GetUpgradeFileUrl(char* pConfigInfo, const char* ip, int port, char* upgradeUrl, bool isTr069, UpgradeSource* source);
void UpgradeSuccessed(int version);

typedef void (* BRUN_FUNC)(int);
int firmware_burn_ex(char *firmware, int len, BRUN_FUNC func);

}

#endif
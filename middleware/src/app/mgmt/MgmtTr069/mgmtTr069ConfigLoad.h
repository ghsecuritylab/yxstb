#ifndef _MGMTTR069CONFIGLOAD_H_
#define _MGMTTR069CONFIGLOAD_H_

#include "hmw_mgmtlib.h"

#ifdef __cplusplus
extern "C" {
#endif
void mgmt_load_ontime(int arg);
int  mgmtCpeConfigDownload(Mgmt_DownloadInfo *info);
int  mgmtCpeConfigUpload(Mgmt_UpLoadInfo *info);

#ifdef __cplusplus
}
#endif

#endif

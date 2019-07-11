
#ifndef _STBINFO_H_
#define _STBINFO_H_
#pragma once

#include "build_info.h"
#include "preconfig.h"

#ifdef __cplusplus
#include <string>

namespace StbInfo {
    namespace STB {
        namespace Version {
            inline const char * BuildTime(void) { return g_make_build_date; }
            inline const char * BuildAuther(void) { return g_make_build_name; }
            inline const char * BuildMachine(void) { return g_make_host_name; }
            inline const char * SourcePath(void) { return g_make_svn_path; }
            inline const char * MaketPlace(void) { return g_make_customer_name; }
            const char * Version(void);
            const char * HWVersion(void);
            const char * HWVersionWithPrefix(void);
            inline int Revision(void) { return g_make_svn_revision; }
        }
        inline const char * Model(void) { return DEFAULT_STBTYPE; }
        inline const char * UpgradeSection(void) { return UPGRADE_SECTION; }
        const char * UpgradeModel(void);
        const char * UpgradeModel(char index);
    }
    namespace Network {
        namespace Dhcp {
            inline int RetryTimes(void) { return DHCP_RETRY_TIMES; }
        }
        namespace PPPoE {
            inline int RetryTimes(void) { return PPPOE_RETRY_TIMES; }
        }
    }
    namespace Play {
        inline int HeartbitInterval(void) { return RTSP_HEARTBIT_INTERVAL; }
        inline int Unimulticast(void) { return RTSP_UNIMULTICAST; }
        inline int RRSTimeout(void) { return RTSP_RRSTIMEOUT; }
        inline bool EnableARQ(void) { return INCLUDE_ARQ; }
        inline bool EnableBurst(void) { return INCLUDE_BURST; }
    }

}

#else
char* StbVersionBuildtime(void);
const char * HwSoftwareVersion(int flag);
const char * StbModel(void);
const char * StbUpgradeModel(void);
int StbUpgradeVersion(char * out);
#endif


#endif








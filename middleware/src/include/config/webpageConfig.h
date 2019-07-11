#ifndef _pathWebpage_H_
#define _pathWebpage_H_

#include "pathConfig.h"

#if defined(ANDROID)

#ifndef ANDROID_PACKAGE_NAME
#error "android_package_name not defined."
#endif

#ifndef LOCAL_WEBPAGE_PATH_PREFIX
#define LOCAL_WEBPAGE_PATH_PREFIX "FILE:////data/data/" ANDROID_PACKAGE_NAME "/files/webpage"
#endif

#ifndef WEBPAGE_PATH_ROOT
#define WEBPAGE_PATH_ROOT "/data/data/" ANDROID_PACKAGE_NAME "/files/webpage"
#endif

#else
#ifndef WEBPAGE_PATH_ROOT
#define WEBPAGE_PATH_ROOT DEFAULT_RESOURCE_DATAPATH"/webpage"
#endif

#ifndef LOCAL_WEBPAGE_PATH_PREFIX
#define LOCAL_WEBPAGE_PATH_PREFIX "FILE:////home/hybroad/share/webpage"
#endif
#endif


#if defined (HUAWEI_C10)
#if defined(C30)
#include "webpageConfigC30.h"
#endif
#include "webpageConfigC10.h"
#else
#include "webpageConfigC20.h"
#endif

#ifndef LOCAL_WEBPAGE_PATH_ERROR
#define LOCAL_WEBPAGE_PATH_ERROR LOCAL_WEBPAGE_PATH_PREFIX"/error.html"
#endif

#ifndef LOCAL_WEBPAGE_PATH_TRANSPARENT
#define LOCAL_WEBPAGE_PATH_TRANSPARENT "/transparent.htm"
#endif

#ifndef LOCAL_WEBPAGE_PATH_USB_UPGRADE
#define LOCAL_WEBPAGE_PATH_USB_UPGRADE "/stb_monitor_upgrade_udisk.html"
#endif

#ifndef LOCAL_WEBPAGE_PATH_BOOT
#define LOCAL_WEBPAGE_PATH_BOOT "/boot.html"
#endif

#ifndef LOCAL_WEBPAGE_PATH_STANDBY
#define LOCAL_WEBPAGE_PATH_STANDBY "/standby.html"
#endif

//#ifndef LOCAL_WEBPAGE_PATH_TIMEOUT // United after error code does not need to have this error page open.
//#define LOCAL_WEBPAGE_PATH_TIMEOUT "/timeout.html"
//#endif

#ifndef LOCAL_WEBPAGE_PATH_USB_CONFIG
#define LOCAL_WEBPAGE_PATH_USB_CONFIG "/judge.html"
#endif

#ifndef LOCAL_WEBPAGE_PATH_CHECK_PPPOEACCOUNT
#define LOCAL_WEBPAGE_PATH_CHECK_PPPOEACCOUNT "/check_pppoeaccount.html"
#endif

#endif //_pathWebpage_H_


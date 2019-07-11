#ifndef __IPTVMIDDLEWAREJNI_H_
#define __IPTVMIDDLEWAREJNI_H_

#define NEW_ANDROID_SETTING
#define EC6106V8_TEST

#include "IPTVMiddleware.h"

#include <jni.h>
#include <utils/threads.h>

#define IPTV_CLASS_PATH_NAME "com/hybroad/iptv/app/IPTVMiddleWare"


class IPTVMiddlewareJNI : public IPTVMiddleware {
public:
    IPTVMiddlewareJNI(JNIEnv* env, jobject thiz, jobject refObj);
    ~IPTVMiddlewareJNI();
    //C/C++ call java interface
    void postEvent(int msg, int arg1, int arg2, void* obj = 0);
    void authSucessedToAndroid();
    void authFailedToAndroid();
private:
    jclass mRefClass;
    jobject mRefObject;
    struct JavaMiddlerware;
    JavaMiddlerware* mjavaMiddlerware;
};

#endif

#include "IPTVMiddlewareJNI.h"
#include "libzebra.h"
#include "nativehelper/jni.h"
#include "nativehelper/JNIHelp.h"
#include "utils/Log.h"

#include <assert.h>
#include <binder/Parcel.h>
#include <utils/threads.h>

#include "AndroidSettingJNI.h"
#include "AndroidNetworkJNI.h"
#include "RemoteServerJNI.h"
#include "SurfaceJNI.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

using namespace android;
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "IPTVMiddlewareJNI"

struct IPTVMiddlewareJNI::JavaMiddlerware {
    jmethodID mPostEvent;
    jmethodID mAuthSucessed;
    jmethodID mAuthFailed;
};

static JavaVM* g_JavaVM = NULL;
static pthread_key_t g_EnvKey;
static jfieldID g_InstanceID;

JNIEnv* _JNI_GetEnv();

IPTVMiddlewareJNI::IPTVMiddlewareJNI(JNIEnv* env, jobject thiz, jobject refObj)
    :mRefClass(0),mRefObject(0),mjavaMiddlerware(new JavaMiddlerware)
{

    jclass clazz = env->FindClass(IPTV_CLASS_PATH_NAME);
    if (clazz) {
        mRefClass = (jclass)env->NewGlobalRef(env->GetObjectClass(thiz));
        mRefObject = (jobject)env->NewGlobalRef(refObj);
        mjavaMiddlerware->mPostEvent = env->GetStaticMethodID(clazz, "postEventFromNative", "(Ljava/lang/Object;IIILjava/lang/Object;)V");
        mjavaMiddlerware->mAuthSucessed = env->GetStaticMethodID(clazz, "authSucessed", "()V");
        mjavaMiddlerware->mAuthFailed = env->GetStaticMethodID(clazz, "authFailed", "()V");
    }

}

IPTVMiddlewareJNI::~IPTVMiddlewareJNI()
{

    JNIEnv* env = _JNI_GetEnv();
    env->DeleteGlobalRef(mRefObject);
    env->DeleteGlobalRef(mRefClass);
}

void IPTVMiddlewareJNI::postEvent(int msg, int arg1, int arg2, void* obj)
{
    //LOGI("postEvent To Iptv Apk, what[%d] arg1[%d] arg2[%d] obj[%s] \n", msg, arg1, arg2, (char*)obj);
    JNIEnv* env = _JNI_GetEnv();
    if (env){
        if (obj != NULL){
            jstring jObj = env->NewStringUTF((char*)obj);
            env->CallStaticVoidMethod(mRefClass,  mjavaMiddlerware->mPostEvent, mRefObject, msg, arg1, arg2, jObj);
            env->DeleteLocalRef(jObj);
        }else{
            env->CallStaticVoidMethod(mRefClass,  mjavaMiddlerware->mPostEvent, mRefObject, msg, arg1, arg2, obj);
        }
    }
}

void IPTVMiddlewareJNI::authSucessedToAndroid()
{
    JNIEnv* env = _JNI_GetEnv();
    if (env)
        env->CallStaticVoidMethod(mRefClass, mjavaMiddlerware->mAuthSucessed);
}

void IPTVMiddlewareJNI::authFailedToAndroid()
{
    JNIEnv* env = _JNI_GetEnv();
    if (env)
        env->CallStaticVoidMethod(mRefClass, mjavaMiddlerware->mAuthFailed);
}

static void _IPTVMiddlewareJNI_jSetParameter(JNIEnv *env, jobject thiz, jint fileNameType, jint fileValueType, jstring jFieldName, jstring jFieldValue)
{
    LOGI("Iptv Apk SetParameter to Iptv MiddleWare.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);

    if (iptv){
        const char* fieldName = env->GetStringUTFChars(jFieldName, 0);
        const char* fieldValue = env->GetStringUTFChars(jFieldValue, 0);

        LOGD("Iptv Apk SetParameter to Iptv MiddleWare, fileNameType = %d, fileValueType = %d, fileName = %s, fieldValue = %s .\n", fileNameType, fileValueType, fieldName, fieldValue);

        if (fieldValue == NULL || fieldName == NULL){
            return ;
        }

        iptv->jSetParameter(fileNameType, fileValueType, fieldName, fieldValue);
    }
}

static void _IPTVMiddlewareJNI_jGetParameter(JNIEnv *env, jobject thiz, jint fileNameType, jint fileValueType, jstring jFieldName, jstring jFieldValue, jint fileLen)
{
    LOGI("Iptv Apk GetParameter From Iptv MiddleWare.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);

    if (iptv){
        const char* fieldName = env->GetStringUTFChars(jFieldName, 0);
        char* fieldValue = (char*)env->GetStringUTFChars(jFieldValue, 0);

        iptv->jGetParameter(fileNameType, fileValueType, fieldName, fieldValue, fileLen);
        LOGD("Iptv Apk GetParameter From Iptv MiddleWare, fileNameType = %d, fileValueType = %d, fileName = %s, fieldValue = %s .\n", fileNameType, fileValueType, fieldName, fieldValue);
    }
}

static void _IPTVMiddlewareJNI_openUrl(JNIEnv *env, jobject thiz, jstring jUrl)
{
    LOGI("openUrl.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    if (iptv) {
        const char* url = env->GetStringUTFChars(jUrl, 0);
        if (url) {
            LOGD("OpenUrl = %s.\n", url);
            iptv->openUrl(url);
            env->ReleaseStringUTFChars(jUrl, url);
        }
    }
}

static void _IPTVMiddlewareJNI_onAuthAndOpenMenu(JNIEnv *env, jobject thiz)
{
    LOGI("On Auth And Open Menu.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    if (iptv)
        iptv->onAuthAndOpenMenu();
}

static void _IPTVMiddlewareJNI_onDoOpenMenu(JNIEnv *env, jobject thiz)
{
    LOGI("onDoOpenMenu.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    if (iptv)
        iptv->onDoOpenMenu();

}
extern "C" int ygp_layer_setDisplaySurface(int surfaceHandle);

static void _IPTVMiddlewareJNI_init(JNIEnv *env, jobject thiz)
{
    LOGI("init.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    iptv->run();
}

static void _IPTVMiddlewareJNI_destroy(JNIEnv *env, jobject thiz)
{
    LOGI("destroy.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    if (iptv){
        iptv->destroy();
        delete(iptv);
    }
    pthread_key_delete(g_EnvKey);
    g_JavaVM = NULL;
}

static void _IPTVMiddlewareJNI_start(JNIEnv *env, jobject thiz)
{
    LOGI("start.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    if (iptv)
        iptv->start();
}

static void _IPTVMiddlewareJNI_restart(JNIEnv *env, jobject thiz)
{
    LOGI("restart.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    if (iptv)
        iptv->restart();
}

static void _IPTVMiddlewareJNI_stop(JNIEnv *env, jobject thiz)
{
    LOGI("stop.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    if (iptv)
        iptv->stop();
}

static void _IPTVMiddlewareJNI_pause(JNIEnv *env, jobject thiz)
{
    LOGI("pause.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    if (iptv)
        iptv->pause();
}

static void _IPTVMiddlewareJNI_resume(JNIEnv *env, jobject thiz)
{
    LOGI("resume.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    if (iptv)
        iptv->resume();
}

static void _IPTVMiddlewareJNI_sendKeyEvent(JNIEnv *env, jobject thiz, jint keyType, jint keyValue)
{
    LOGI("sendKeyEvent.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    if (iptv) {
        iptv->sendKeyEvent(keyType, keyValue);
    }
}

JNIEnv* _JNI_GetEnv()
{
    JNIEnv* env = (JNIEnv*)pthread_getspecific(g_EnvKey);
    if (!env) {
        LOGD("AttachCurrentThread.\n");
        if (g_JavaVM->AttachCurrentThread(&env, 0) != JNI_OK)
            return NULL;
        pthread_setspecific(g_EnvKey, env);
    }
    return env;
}

static void _JNI_ThreadDestructor(void* arg)
{
    JNIEnv *env = (JNIEnv*)arg;
    if (env) {
        g_JavaVM->DetachCurrentThread();
        pthread_setspecific(g_EnvKey, 0);
    }
}

static void _IPTVMiddlewareJNI_Native_Init(JNIEnv *env)
{
    LOGI("nativeInit.\n");
    if (pthread_key_create(&g_EnvKey, _JNI_ThreadDestructor))
        return ;
    pthread_setspecific(g_EnvKey, env);
    jclass clazz;
    if (!(clazz = env->FindClass(IPTV_CLASS_PATH_NAME)))
        return ;
    g_InstanceID = env->GetFieldID(clazz, "mNativeContext", "I");
}

static void _IPTVMiddlewareJNI_Native_Setup(JNIEnv *env, jobject thiz, jobject refObj)
{
    LOGI("nativeSetup.\n");
    IPTVMiddlewareJNI* iptv = new IPTVMiddlewareJNI(env, thiz, refObj);
    if (iptv) {
        env->SetIntField(thiz, g_InstanceID, (int)iptv);
    }
}

static void _IPTVMiddlewareJNI_jEnterStandby(JNIEnv *env, jobject thiz)
{
    LOGI("EnterStandby.\n");
    IPTVMiddlewareJNI* iptv = (IPTVMiddlewareJNI*)env->GetIntField(thiz, g_InstanceID);
    if (iptv){
        iptv->enterStandby();
        pthread_key_delete(g_EnvKey);
        g_JavaVM = NULL;
    }
}

static void _IPTVMiddlewareJNI_jKillAllProcess(JNIEnv *env, jobject thiz)
{
    LOGI("KillAllProcess.\n");
    yos_systemcall_runSystemCMD((char*)"busybox killall -9 cvm;busybox killall -9 com.hybroad.iptv.app;", NULL);
}

static void _IPTVMiddlewareJNI_jNotify(JNIEnv* env, jobject thiz, jstring jstr)
{
    const char* str = env->GetStringUTFChars(jstr, 0);
    if (str) {
        IPTVMiddleware_Notify(str);
    }
}

static JNINativeMethod sMethods[] =
{
    {"nativeinit",             "()V",                                      (void*)_IPTVMiddlewareJNI_Native_Init},
    {"nativesetup",            "(Ljava/lang/Object;)V",                    (void*)_IPTVMiddlewareJNI_Native_Setup},
    {"openUrl",                "(Ljava/lang/String;)V",                    (void*)_IPTVMiddlewareJNI_openUrl},
    {"onAuthAndOpenMenu",      "()V",                                      (void*)_IPTVMiddlewareJNI_onAuthAndOpenMenu},
    {"onDoOpenMenu",           "()V",                                      (void*)_IPTVMiddlewareJNI_onDoOpenMenu},
    {"initMiddleware",         "()V",                                      (void*)_IPTVMiddlewareJNI_init},
    {"destroy",                "()V",                                      (void*)_IPTVMiddlewareJNI_destroy},
    {"start",                  "()V",                                      (void*)_IPTVMiddlewareJNI_start},
    {"restart",                "()V",                                      (void*)_IPTVMiddlewareJNI_restart},
    {"pause",                  "()V",                                      (void*)_IPTVMiddlewareJNI_pause},
    {"resume",                 "()V",                                      (void*)_IPTVMiddlewareJNI_resume},
    {"stop",                   "()V",                                      (void*)_IPTVMiddlewareJNI_stop},
    {"sendKeyEvent",           "(II)Z",                                    (void*)_IPTVMiddlewareJNI_sendKeyEvent},
    {"setNativeParameter",     "(IILjava/lang/String;Ljava/lang/String;)V",(void*)_IPTVMiddlewareJNI_jSetParameter},
    {"enterStandby",           "()V",                                      (void*)_IPTVMiddlewareJNI_jEnterStandby},
    {"killAllProcess",         "()V",                                      (void*)_IPTVMiddlewareJNI_jKillAllProcess},
    {"notify",                  "(Ljava/lang/String;)V",                   (void*)_IPTVMiddlewareJNI_jNotify},
};

void registNativeMethods(JNIEnv *env, jclass clazz)
{
	LOGI("registNativeMethods\n");
    env->RegisterNatives(clazz, sMethods, sizeof(sMethods) / sizeof(sMethods[0]));
    
    registSettingNatives(env);
    registNetworkNatives(env);
    registRemoteServerNatives(env);
    registSurfaceNatives(env);
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    LOGI("JNI_OnLoad\n");
    jclass clazz = NULL;
    JNIEnv* env = NULL;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_4) != JNI_OK)
        return JNI_ERR;

    if (!(clazz = env->FindClass(IPTV_CLASS_PATH_NAME)))
        return JNI_ERR;

    registNativeMethods(env, clazz);
    g_JavaVM = vm;
    return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM* vm, void* reserved)
{
    LOGI("JNI_Unload\n");
}

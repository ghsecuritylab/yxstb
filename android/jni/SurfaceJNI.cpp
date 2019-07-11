#include "SurfaceJNI.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"
#include "nativehelper/jni.h"
#include "nativehelper/JNIHelp.h"
#include "utils/Log.h"
#include "libzebra.h"

#include <gui/Surface.h>

#include <stdio.h>
#include <string.h>
#include <string>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SurfaceJNI"

extern JNIEnv* _JNI_GetEnv();

using namespace android;

struct SurfaceJNI::SurfaceMethod{
    jmethodID   m_showIme;
    jmethodID   m_hideIme;
};

static jfieldID gInstanceID;
SurfaceJNI* SurfaceJNI::s_surface = 0;

SurfaceJNI::SurfaceJNI(JNIEnv* env, jobject thiz, jobject refObj)
    :m_RefClass(0), m_RefObject(0), m_SurfaceHandle(0), m_surfaceMethod(new SurfaceMethod)
{
    s_surface = this;
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz) {
        m_RefClass = (jclass)env->NewGlobalRef(clazz);
        m_RefObject = env->NewGlobalRef(refObj);
        m_surfaceMethod->m_showIme = env->GetMethodID(clazz, "showIme", "(I)V");
        m_surfaceMethod->m_hideIme = env->GetMethodID(clazz, "hideIme", "()V");
    }
}

SurfaceJNI::~SurfaceJNI()
{
    JNIEnv* env = _JNI_GetEnv();
    env->DeleteGlobalRef(m_RefObject);
    env->DeleteGlobalRef(m_RefClass);
    m_RefObject = 0;
    m_RefClass = 0;
}

void 
SurfaceJNI::setSurface(JNIEnv *env, jint surfaceInt, jobject jsurface)
{
    if (jsurface) {
       sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));

       LOGI("setSurface->Epg Surface :[%d]: %d", __LINE__, (int)surface.get());
       if (m_SurfaceHandle != (int)surface.get()) {
           ygp_layer_setDisplaySurface((int)surface.get());
       }
       m_SurfaceHandle = (int)surface.get();
    } else {
        jniThrowException(env, "java/lang/IllegalArgumentException",  "The surface has been released.");
        return;
    }
}

void 
SurfaceJNI::clearSurface()
{
    ygp_layer_clearSurface();
}

void 
SurfaceJNI::showIme(int top)
{
    JNIEnv* env = _JNI_GetEnv();
    if (env)
        env->CallVoidMethod(m_RefObject, m_surfaceMethod->m_showIme, top);
        
     return ;
}

void 
SurfaceJNI::hideIme()
{
    JNIEnv* env = _JNI_GetEnv();
    if (env)
        env->CallVoidMethod(m_RefObject, m_surfaceMethod->m_hideIme);
    return ;
}

SurfaceJNI& SurfaceJNIAgent()
{
    return *(SurfaceJNI::s_surface);
}

static void setSurfaceJNI(JNIEnv *env, jobject thiz, jint surfaceInt, jobject jsurface)
{
    LOGI("setEpgSurface.\n");
    SurfaceJNI* surfaceView = (SurfaceJNI*)env->GetIntField(thiz, gInstanceID);
    if (surfaceView)
        surfaceView->setSurface(env, surfaceInt, jsurface);
    
    return ;
}

static void clearSurfaceJNI(JNIEnv *env, jobject thiz)
{
    LOGI("clearSurfaceJNI.\n");
    SurfaceJNI* surfaceView = (SurfaceJNI*)env->GetIntField(thiz, gInstanceID);
    if (surfaceView)
        surfaceView->clearSurface();
    
    return ;
}

void AndroidSurface_showIme(int top)
{
    LOGI("Surface_showIme.\n");
    SurfaceJNIAgent().showIme(top);  
    return;
}

void AndroidSurface_hideIme()
{
    LOGI("Surface_showIme.\n");
    SurfaceJNIAgent().hideIme();    
    return;
}

static void Surface_NativeSetupJNI(JNIEnv *env, jobject thiz, jobject refObj)
{
    LOGI("nativeSetup.\n");
    SurfaceJNI* surface = new SurfaceJNI(env, thiz, refObj);
    if (surface) {
        env->SetIntField(thiz, gInstanceID, (int)surface);
    }
    
    return ;
}

static JNINativeMethod sMethods[] = {
    {"nativeSurfaceInit",          "(Ljava/lang/Object;)V",              (void*)Surface_NativeSetupJNI},
    {"nativeSetSurface",           "(ILandroid/view/Surface;)V",         (void*)setSurfaceJNI},
    {"nativeClearSurface",         "()V",                                (void*)clearSurfaceJNI},
};

void registSurfaceNatives(JNIEnv* env)
{
	LOGI("regist Surface natives methods.\n");
    jclass clazz = NULL;
    
    if (!(clazz = env->FindClass("com/hybroad/iptv/app/HybroadSurfaceView")))
        return ;
    
    env->RegisterNatives(clazz, sMethods, sizeof(sMethods) / sizeof(sMethods[0]));
    
    gInstanceID = env->GetFieldID(clazz, "mNativeClass", "I");
}

#ifndef SurfaceJNI_h
#define SurfaceJNI_h

#include "nativehelper/jni.h"

class SurfaceJNI {
public:
    SurfaceJNI(JNIEnv* env, jobject thiz, jobject refObj);
    ~SurfaceJNI();	
    
    static SurfaceJNI* s_surface;
    void setSurface(JNIEnv *env, jint surfaceInt, jobject jsurface);
    void clearSurface();
    void showIme(int top);
    void hideIme();
    
private:
    jclass m_RefClass;
    jobject m_RefObject;
    int m_SurfaceHandle;
    struct SurfaceMethod;
    struct SurfaceMethod* m_surfaceMethod;
};

void registSurfaceNatives(JNIEnv* env);

#endif
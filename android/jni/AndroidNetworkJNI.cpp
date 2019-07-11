#include "AndroidNetworkJNI.h"
#include "nativehelper/jni.h"
#include "utils/Log.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AndroidNetworkJNI"

struct NetworkMethod {
    jmethodID nGetNetowrkInfo;
}gNetworkMethod;

static jclass g_RefClass;

extern JNIEnv* _JNI_GetEnv();

int getAndroidNetworkInfo(const char* iface, const char* name, char* value, int len)
{
	JNIEnv* env = _JNI_GetEnv();
	const char* txt = 0;
    if (iface && name && value && env) {
		jstring jIface = env->NewStringUTF(iface);
		jstring jName  = env->NewStringUTF(name);
        jstring jValue = (jstring)env->CallStaticObjectMethod(g_RefClass, gNetworkMethod.nGetNetowrkInfo, jIface, jName);

        if (jValue == NULL) {
        	env->DeleteLocalRef(jValue);
            env->DeleteLocalRef(jName );
            env->DeleteLocalRef(jIface);
        	return -1;
        } else	{
            if (env->GetStringUTFLength(jValue) > 0) {
                txt = env->GetStringUTFChars(jValue, 0);
                strncpy(value, txt, len - 1);
                env->ReleaseStringUTFChars(jValue, txt);
            }
            env->DeleteLocalRef(jValue);
            env->DeleteLocalRef(jName );   
            env->DeleteLocalRef(jIface);
            LOGI("NetworkInfoGet iface = %s name = %s, value = %s.\n", iface, name, value);
            return 0;
        } 
       
    }
}
	
void nativeNetworkJNI(JNIEnv* env, jobject obj)
{
	 LOGI("nativeNetworkJNI\n");
     g_RefClass = (jclass)env->NewGlobalRef(env->GetObjectClass(obj));
}

static JNINativeMethod sMethods[] = {
    { "nativeNetworkInit", "()V", (void*)nativeNetworkJNI },
};

void registNetworkNatives(JNIEnv* env)
{
	LOGI("registNativeMethods\n");
    jclass clazz = NULL;
    
    if (!(clazz = env->FindClass(NETWORK_CLASS_PATH_NAME)))
		return ;
	env->RegisterNatives(clazz, sMethods, sizeof(sMethods) / sizeof(sMethods[0]));

	gNetworkMethod.nGetNetowrkInfo = env->GetStaticMethodID(clazz, "getNetworkInfo", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
}

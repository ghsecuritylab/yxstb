#include "AndroidSettingJNI.h"
#include "nativehelper/jni.h"
#include "utils/Log.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AndroidSettingJNI"

struct SettingMethod{
    jmethodID   m_get;
    jmethodID   m_set;
    jmethodID   m_regist;
}gSettingMethod;

static jclass g_RefClass;

extern JNIEnv* _JNI_GetEnv();

int setAndroidSetting(const char* name, const char* value)
{
    LOGI("setAndroidSetting name = %s, value = %s.\n", name, value);
    JNIEnv* env = _JNI_GetEnv();
    int ret = -1;
    if (name && value && env) {
        jstring jParam = env->NewStringUTF(name);
        jstring jValue = env->NewStringUTF(value);
        ret = env->CallStaticIntMethod(g_RefClass, gSettingMethod.m_set, jParam, jValue); 
        env->DeleteLocalRef(jParam);
        env->DeleteLocalRef(jValue);
    }
    return ret;
}

int getAndroidSetting(const char* name, char* value, int len)
{
	JNIEnv* env = _JNI_GetEnv();
	const char* txt = 0;
    if (name && value && env) {
		jstring jParam = env->NewStringUTF(name);
        jstring jValue = (jstring)env->CallStaticObjectMethod(g_RefClass, gSettingMethod.m_get, jParam);

        if (jValue == NULL) {
        	env->DeleteLocalRef(jValue);
            env->DeleteLocalRef(jParam);
        	return -1;
        } else	{
           	if (env->GetStringUTFLength(jValue) > 0) {
                txt = env->GetStringUTFChars(jValue, 0);
                strncpy(value, txt, len - 1);
               LOGD("getAndroidSetting::getParam %s: is %s...", name, value);
                env->ReleaseStringUTFChars(jValue, txt);
            }
            env->DeleteLocalRef(jValue);
            env->DeleteLocalRef(jParam);   
            return 0;
        } 
       
    }
}
	
void nativeSettingJNI(JNIEnv* env, jobject obj)
{
	 LOGI("nativeSettingJNI\n");
     g_RefClass = (jclass)env->NewGlobalRef(env->GetObjectClass(obj));
}

static JNINativeMethod sMethods[] = {
    { "nativeSettingInit", "()V", (void*)nativeSettingJNI },
};

void registSettingNatives(JNIEnv* env)
{
    LOGI("regist Setting NativeMethods\n");
    jclass clazz = NULL;
    
    if (!(clazz = env->FindClass("com/hybroad/iptv/param/ParamManager")))
		return ;
	env->RegisterNatives(clazz, sMethods, sizeof(sMethods) / sizeof(sMethods[0]));

    gSettingMethod.m_get = env->GetStaticMethodID(clazz, "get", "(Ljava/lang/String;)Ljava/lang/String;");
    gSettingMethod.m_set = env->GetStaticMethodID(clazz, "set", "(Ljava/lang/String;Ljava/lang/String;)I");
}

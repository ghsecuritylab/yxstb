#include "RemoteServerJNI.h"
#include "nativehelper/jni.h"
#include "XmppandroidControlMessageParser.h"
#include "utils/Log.h"

#include <stdio.h>
#include <string.h>
#include <string>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RemoteServerJNI"

static void RemoteServerSend_JNI(JNIEnv *env, jobject thiz, jstring jstr)
{
    LOGI("RemoteServerSend.\n");

    const char* str = env->GetStringUTFChars(jstr, 0);
    if (str) {
        LOGD("RemoteServerSend = %s.\n", str);
        std::string newstr;
		newstr = str;
		char result[1024] = {0};
		gloox::XmppAndroidControlMessageParser * RemoteServer =  gloox::XmppAndroidControlMessageParser::GetInstance();
		if (!RemoteServer) {
		    LOGD("RemoteServer err ...");
		    env->ReleaseStringUTFChars(jstr, str);
		    return;
		}
		RemoteServer->startParseMessageAndroid(newstr,result,1024);
    }
    env->ReleaseStringUTFChars(jstr, str);
}

static jstring  RemoteServerGet_JNI(JNIEnv *env,jobject thiz,jstring jstr)
{
    LOGI("RemoteServerGet.\n");
    char result[1024] = {0};
     const char* str = env->GetStringUTFChars(jstr, 0);
    if (str) {
        LOGD("RemoteServerGet = %s.\n", str);
        std::string newstr ;
        
	    newstr = str;
	    gloox::XmppAndroidControlMessageParser * RemoteServer =  gloox::XmppAndroidControlMessageParser::GetInstance();
	    if (!RemoteServer) {
	        LOGD("RemoteServer err ...");
	        env->ReleaseStringUTFChars(jstr, str);
	        return env->NewStringUTF(result);
	    }
	    RemoteServer->startParseMessageAndroid(newstr,result,1024);
    }
    env->ReleaseStringUTFChars(jstr, str);
    return env->NewStringUTF(result);
}

static JNINativeMethod sMethods[] = {
    { "RemoteServerSend", "(Ljava/lang/String;)V", (void*)RemoteServerSend_JNI },
    { "RemoteServerGet", "(Ljava/lang/String;)Ljava/lang/String;", (void*)RemoteServerGet_JNI },
};

void registRemoteServerNatives(JNIEnv* env)
{
	LOGI("regist RemoteServer natives methods.\n");
    jclass clazz = NULL;
    
    if (!(clazz = env->FindClass("com/hybroad/iptv/app/RemoteServer")))
		return ;
	env->RegisterNatives(clazz, sMethods, sizeof(sMethods) / sizeof(sMethods[0]));
}

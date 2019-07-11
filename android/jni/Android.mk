LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng

LOCAL_MODULE := libiptvmw_jni

LOCAL_SRC_FILES := ./net_manager_interface/IHybroadService.cpp \
	./net_manager_interface/BpHybroadService.cpp \
	./net_manager_interface/ICallbackListener.cpp \
	./net_manager_interface/BpCallbackListener.cpp \
	./net_manager_interface/CallbackListener.cpp \
	./net_manager_interface/nm_interface.cpp \
	AndroidSettingJNI.cpp \
	AndroidNetworkJNI.cpp \
	RemoteServerJNI.cpp \
	SurfaceJNI.cpp \
	IPTVMiddlewareJNI.cpp 
  
LOCAL_C_INCLUDES += ${NDK_PROJECT_PATH}/../middleware/src/app
LOCAL_C_INCLUDES += ${NDK_PROJECT_PATH}/../middleware/src/app/Setting
LOCAL_C_INCLUDES += ${NDK_PROJECT_PATH}/../middleware/src/xmpp
LOCAL_C_INCLUDES += ${WORKING_DIR}/platform_sdk/include
LOCAL_C_INCLUDES += ${WORKING_DIR}/android_sdk/include/system
LOCAL_C_INCLUDES += ${WORKING_DIR}/android_sdk/include
LOCAL_C_INCLUDES += ${WORKING_DIR}/android_sdk/include/bionic
LOCAL_C_INCLUDES += ${WORKING_DIR}/android_sdk/include/bionic/libstdc++/include
LOCAL_C_INCLUDES += ${WORKING_DIR}/android_sdk/include/stlport

LOCAL_C_INCLUDES += ${LOCAL_PATH}

WHOLE_LIB :=
WHOLE_LIB += $(WORKING_DIR)/takin_sdk/lib/libTakin.a 
WHOLE_LIB += $(WORKING_DIR)/takin_sdk/lib/libwebkit-takin.a 
WHOLE_LIB += $(WORKING_DIR)/takin_sdk/lib/libwebcore-takin.a 
WHOLE_LIB += $(WORKING_DIR)/takin_sdk/lib/libHippoGlue.a
WHOLE_LIB += $(WORKING_DIR)/takin_sdk/lib/libjsc.a 
WHOLE_LIB += $(WORKING_DIR)/takin_sdk/lib/libwtf.a 
WHOLE_LIB += ${WORKING_DIR}/lib/libandroidmw.a 

LOCAL_LDFLAGS += -nostdlib
LOCAL_LDFLAGS += -Wl,-soname,libiptvmw_jni.so -Wl,--gc-sections
LOCAL_LDFLAGS += -L${NDK_PROJECT_PATH}/../../platform/hi3716m/release/SQA_lib/AndroidSQA
LOCAL_LDFLAGS += -L${WORKING_DIR}/android_sdk/lib
LOCAL_LDFLAGS += -L$(WORKING_DIR)/third_party/lib
LOCAL_LDFLAGS += -L${WORKING_DIR}/platform_sdk/lib
LOCAL_LDFLAGS += -llog -lc -lutils -lstdc++ -licuuc -lstlport -ldl -lcurl_withssl -lcrypto  -lssl -lgloox -lzebra -landroid_runtime -lnativehelper -lbinder -lui -lcutils -ljpeg -lfreetype2 -lsqa -Wl,--whole-archive ${WHOLE_LIB} -Wl,--no-whole-archive -lxml2 -lcairo -lgcc

include $(BUILD_SHARED_LIBRARY) 

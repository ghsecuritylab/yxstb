LOCAL_PATH:= $(call my-dir)

##############################################################

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES:= common/mid_select.c \
	common/mid_msgq.c \
	common/mid_timer.c
 
LOCAL_MODULE := libcommon

LOCAL_SHARED_LIBRARIES := libcutils libutils 

LOCAL_C_INCLUDES += $(LOCAL_PATH)/dbg_utils

include $(BUILD_STATIC_LIBRARY)

##############################################################

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES:= json/arraylist.c \
	json/debug.c \
	json/json_object.c \
	json/json_public.c \
	json/json_tokener.c \
	json/json_util.c \
	json/linkhash.c \
	json/printbuf.c
 
LOCAL_MODULE := libjson

LOCAL_C_INCLUDES += $(LOCAL_PATH)/json

LOCAL_SHARED_LIBRARIES := libcutils libutils libbinder

include $(BUILD_STATIC_LIBRARY)

##############################################################

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES:= HybroadLogcat/DataRecive.cpp \
	HybroadLogcat/DataSink.cpp \
	HybroadLogcat/DataSource.cpp \
	HybroadLogcat/LogFilter.cpp \
	HybroadLogcat/LogPool.cpp \
	HybroadLogcat/LogPostTerminal.cpp \
	HybroadLogcat/LogPostUDP.cpp \
	HybroadLogcat/LogInit.cpp \
	HybroadLogcat/MonitorToolLog.cpp \
	HybroadLogcat/MonitorLogPostUDP.cpp \
	HybroadLogcat/MonitorLogPostFTP.cpp \
	HybroadLogcat/PlatBuffer.cpp \
	HybroadLogcat/RingBuffer.cpp \
	HybroadLogcat/tr069_log.cpp 	
 
LOCAL_MODULE := libHybroadLogCat

LOCAL_C_INCLUDES += $(LOCAL_PATH)/HybroadLogcat

LOCAL_C_INCLUDES += $(LOCAL_PATH)/dbg_utils

LOCAL_C_INCLUDES += $(LOCAL_PATH)/common

include external/stlport/libstlport.mk

include $(BUILD_STATIC_LIBRARY)

#########################################################

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES:= android/IHybroadService.cpp \
 android/BpHybroadService.cpp \
 android/HybroadService.cpp \
 android/ICallbackListener.cpp \
 android/BpCallbackListener.cpp \
 android/CallbackListener.cpp \
 android/Android_port.cpp \
 android/hash_func.cpp \
 android/param_map.cpp \
 android/local_service.cpp \
 android/init.cpp
 
LOCAL_MODULE := libHybroadService

LOCAL_C_INCLUDES += $(LOCAL_PATH)/android

LOCAL_C_INCLUDES += $(LOCAL_PATH)/dbg_utils

LOCAL_C_INCLUDES += $(LOCAL_PATH)/json

LOCAL_C_INCLUDES += $(LOCAL_PATH)/tr069_include

LOCAL_C_INCLUDES += $(LOCAL_PATH)/stb_monitor_include

LOCAL_C_INCLUDES += $(LOCAL_PATH)/HybroadLogcat
LOCAL_C_INCLUDES += $(LOCAL_PATH)/tr069PactetCapture

LOCAL_SHARED_LIBRARIES := libcutils libutils libbinder

include external/stlport/libstlport.mk

include $(BUILD_STATIC_LIBRARY)

##############################################################

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES:= dbg_utils/nm_dbg.c \
	tr069PactetCapture/tr069_port_PacketCapture.c \
	common/aesCrypto.c\
    common/yx_crypto.c\
	tr069_interface.c \
	main.cpp
	
LOCAL_CFLAGS += -DANDROID_LOGCAT_OUTPUT
 
LOCAL_MODULE := net_manager

LOCAL_SHARED_LIBRARIES := libcrypto libstlport libutils libbinder libz libcurl_withssl

LOCAL_STATIC_LIBRARIES := libHybroadService libjson libHybroadLogCat libcommon

LOCAL_LDFLAGS = $(LOCAL_PATH)/libs/libstbMonitor.a $(LOCAL_PATH)/libs/libtr069.a 


LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_C_INCLUDES += $(LOCAL_PATH)/stb_monitor_include

LOCAL_C_INCLUDES += external/openssl/crypto/md5

LOCAL_C_INCLUDES += external/openssl/include/

LOCAL_C_INCLUDES += $(LOCAL_PATH)/android

LOCAL_C_INCLUDES += $(LOCAL_PATH)/tr069_include

LOCAL_C_INCLUDES += $(LOCAL_PATH)/dbg_utils

LOCAL_C_INCLUDES += $(LOCAL_PATH)/json

LOCAL_C_INCLUDES += $(LOCAL_PATH)/common

include external/stlport/libstlport.mk

include $(BUILD_EXECUTABLE)



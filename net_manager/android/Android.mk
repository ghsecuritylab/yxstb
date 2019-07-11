LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES:= IHybroadService.cpp \
 BpHybroadService.cpp \
 HybroadService.cpp \
 ICallbackListener.cpp \
 BpCallbackListener.cpp \
 CallbackListener.cpp \
 Android_port.cpp \
 init.cpp
 
LOCAL_MODULE := libHybroadService

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/callback

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := libcutils libutils libbinder

include $(BUILD_STATIC_LIBRARY)

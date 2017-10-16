LOCAL_PATH := $(realpath $(call my-dir)/../../..)
PVRSDKDIR := $(realpath $(call my-dir)/../../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE     := PVREngineUtils
LOCAL_SRC_FILES  := $(notdir $(wildcard $(LOCAL_PATH)/*.cpp))

LOCAL_CFLAGS     := $(SDK_BUILD_FLAGS)

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Framework/ $(PVRSDKDIR)/Builds/Include/

include $(BUILD_STATIC_LIBRARY)

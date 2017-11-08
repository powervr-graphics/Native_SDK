LOCAL_PATH := $(realpath $(call my-dir)/../../../..)
PVRSDKDIR := $(realpath $(call my-dir)/../../../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE     := PVRUtilsGles
LOCAL_SRC_FILES  := $(notdir $(wildcard $(LOCAL_PATH)/*.cpp))\
                    $(addprefix OGLES/, $(notdir $(wildcard $(LOCAL_PATH)/OGLES/*.cpp)))\
                    $(addprefix EGL/, $(notdir $(wildcard $(LOCAL_PATH)/EGL/*.cpp)))

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Framework/ $(PVRSDKDIR)/Builds/Include/

LOCAL_CFLAGS     := $(SDK_BUILD_FLAGS)

include $(BUILD_STATIC_LIBRARY)

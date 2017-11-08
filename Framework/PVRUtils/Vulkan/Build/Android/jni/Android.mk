LOCAL_PATH := $(realpath $(call my-dir)/../../../..)
PVRSDKDIR  := $(realpath $(call my-dir)/../../../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE     := PVRUtilsVk
LOCAL_SRC_FILES  := $(notdir $(wildcard $(LOCAL_PATH)/*.cpp))\
                    $(addprefix Vulkan/, $(notdir $(wildcard $(LOCAL_PATH)/Vulkan/*.cpp)))

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Framework/ $(PVRSDKDIR)/Builds/Include/

LOCAL_CFLAGS     := $(SDK_BUILD_FLAGS)

include $(BUILD_STATIC_LIBRARY)

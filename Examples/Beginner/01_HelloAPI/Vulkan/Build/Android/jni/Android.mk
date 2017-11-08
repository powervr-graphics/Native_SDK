LOCAL_PATH := $(call my-dir)/../../..
PVRSDKDIR := $(realpath $(call my-dir)/../../../../../../..)

ASSETDIR := $(PVRSDKDIR)/Examples/Beginner/01_HelloAPI/Vulkan/Build/Android/assets



# Module VulkanHelloAPI
include $(CLEAR_VARS)

LOCAL_MODULE    := VulkanHelloAPI

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := vk_getProcAddrs.cpp \
                    VulkanHelloAPI.cpp \
                    MainAndroid.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/. \
                    $(PVRSDKDIR)/Framework \
                    $(PVRSDKDIR)/Builds/Include



LOCAL_LDLIBS := -latomic \
                -llog \
                -landroid

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

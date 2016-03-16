LOCAL_PATH := $(call my-dir)/../../..
PVRSDKDIR := $(realpath $(call my-dir)/../../../../../../..)


# Module VulkanHelloAPI
include $(CLEAR_VARS)

LOCAL_MODULE    := VulkanHelloAPI

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := VulkanHelloAPI_Android.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Builds/Include

LOCAL_LDLIBS := -llog \
                -landroid 

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

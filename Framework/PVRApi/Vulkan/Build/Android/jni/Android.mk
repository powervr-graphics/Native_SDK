LOCAL_PATH := $(realpath $(call my-dir)/../../../..)
PVRSDKDIR  := $(realpath $(call my-dir)/../../../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE     := PVRVulkan
LOCAL_SRC_FILES  := $(notdir $(wildcard $(LOCAL_PATH)/*.cpp))\
                    $(addprefix Vulkan/, $(notdir $(wildcard $(LOCAL_PATH)/Vulkan/*.cpp)))

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Framework/ $(PVRSDKDIR)/Builds/Include/

LOCAL_CFLAGS := $(SDK_BUILD_FLAGS)

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_STATIC_LIBRARY)
$(call import-module,android/native_app_glue) 

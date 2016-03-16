LOCAL_PATH := $(realpath $(call my-dir)/../../../..)
PVRSDKDIR  := $(realpath $(call my-dir)/../../../../../..)

$(info $(LOCAL_PATH))
$(info $(PVRSDKDIR))

include $(CLEAR_VARS)
   
LOCAL_MODULE     := PVRNativeVulkan
LOCAL_SRC_FILES  := $(notdir $(wildcard $(LOCAL_PATH)/*.cpp))\
                    $(addprefix Vulkan/, $(notdir $(wildcard $(LOCAL_PATH)/Vulkan/*.cpp)))

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Framework/ \
                    $(PVRSDKDIR)/Builds/Include/
					
					

LOCAL_CFLAGS := $(SDK_BUILD_FLAGS)

LOCAL_STATIC_LIBRARIES += PVRVulkanGlue PVRAssets PVRCore android_native_app_glue

include $(BUILD_STATIC_LIBRARY)
$(call import-module,android/native_app_glue) 
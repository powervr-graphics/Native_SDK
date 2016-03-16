LOCAL_PATH := $(realpath $(call my-dir)/../../../..)
PVRFRAMEWORKDIR := $(realpath $(call my-dir)/../../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE      := PVRVulkanGlue
LOCAL_SRC_FILES := 	$(notdir $(wildcard $(LOCAL_PATH)/*.cpp)) \
                    $(addprefix Vulkan/, $(notdir $(wildcard $(LOCAL_PATH)/Vulkan/*.cpp)))

LOCAL_CFLAGS := $(SDK_BUILD_FLAGS)
LOCAL_C_INCLUDES := $(PVRFRAMEWORKDIR) $(PVRFRAMEWORKDIR)/../Builds/Include $(PVRFRAMEWORKDIR)/PVRPlatformGlue/Vulkan

LOCAL_STATIC_LIBRARIES += PVRCore android_native_app_glue

include $(BUILD_STATIC_LIBRARY)
$(call import-module,android/native_app_glue) 


LOCAL_PATH := $(call my-dir)/../../..
PVRSDKDIR := $(realpath $(call my-dir)/../../../../../../..)

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/DeferredShadingPFX/Vulkan/Build/Android/assets


ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVREngineUtils
include $(CLEAR_VARS)
LOCAL_MODULE := PVREngineUtils
LOCAL_SRC_FILES := $(PVRSDKDIR)/Framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVREngineUtils.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRShell
include $(CLEAR_VARS)
LOCAL_MODULE := PVRShell
LOCAL_SRC_FILES := $(PVRSDKDIR)/Framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRShell.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRVulkan
include $(CLEAR_VARS)
LOCAL_MODULE := PVRVulkan
LOCAL_SRC_FILES := $(PVRSDKDIR)/Framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRVulkan.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRNativeVulkan
include $(CLEAR_VARS)
LOCAL_MODULE := PVRNativeVulkan
LOCAL_SRC_FILES := $(PVRSDKDIR)/Framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRNativeVulkan.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRAssets
include $(CLEAR_VARS)
LOCAL_MODULE := PVRAssets
LOCAL_SRC_FILES := $(PVRSDKDIR)/Framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRAssets.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRCore
include $(CLEAR_VARS)
LOCAL_MODULE := PVRCore
LOCAL_SRC_FILES := $(PVRSDKDIR)/Framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRCore.a
include $(PREBUILT_STATIC_LIBRARY)
endif


# Module VulkanDeferredShadingPFX
include $(CLEAR_VARS)

LOCAL_MODULE    := VulkanDeferredShadingPFX

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := VulkanDeferredShadingPFX.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Framework \
                    $(PVRSDKDIR)/Builds/Include



LOCAL_LDLIBS := -landroid \
                -llog \
                -latomic

LOCAL_STATIC_LIBRARIES := PVREngineUtils PVRVulkan PVRNativeVulkan PVRAssets PVRCore android_native_app_glue


LOCAL_CFLAGS += $(SDK_BUILD_FLAGS)

LOCAL_WHOLE_STATIC_LIBRARIES := PVRShell

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

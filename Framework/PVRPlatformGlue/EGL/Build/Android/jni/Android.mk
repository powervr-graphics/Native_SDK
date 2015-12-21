LOCAL_PATH := $(realpath $(call my-dir)/../../../..)
PVRFRAMEWORKDIR := $(realpath $(call my-dir)/../../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE      := PVREgl
LOCAL_SRC_FILES := 	$(notdir $(wildcard $(LOCAL_PATH)/*.cpp)) \
                    $(addprefix EGL/, $(notdir $(wildcard $(LOCAL_PATH)/EGL/*.cpp)))

LOCAL_CFLAGS := $(SDK_BUILD_FLAGS)
LOCAL_C_INCLUDES := $(PVRFRAMEWORKDIR) $(PVRFRAMEWORKDIR)/../Builds/Include $(PVRFRAMEWORKDIR)/PVRPlatformGlue/EGL

LOCAL_STATIC_LIBRARIES += PVRCore

include $(BUILD_STATIC_LIBRARY)

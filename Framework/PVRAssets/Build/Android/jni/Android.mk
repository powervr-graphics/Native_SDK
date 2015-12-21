LOCAL_PATH := $(realpath $(call my-dir)/../../..)
PVRFRAMEWORKDIR := $(realpath $(call my-dir)/../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE      := PVRAssets
LOCAL_SRC_FILES := 	$(notdir $(wildcard $(LOCAL_PATH)/*.cpp)) \
                    $(addprefix FileIO/, $(notdir $(wildcard $(LOCAL_PATH)/FileIO/*.cpp)))\
                    $(addprefix Model/, $(notdir $(wildcard $(LOCAL_PATH)/Model/*.cpp)))\
                    $(addprefix Texture/, $(notdir $(wildcard $(LOCAL_PATH)/Texture/*.cpp)))

LOCAL_CFLAGS := $(SDK_BUILD_FLAGS)

LOCAL_C_INCLUDES := $(PVRFRAMEWORKDIR) \
                    $(PVRFRAMEWORKDIR)/../Builds/Include/
					
LOCAL_STATIC_LIBRARIES += PVRCore

include $(BUILD_STATIC_LIBRARY)

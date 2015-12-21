LOCAL_PATH := $(realpath $(call my-dir)/../../..)
PVRSDKDIR  := $(realpath $(call my-dir)/../../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE     := PVRUIRenderer
LOCAL_SRC_FILES  := $(notdir $(wildcard $(LOCAL_PATH)/*.cpp))

LOCAL_CFLAGS     := $(SDK_BUILD_FLAGS)

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Framework/ \
                    $(PVRSDKDIR)/Builds/Include/

LOCAL_STATIC_LIBRARIES += PVRGles PVRCore

include $(BUILD_STATIC_LIBRARY)
$(call import-module,android/native_app_glue)

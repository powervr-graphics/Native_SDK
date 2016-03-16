LOCAL_PATH := $(realpath $(call my-dir)/../../../..)
PVRSDKDIR  := $(realpath $(call my-dir)/../../../../../..)

$(info $(LOCAL_PATH))
$(info $(PVRSDKDIR))

include $(CLEAR_VARS)
   
LOCAL_MODULE     := PVRNativeGles
LOCAL_SRC_FILES  := $(notdir $(wildcard $(LOCAL_PATH)/*.cpp))\
                    $(addprefix OGLES/, $(notdir $(wildcard $(LOCAL_PATH)/OGLES/*.cpp)))

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Framework/ \
                    $(PVRSDKDIR)/Builds/Include/
$(info $(LOCAL_C_INCLUDES))
LOCAL_CFLAGS := $(SDK_BUILD_FLAGS)

LOCAL_STATIC_LIBRARIES += PVREgl PVRAssets PVRCore

include $(BUILD_STATIC_LIBRARY)
$(call import-module,android/native_app_glue) 
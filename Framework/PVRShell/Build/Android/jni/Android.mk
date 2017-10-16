LOCAL_PATH := $(realpath $(call my-dir)/../../..)
PVRSDKDIR := $(realpath $(call my-dir)/../../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE      := PVRShell
LOCAL_SRC_FILES := 	$(notdir $(wildcard $(LOCAL_PATH)/*.cpp)) \
                    $(addprefix OS/Android/, $(notdir $(wildcard $(LOCAL_PATH)/OS/Android/*.cpp))) \
                    $(addprefix EntryPoint/android_main/, $(notdir $(wildcard $(LOCAL_PATH)/EntryPoint/android_main/*.cpp)))

LOCAL_CFLAGS := $(SDK_BUILD_FLAGS)
LOCAL_C_INCLUDES := $(PVRSDKDIR)/Framework/ $(PVRSDKDIR)/Builds/Include/ $(PVRSDKDIR)/Framework/PVRShell/OS/Android/

LOCAL_STATIC_LIBRARIES := android_native_app_glue
                    
include $(BUILD_STATIC_LIBRARY)
$(call import-module,android/native_app_glue) 

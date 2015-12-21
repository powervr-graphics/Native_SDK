LOCAL_PATH := $(realpath $(call my-dir)/../../..)
PVRFRAMEWORKDIR := $(realpath $(call my-dir)/../../../..)

include $(CLEAR_VARS)
   
LOCAL_MODULE      := PVRShell
LOCAL_SRC_FILES := 	$(notdir $(wildcard $(LOCAL_PATH)/*.cpp)) \
                    $(addprefix OS/Android/, $(notdir $(wildcard $(LOCAL_PATH)/OS/Android/*.cpp))) \
                    $(addprefix EntryPoint/android_main/, $(notdir $(wildcard $(LOCAL_PATH)/EntryPoint/android_main/*.cpp)))

LOCAL_CFLAGS := $(SDK_BUILD_FLAGS)
LOCAL_C_INCLUDES := $(PVRFRAMEWORKDIR) \
					$(PVRFRAMEWORKDIR)/../Builds/Include \
                    $(PVRFRAMEWORKDIR)/PVRShell/OS/Android/
                    
LOCAL_STATIC_LIBRARIES += android_native_app_glue PVRCore PVREgl

include $(BUILD_STATIC_LIBRARY)
$(call import-module,android/native_app_glue) 
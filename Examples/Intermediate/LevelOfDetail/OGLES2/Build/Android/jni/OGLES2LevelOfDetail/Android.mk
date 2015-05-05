LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Intermediate/LevelOfDetail/OGLES2/Build/Android/assets


ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module ogles2tools
include $(CLEAR_VARS)
LOCAL_MODULE := ogles2tools
LOCAL_SRC_FILES := $(PVRSDKDIR)/Tools/OGLES2/Build/Android/obj/local/$(TARGET_ARCH_ABI)/libogles2tools.a
include $(PREBUILT_STATIC_LIBRARY)
endif


# Module OGLES2LevelOfDetail
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES2LevelOfDetail

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Intermediate/LevelOfDetail/OGLES2/OGLES2LevelOfDetail.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Shell \
                    $(PVRSDKDIR)/Shell/API/KEGL \
                    $(PVRSDKDIR)/Shell/OS/Android \
                    $(PVRSDKDIR)/Builds/Include \
                    $(PVRSDKDIR)/Tools \
                    $(PVRSDKDIR)/Tools/OGLES2

LOCAL_CFLAGS := -DBUILD_OGLES2

LOCAL_LDLIBS := -llog \
                -landroid \
                -lEGL \
                -lGLESv2

LOCAL_STATIC_LIBRARIES := android_native_app_glue \
                          ogles2tools

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Beginner/06_IntroducingPrint3D/OGLES3/Build/Android/assets


ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module ogles3tools
include $(CLEAR_VARS)
LOCAL_MODULE := ogles3tools
LOCAL_SRC_FILES := $(PVRSDKDIR)/Tools/OGLES3/Build/Android/obj/local/$(TARGET_ARCH_ABI)/libogles3tools.a
include $(PREBUILT_STATIC_LIBRARY)
endif


# Module OGLES3IntroducingPrint3D
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3IntroducingPrint3D

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Beginner/06_IntroducingPrint3D/OGLES3/OGLES3IntroducingPrint3D.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Shell \
                    $(PVRSDKDIR)/Shell/API/KEGL \
                    $(PVRSDKDIR)/Shell/OS/Android \
                    $(PVRSDKDIR)/Builds/Include \
                    $(PVRSDKDIR)/Tools \
                    $(PVRSDKDIR)/Tools/OGLES2 \
                    $(PVRSDKDIR)/Tools/OGLES3

LOCAL_CFLAGS := -DBUILD_OGLES3

LOCAL_LDLIBS := -llog \
                -landroid \
                -lEGL \
                -lGLESv3

LOCAL_STATIC_LIBRARIES := android_native_app_glue \
                          ogles3tools

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

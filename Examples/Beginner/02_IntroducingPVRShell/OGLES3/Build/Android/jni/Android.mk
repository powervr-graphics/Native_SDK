LOCAL_PATH := $(call my-dir)/../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

# Module OGLES3IntroducingPVRShell
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3IntroducingPVRShell

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Beginner/02_IntroducingPVRShell/OGLES3/OGLES3IntroducingPVRShell.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Shell \
                    $(PVRSDKDIR)/Shell/API/KEGL \
                    $(PVRSDKDIR)/Shell/OS/Android \
                    $(PVRSDKDIR)/Builds/Include

LOCAL_CFLAGS := -DBUILD_OGLES3

LOCAL_LDLIBS := -llog \
                -landroid \
                -lEGL \
                -lGLESv3

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)



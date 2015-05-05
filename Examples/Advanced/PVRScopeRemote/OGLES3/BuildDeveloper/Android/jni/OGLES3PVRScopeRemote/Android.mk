LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/PVRScopeRemote/OGLES3/BuildDeveloper/Android/assets


ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRScopeDeveloper
include $(CLEAR_VARS)
LOCAL_MODULE := PVRScopeDeveloper
LOCAL_SRC_FILES := $(PVRSDKDIR)/Builds/Android/$(TARGET_ARCH_ABI)/Lib/libPVRScopeDeveloper.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module ogles3tools
include $(CLEAR_VARS)
LOCAL_MODULE := ogles3tools
LOCAL_SRC_FILES := $(PVRSDKDIR)/Tools/OGLES3/Build/Android/obj/local/$(TARGET_ARCH_ABI)/libogles3tools.a
include $(PREBUILT_STATIC_LIBRARY)
endif


# Module OGLES3PVRScopeRemote
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3PVRScopeRemote

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/PVRScopeRemote/OGLES3/OGLES3PVRScopeRemote.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Builds/Include \
                    $(PVRSDKDIR)/Shell \
                    $(PVRSDKDIR)/Shell/API/KEGL \
                    $(PVRSDKDIR)/Shell/OS/Android \
                    $(PVRSDKDIR)/Tools \
                    $(PVRSDKDIR)/Tools/OGLES2 \
                    $(PVRSDKDIR)/Tools/OGLES3

LOCAL_CFLAGS := -DBUILD_OGLES3

LOCAL_LDLIBS := -ldl \
                -llog \
                -landroid \
                -lEGL \
                -lGLESv3

LOCAL_STATIC_LIBRARIES := PVRScopeDeveloper \
                          android_native_app_glue \
                          ogles3tools

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

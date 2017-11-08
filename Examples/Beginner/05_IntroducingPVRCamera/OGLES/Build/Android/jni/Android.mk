LOCAL_PATH := $(call my-dir)/../../..
PVRSDKDIR := $(realpath $(call my-dir)/../../../../../../..)

ASSETDIR := $(PVRSDKDIR)/Examples/Beginner/05_IntroducingPVRCamera/OGLES/Build/Android/assets
LIBDIR := $(PVRSDKDIR)/Examples/Beginner/05_IntroducingPVRCamera/OGLES/Build/Android/libs

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRUtilsGles
include $(CLEAR_VARS)
LOCAL_MODULE := PVRUtilsGles
LOCAL_SRC_FILES := $(PVRSDKDIR)/Framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRUtilsGles.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRShell
include $(CLEAR_VARS)
LOCAL_MODULE := PVRShell
LOCAL_SRC_FILES := $(PVRSDKDIR)/Framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRShell.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRAssets
include $(CLEAR_VARS)
LOCAL_MODULE := PVRAssets
LOCAL_SRC_FILES := $(PVRSDKDIR)/Framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRAssets.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRCore
include $(CLEAR_VARS)
LOCAL_MODULE := PVRCore
LOCAL_SRC_FILES := $(PVRSDKDIR)/Framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRCore.a
include $(PREBUILT_STATIC_LIBRARY)
endif

ifneq "$(MAKECMDGOALS)" "clean"
# Prebuilt module PVRCamera
include $(CLEAR_VARS)
LOCAL_MODULE := PVRCamera
LOCAL_SRC_FILES := $(PVRSDKDIR)/Framework/Bin/Android/local/$(TARGET_ARCH_ABI)/libPVRCamera.a
include $(PREBUILT_STATIC_LIBRARY)
endif


# Module OGLESIntroducingPVRCamera
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLESIntroducingPVRCamera

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := OGLESIntroducingPVRCamera.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Framework \
                    $(PVRSDKDIR)/Builds/Include



LOCAL_LDLIBS := -llog \
                -latomic \
                -landroid

LOCAL_STATIC_LIBRARIES := PVRUtilsGles PVRAssets PVRCore android_native_app_glue


LOCAL_CFLAGS += $(SDK_BUILD_FLAGS)

LOCAL_WHOLE_STATIC_LIBRARIES := PVRShell PVRCamera

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/PVRScopeExample/OGLES/BuildDeveloper/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module PVRScopeDeveloper
include $(CLEAR_VARS)
LOCAL_MODULE := PVRScopeDeveloper
LOCAL_SRC_FILES := $(PVRSDKDIR)/Builds/Android/$(TARGET_ARCH_ABI)/Lib/libPVRScopeDeveloper.a
include $(PREBUILT_STATIC_LIBRARY)

# Module OGLESPVRScopeExample
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLESPVRScopeExample

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/PVRScopeExample/OGLES/OGLESPVRScopeExample.cpp \
                    Examples/Advanced/PVRScopeExample/OGLES/PVRScopeGraphAPI.cpp \
                    Examples/Advanced/PVRScopeExample/PVRScopeGraph.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Examples/Advanced/PVRScopeExample \
                    $(PVRSDKDIR)/Builds/Include \
                    $(PVRSDKDIR)/Shell \
                    $(PVRSDKDIR)/Shell/API/KEGL \
                    $(PVRSDKDIR)/Shell/OS/Android \
                    $(PVRSDKDIR)/Tools \
                    $(PVRSDKDIR)/Tools/OGLES

LOCAL_CFLAGS := -DBUILD_OGLES

LOCAL_LDLIBS := -ldl \
                -llog \
                -landroid \
                -lEGL \
                -lGLESv1_CM

LOCAL_STATIC_LIBRARIES := PVRScopeDeveloper \
                          android_native_app_glue \
                          oglestools

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

### Copy our external files to the assets folder, but only do it for the first abi
ifeq ($(TARGET_ARCH_ABI),$(firstword $(NDK_APP_ABI)))

all:  \
	$(ASSETDIR)/Mask.pod \
	$(ASSETDIR)/MaskTex.pvr

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/MaskTex.pvr: $(PVRSDKDIR)/Examples/Advanced/PVRScopeExample/OGLES/MaskTex.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Mask.pod: $(PVRSDKDIR)/Examples/Advanced/PVRScopeExample/OGLES/Mask.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


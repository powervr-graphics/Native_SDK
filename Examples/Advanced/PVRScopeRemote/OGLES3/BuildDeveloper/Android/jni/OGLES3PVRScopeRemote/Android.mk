LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/PVRScopeRemote/OGLES3/BuildDeveloper/Android/assets

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

### Copy our external files to the assets folder, but only do it for the first abi
ifeq ($(TARGET_ARCH_ABI),$(firstword $(NDK_APP_ABI)))

all:  \
	$(ASSETDIR)/Mask.pod \
	$(ASSETDIR)/Reflection.pvr \
	$(ASSETDIR)/Thickness.pvr \
	$(ASSETDIR)/FragShader.fsh \
	$(ASSETDIR)/VertShader.vsh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Reflection.pvr: $(PVRSDKDIR)/Examples/Advanced/PVRScopeRemote/OGLES3/Reflection.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Thickness.pvr: $(PVRSDKDIR)/Examples/Advanced/PVRScopeRemote/OGLES3/Thickness.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/PVRScopeRemote/OGLES3/FragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/VertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/PVRScopeRemote/OGLES3/VertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Mask.pod: $(PVRSDKDIR)/Examples/Advanced/PVRScopeRemote/OGLES3/Mask.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif



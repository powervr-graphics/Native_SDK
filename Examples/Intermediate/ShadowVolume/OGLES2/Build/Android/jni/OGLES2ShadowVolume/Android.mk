LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Intermediate/ShadowVolume/OGLES2/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES2ShadowVolume
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES2ShadowVolume

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Intermediate/ShadowVolume/OGLES2/OGLES2ShadowVolume.cpp \
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

### Copy our external files to the assets folder, but only do it for the first abi
ifeq ($(TARGET_ARCH_ABI),$(firstword $(NDK_APP_ABI)))

all:  \
	$(ASSETDIR)/scene.pod \
	$(ASSETDIR)/Background.pvr \
	$(ASSETDIR)/Rust.pvr \
	$(ASSETDIR)/BaseFragShader.fsh \
	$(ASSETDIR)/BaseVertShader.vsh \
	$(ASSETDIR)/ConstFragShader.fsh \
	$(ASSETDIR)/ShadowVolVertShader.vsh \
	$(ASSETDIR)/FullscreenVertShader.vsh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Background.pvr: $(PVRSDKDIR)/Examples/Intermediate/ShadowVolume/OGLES2/Background.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Rust.pvr: $(PVRSDKDIR)/Examples/Intermediate/ShadowVolume/OGLES2/Rust.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/BaseFragShader.fsh: $(PVRSDKDIR)/Examples/Intermediate/ShadowVolume/OGLES2/BaseFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/BaseVertShader.vsh: $(PVRSDKDIR)/Examples/Intermediate/ShadowVolume/OGLES2/BaseVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ConstFragShader.fsh: $(PVRSDKDIR)/Examples/Intermediate/ShadowVolume/OGLES2/ConstFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ShadowVolVertShader.vsh: $(PVRSDKDIR)/Examples/Intermediate/ShadowVolume/OGLES2/ShadowVolVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FullscreenVertShader.vsh: $(PVRSDKDIR)/Examples/Intermediate/ShadowVolume/OGLES2/FullscreenVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/scene.pod: $(PVRSDKDIR)/Examples/Intermediate/ShadowVolume/OGLES2/scene.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


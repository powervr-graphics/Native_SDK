LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/Skybox2/OGLES2/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES2Skybox2
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES2Skybox2

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/Skybox2/OGLES2/OGLES2Skybox2.cpp \
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
	$(ASSETDIR)/Scene.pod \
	$(ASSETDIR)/effects.pfx \
	$(ASSETDIR)/Balloon.pvr \
	$(ASSETDIR)/Balloon_pvr.pvr \
	$(ASSETDIR)/Noise.pvr \
	$(ASSETDIR)/Skybox.pvr \
	$(ASSETDIR)/SkyboxMidnight.pvr

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Balloon.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox2/OGLES2/Balloon.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Balloon_pvr.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox2/OGLES2/Balloon_pvr.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Noise.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox2/OGLES2/Noise.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Skybox.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox2/OGLES2/Skybox.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkyboxMidnight.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox2/OGLES2/SkyboxMidnight.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Scene.pod: $(PVRSDKDIR)/Examples/Advanced/Skybox2/OGLES2/Scene.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/effects.pfx: $(PVRSDKDIR)/Examples/Advanced/Skybox2/OGLES2/effects.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


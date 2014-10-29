LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/MagicLantern/OGLES2/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES2MagicLantern
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES2MagicLantern

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/MagicLantern/OGLES2/OGLES2MagicLantern.cpp \
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
	$(ASSETDIR)/MagicLantern.pod \
	$(ASSETDIR)/MagicLanternShaders.pfx \
	$(ASSETDIR)/Marble.pvr \
	$(ASSETDIR)/Floor.pvr \
	$(ASSETDIR)/LanternCubemap.pvr

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Marble.pvr: $(PVRSDKDIR)/Examples/Advanced/MagicLantern/OGLES2/Marble.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Floor.pvr: $(PVRSDKDIR)/Examples/Advanced/MagicLantern/OGLES2/Floor.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/LanternCubemap.pvr: $(PVRSDKDIR)/Examples/Advanced/MagicLantern/OGLES2/LanternCubemap.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/MagicLantern.pod: $(PVRSDKDIR)/Examples/Advanced/MagicLantern/OGLES2/MagicLantern.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/MagicLanternShaders.pfx: $(PVRSDKDIR)/Examples/Advanced/MagicLantern/OGLES2/MagicLanternShaders.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


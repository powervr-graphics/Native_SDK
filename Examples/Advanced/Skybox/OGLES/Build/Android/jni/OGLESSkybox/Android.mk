LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/Skybox/OGLES/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLESSkybox
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLESSkybox

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/Skybox/OGLES/OGLESSkybox.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Shell \
                    $(PVRSDKDIR)/Shell/API/KEGL \
                    $(PVRSDKDIR)/Shell/OS/Android \
                    $(PVRSDKDIR)/Builds/Include \
                    $(PVRSDKDIR)/Tools \
                    $(PVRSDKDIR)/Tools/OGLES

LOCAL_CFLAGS := -DBUILD_OGLES

LOCAL_LDLIBS := -llog \
                -landroid \
                -lEGL \
                -lGLESv1_CM

LOCAL_STATIC_LIBRARIES := android_native_app_glue \
                          oglestools

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

### Copy our external files to the assets folder, but only do it for the first abi
ifeq ($(TARGET_ARCH_ABI),$(firstword $(NDK_APP_ABI)))

all:  \
	$(ASSETDIR)/HotAirBalloon.pod \
	$(ASSETDIR)/balloon.pvr \
	$(ASSETDIR)/skybox1.pvr \
	$(ASSETDIR)/skybox2.pvr \
	$(ASSETDIR)/skybox3.pvr \
	$(ASSETDIR)/skybox4.pvr \
	$(ASSETDIR)/skybox5.pvr \
	$(ASSETDIR)/skybox6.pvr

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/balloon.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox/OGLES/balloon.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/skybox1.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox/OGLES/skybox1.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/skybox2.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox/OGLES/skybox2.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/skybox3.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox/OGLES/skybox3.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/skybox4.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox/OGLES/skybox4.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/skybox5.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox/OGLES/skybox5.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/skybox6.pvr: $(PVRSDKDIR)/Examples/Advanced/Skybox/OGLES/skybox6.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/HotAirBalloon.pod: $(PVRSDKDIR)/Examples/Advanced/Skybox/OGLES/HotAirBalloon.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


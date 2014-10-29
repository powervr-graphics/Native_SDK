LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES2FilmTV
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES2FilmTV

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/FilmTV/OGLES2/OGLES2FilmTV.cpp \
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
	$(ASSETDIR)/FilmTVScene.pod \
	$(ASSETDIR)/Table.pvr \
	$(ASSETDIR)/Floor.pvr \
	$(ASSETDIR)/Wall.pvr \
	$(ASSETDIR)/TV.pvr \
	$(ASSETDIR)/TVCase.pvr \
	$(ASSETDIR)/TVSpeaker.pvr \
	$(ASSETDIR)/Alum.pvr \
	$(ASSETDIR)/Skirting.pvr \
	$(ASSETDIR)/Camera.pvr \
	$(ASSETDIR)/FragShader.fsh \
	$(ASSETDIR)/BWFragShader.fsh \
	$(ASSETDIR)/VertShader.vsh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Table.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/Table.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Floor.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/Floor.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Wall.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/Wall.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/TV.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/TV.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/TVCase.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/TVCase.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/TVSpeaker.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/TVSpeaker.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Alum.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/Alum.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Skirting.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/Skirting.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Camera.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/Camera.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/FragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/BWFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/BWFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/VertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/VertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FilmTVScene.pod: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES2/FilmTVScene.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


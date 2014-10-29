LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLESFilmTV
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLESFilmTV

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/FilmTV/OGLES/OGLESFilmTV.cpp \
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
	$(ASSETDIR)/FilmTVScene.pod \
	$(ASSETDIR)/Table.pvr \
	$(ASSETDIR)/Floor.pvr \
	$(ASSETDIR)/Wall.pvr \
	$(ASSETDIR)/TV.pvr \
	$(ASSETDIR)/TVCase.pvr \
	$(ASSETDIR)/TVSpeaker.pvr \
	$(ASSETDIR)/Alum.pvr \
	$(ASSETDIR)/Skirting.pvr \
	$(ASSETDIR)/Camera.pvr

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Table.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES/Table.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Floor.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES/Floor.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Wall.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES/Wall.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/TV.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES/TV.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/TVCase.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES/TVCase.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/TVSpeaker.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES/TVSpeaker.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Alum.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES/Alum.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Skirting.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES/Skirting.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Camera.pvr: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES/Camera.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FilmTVScene.pod: $(PVRSDKDIR)/Examples/Advanced/FilmTV/OGLES/FilmTVScene.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


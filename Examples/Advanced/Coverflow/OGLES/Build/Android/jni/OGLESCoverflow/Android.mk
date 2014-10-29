LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLESCoverflow
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLESCoverflow

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/Coverflow/OGLES/OGLESCoverflow.cpp \
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
	$(ASSETDIR)/Album1.pvr \
	$(ASSETDIR)/Album2.pvr \
	$(ASSETDIR)/Album3.pvr \
	$(ASSETDIR)/Album4.pvr \
	$(ASSETDIR)/Album5.pvr \
	$(ASSETDIR)/Album6.pvr \
	$(ASSETDIR)/Album7.pvr \
	$(ASSETDIR)/Album8.pvr \
	$(ASSETDIR)/Album9.pvr \
	$(ASSETDIR)/Album10.pvr \
	$(ASSETDIR)/Album11.pvr \
	$(ASSETDIR)/Album12.pvr \
	$(ASSETDIR)/Album13.pvr \
	$(ASSETDIR)/Album14.pvr \
	$(ASSETDIR)/Album15.pvr \
	$(ASSETDIR)/Album16.pvr

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Album1.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album1.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album2.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album2.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album3.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album3.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album4.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album4.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album5.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album5.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album6.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album6.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album7.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album7.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album8.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album8.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album9.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album9.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album10.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album10.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album11.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album11.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album12.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album12.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album13.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album13.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album14.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album14.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album15.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album15.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Album16.pvr: $(PVRSDKDIR)/Examples/Advanced/Coverflow/OGLES/Album16.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


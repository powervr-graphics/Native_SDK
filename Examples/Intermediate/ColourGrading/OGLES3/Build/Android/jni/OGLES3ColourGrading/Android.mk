LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES3ColourGrading
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3ColourGrading

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Intermediate/ColourGrading/OGLES3/OGLES3ColourGrading.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Shell \
                    $(PVRSDKDIR)/Shell/API/KEGL \
                    $(PVRSDKDIR)/Shell/OS/Android \
                    $(PVRSDKDIR)/Builds/Include \
                    $(PVRSDKDIR)/Tools \
                    $(PVRSDKDIR)/Tools/OGLES2 \
                    $(PVRSDKDIR)/Tools/OGLES3

LOCAL_CFLAGS := -DBUILD_OGLES3

LOCAL_LDLIBS := -llog \
                -landroid \
                -lEGL \
                -lGLESv3

LOCAL_STATIC_LIBRARIES := android_native_app_glue \
                          ogles3tools

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

### Copy our external files to the assets folder, but only do it for the first abi
ifeq ($(TARGET_ARCH_ABI),$(firstword $(NDK_APP_ABI)))

all:  \
	$(ASSETDIR)/Mask.pod \
	$(ASSETDIR)/identity.pvr \
	$(ASSETDIR)/cooler.pvr \
	$(ASSETDIR)/warmer.pvr \
	$(ASSETDIR)/bw.pvr \
	$(ASSETDIR)/sepia.pvr \
	$(ASSETDIR)/inverted.pvr \
	$(ASSETDIR)/highcontrast.pvr \
	$(ASSETDIR)/bluewhitegradient.pvr \
	$(ASSETDIR)/MaskTexture.pvr \
	$(ASSETDIR)/Background.pvr \
	$(ASSETDIR)/FragShader.fsh \
	$(ASSETDIR)/VertShader.vsh \
	$(ASSETDIR)/SceneFragShader.fsh \
	$(ASSETDIR)/SceneVertShader.vsh \
	$(ASSETDIR)/BackgroundFragShader.fsh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/MaskTexture.pvr: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/MaskTexture.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Background.pvr: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/Background.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FragShader.fsh: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/FragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/VertShader.vsh: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/VertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SceneFragShader.fsh: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/SceneFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SceneVertShader.vsh: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/SceneVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/BackgroundFragShader.fsh: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/BackgroundFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Mask.pod: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/Mask.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/identity.pvr: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/identity.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/cooler.pvr: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/cooler.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/warmer.pvr: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/warmer.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/bw.pvr: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/bw.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/sepia.pvr: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/sepia.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/inverted.pvr: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/inverted.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/highcontrast.pvr: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/highcontrast.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/bluewhitegradient.pvr: $(PVRSDKDIR)/Examples/Intermediate/ColourGrading/OGLES3/bluewhitegradient.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif



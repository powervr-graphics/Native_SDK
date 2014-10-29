LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Intermediate/ImageConvolution/OGLES3/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES3ImageConvolution
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3ImageConvolution

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Intermediate/ImageConvolution/OGLES3/OGLES3ImageConvolution.cpp \
                    Examples/Intermediate/ImageConvolution/OGLES3/CSImageConvolution.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Examples/Intermediate/ImageConvolution/OGLES3 \
                    $(PVRSDKDIR)/Shell \
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
	$(ASSETDIR)/Image.pvr \
	$(ASSETDIR)/FragShader.fsh \
	$(ASSETDIR)/VertShader.vsh \
	$(ASSETDIR)/ComputeShader.csh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Image.pvr: $(PVRSDKDIR)/Examples/Intermediate/ImageConvolution/OGLES3/Image.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FragShader.fsh: $(PVRSDKDIR)/Examples/Intermediate/ImageConvolution/OGLES3/FragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/VertShader.vsh: $(PVRSDKDIR)/Examples/Intermediate/ImageConvolution/OGLES3/VertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ComputeShader.csh: $(PVRSDKDIR)/Examples/Intermediate/ImageConvolution/OGLES3/ComputeShader.csh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif



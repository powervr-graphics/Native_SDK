LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES2Bloom
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES2Bloom

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Intermediate/Bloom/OGLES2/OGLES2Bloom.cpp \
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
	$(ASSETDIR)/Mask.pod \
	$(ASSETDIR)/BaseTex.pvr \
	$(ASSETDIR)/bloom_mapping.pvr \
	$(ASSETDIR)/PostBloomFragShader.fsh \
	$(ASSETDIR)/PostBloomVertShader.vsh \
	$(ASSETDIR)/PreBloomFragShader.fsh \
	$(ASSETDIR)/PreBloomVertShader.vsh \
	$(ASSETDIR)/BlurFragShader.fsh \
	$(ASSETDIR)/BlurVertShader.vsh \
	$(ASSETDIR)/FragShader.fsh \
	$(ASSETDIR)/VertShader.vsh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/BaseTex.pvr: $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/BaseTex.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/bloom_mapping.pvr: $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/bloom_mapping.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/PostBloomFragShader.fsh: $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/PostBloomFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/PostBloomVertShader.vsh: $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/PostBloomVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/PreBloomFragShader.fsh: $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/PreBloomFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/PreBloomVertShader.vsh: $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/PreBloomVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/BlurFragShader.fsh: $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/BlurFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/BlurVertShader.vsh: $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/BlurVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FragShader.fsh: $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/FragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/VertShader.vsh: $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/VertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Mask.pod: $(PVRSDKDIR)/Examples/Intermediate/Bloom/OGLES2/Mask.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


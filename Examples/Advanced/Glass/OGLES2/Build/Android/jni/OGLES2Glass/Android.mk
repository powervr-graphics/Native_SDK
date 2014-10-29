LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES2Glass
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES2Glass

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/Glass/OGLES2/OGLES2Glass.cpp \
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
	$(ASSETDIR)/Balloon.pod \
	$(ASSETDIR)/Ball.pod \
	$(ASSETDIR)/BalloonTex.pvr \
	$(ASSETDIR)/SkyboxTex.pvr \
	$(ASSETDIR)/DefaultVertShader.vsh \
	$(ASSETDIR)/DefaultFragShader.fsh \
	$(ASSETDIR)/ParaboloidVertShader.vsh \
	$(ASSETDIR)/SkyboxVertShader.vsh \
	$(ASSETDIR)/SkyboxFragShader.fsh \
	$(ASSETDIR)/EffectVertShader.vsh \
	$(ASSETDIR)/EffectFragShader.fsh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/BalloonTex.pvr: $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/BalloonTex.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkyboxTex.pvr: $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/SkyboxTex.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/DefaultVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/DefaultVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/DefaultFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/DefaultFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ParaboloidVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/ParaboloidVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkyboxVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/SkyboxVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkyboxFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/SkyboxFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/EffectVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/EffectVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/EffectFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/EffectFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Balloon.pod: $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/Balloon.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Ball.pod: $(PVRSDKDIR)/Examples/Advanced/Glass/OGLES2/Ball.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


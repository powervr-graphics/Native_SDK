LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/ParticleSystem/OGLES3/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES3ParticleSystem
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3ParticleSystem

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/ParticleSystem/OGLES3/OGLES3ParticleSystem.cpp \
                    Examples/Advanced/ParticleSystem/OGLES3/ParticleSystemGPU.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Examples/Advanced/ParticleSystem/OGLES3 \
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
	$(ASSETDIR)/sphere.pod \
	$(ASSETDIR)/ParticleGradient.pvr \
	$(ASSETDIR)/FragShader.fsh \
	$(ASSETDIR)/VertShader.vsh \
	$(ASSETDIR)/FloorFragShader.fsh \
	$(ASSETDIR)/FloorVertShader.vsh \
	$(ASSETDIR)/ParticleFragShader.fsh \
	$(ASSETDIR)/ParticleVertShader.vsh \
	$(ASSETDIR)/ParticleSolver.csh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/ParticleGradient.pvr: $(PVRSDKDIR)/Examples/Advanced/ParticleSystem/OGLES3/ParticleGradient.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/ParticleSystem/OGLES3/FragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/VertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/ParticleSystem/OGLES3/VertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FloorFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/ParticleSystem/OGLES3/FloorFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FloorVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/ParticleSystem/OGLES3/FloorVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ParticleFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/ParticleSystem/OGLES3/ParticleFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ParticleVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/ParticleSystem/OGLES3/ParticleVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ParticleSolver.csh: $(PVRSDKDIR)/Examples/Advanced/ParticleSystem/OGLES3/ParticleSolver.csh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/sphere.pod: $(PVRSDKDIR)/Examples/Advanced/ParticleSystem/OGLES3/sphere.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif



LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES2Water
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES2Water

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/Water/OGLES2/OGLES2Water.cpp \
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
	$(ASSETDIR)/Scene.pod \
	$(ASSETDIR)/normalmap.pvr \
	$(ASSETDIR)/skybox.pvr \
	$(ASSETDIR)/galleon.pvr \
	$(ASSETDIR)/wood.pvr \
	$(ASSETDIR)/coins.pvr \
	$(ASSETDIR)/coins-specular.pvr \
	$(ASSETDIR)/flag.pvr \
	$(ASSETDIR)/crate.pvr \
	$(ASSETDIR)/galleon-sails.pvr \
	$(ASSETDIR)/sand.pvr \
	$(ASSETDIR)/palmleaf.pvr \
	$(ASSETDIR)/FragShader.fsh \
	$(ASSETDIR)/VertShader.vsh \
	$(ASSETDIR)/SkyboxFShader.fsh \
	$(ASSETDIR)/SkyboxVShader.vsh \
	$(ASSETDIR)/ModelFShader.fsh \
	$(ASSETDIR)/ModelVShader.vsh \
	$(ASSETDIR)/Tex2DFShader.fsh \
	$(ASSETDIR)/Tex2DVShader.vsh \
	$(ASSETDIR)/PlaneTexFShader.fsh \
	$(ASSETDIR)/PlaneTexVShader.vsh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/normalmap.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/normalmap.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/skybox.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/skybox.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/galleon.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/galleon.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/wood.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/wood.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/coins.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/coins.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/coins-specular.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/coins-specular.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/flag.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/flag.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/crate.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/crate.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/galleon-sails.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/galleon-sails.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/sand.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/sand.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/palmleaf.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/palmleaf.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/FragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/VertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/VertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkyboxFShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/SkyboxFShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkyboxVShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/SkyboxVShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ModelFShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/ModelFShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ModelVShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/ModelVShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Tex2DFShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/Tex2DFShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Tex2DVShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/Tex2DVShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/PlaneTexFShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/PlaneTexFShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/PlaneTexVShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/PlaneTexVShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Scene.pod: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES2/Scene.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


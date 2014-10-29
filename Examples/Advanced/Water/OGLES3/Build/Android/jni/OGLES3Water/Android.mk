LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES3Water
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3Water

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/Water/OGLES3/OGLES3Water.cpp \
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

$(ASSETDIR)/normalmap.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/normalmap.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/skybox.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/skybox.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/galleon.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/galleon.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/wood.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/wood.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/coins.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/coins.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/coins-specular.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/coins-specular.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/flag.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/flag.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/crate.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/crate.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/galleon-sails.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/galleon-sails.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/sand.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/sand.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/palmleaf.pvr: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/palmleaf.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/FragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/VertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/VertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkyboxFShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/SkyboxFShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkyboxVShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/SkyboxVShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ModelFShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/ModelFShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ModelVShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/ModelVShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Tex2DFShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/Tex2DFShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Tex2DVShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/Tex2DVShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/PlaneTexFShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/PlaneTexFShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/PlaneTexVShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/PlaneTexVShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Scene.pod: $(PVRSDKDIR)/Examples/Advanced/Water/OGLES3/Scene.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif



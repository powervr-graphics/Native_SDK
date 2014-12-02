LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES3ChameleonMan
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3ChameleonMan

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/ChameleonMan/OGLES3/OGLES3ChameleonMan.cpp \
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
	$(ASSETDIR)/ChameleonScene.pod \
	$(ASSETDIR)/skyline.pvr \
	$(ASSETDIR)/Wall_diffuse_baked.pvr \
	$(ASSETDIR)/Tang_space_BodyMap.pvr \
	$(ASSETDIR)/Tang_space_LegsMap.pvr \
	$(ASSETDIR)/Tang_space_BeltMap.pvr \
	$(ASSETDIR)/FinalChameleonManLegs.pvr \
	$(ASSETDIR)/FinalChameleonManHeadBody.pvr \
	$(ASSETDIR)/lamp.pvr \
	$(ASSETDIR)/ChameleonBelt.pvr \
	$(ASSETDIR)/SkinnedVertShader.vsh \
	$(ASSETDIR)/SkinnedFragShader.fsh \
	$(ASSETDIR)/DefaultVertShader.vsh \
	$(ASSETDIR)/DefaultFragShader.fsh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/skyline.pvr: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/skyline.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Wall_diffuse_baked.pvr: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/Wall_diffuse_baked.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Tang_space_BodyMap.pvr: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/Tang_space_BodyMap.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Tang_space_LegsMap.pvr: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/Tang_space_LegsMap.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Tang_space_BeltMap.pvr: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/Tang_space_BeltMap.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FinalChameleonManLegs.pvr: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/FinalChameleonManLegs.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FinalChameleonManHeadBody.pvr: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/FinalChameleonManHeadBody.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/lamp.pvr: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/lamp.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ChameleonBelt.pvr: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/ChameleonBelt.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkinnedVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/SkinnedVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkinnedFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/SkinnedFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/DefaultVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/DefaultVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/DefaultFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/DefaultFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ChameleonScene.pod: $(PVRSDKDIR)/Examples/Advanced/ChameleonMan/OGLES3/ChameleonScene.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif



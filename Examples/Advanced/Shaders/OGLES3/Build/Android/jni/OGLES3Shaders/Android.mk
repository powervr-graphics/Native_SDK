LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES3Shaders
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES3Shaders

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/Shaders/OGLES3/OGLES3Shaders.cpp \
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
	$(ASSETDIR)/anisotropic_lighting.pfx \
	$(ASSETDIR)/directional_lighting.pfx \
	$(ASSETDIR)/envmap.pfx \
	$(ASSETDIR)/fasttnl.pfx \
	$(ASSETDIR)/lattice.pfx \
	$(ASSETDIR)/phong_lighting.pfx \
	$(ASSETDIR)/point_lighting.pfx \
	$(ASSETDIR)/reflections.pfx \
	$(ASSETDIR)/simple.pfx \
	$(ASSETDIR)/spot_lighting.pfx \
	$(ASSETDIR)/toon.pfx \
	$(ASSETDIR)/vertex_sine.pfx \
	$(ASSETDIR)/wood.pfx \
	$(ASSETDIR)/Basetex.pvr \
	$(ASSETDIR)/Reflection.pvr \
	$(ASSETDIR)/Cubemap.pvr \
	$(ASSETDIR)/AnisoMap.pvr

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Basetex.pvr: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/Basetex.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Reflection.pvr: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/Reflection.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Cubemap.pvr: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/Cubemap.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/AnisoMap.pvr: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/AnisoMap.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/anisotropic_lighting.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/anisotropic_lighting.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/directional_lighting.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/directional_lighting.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/envmap.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/envmap.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/fasttnl.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/fasttnl.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/lattice.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/lattice.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/phong_lighting.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/phong_lighting.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/point_lighting.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/point_lighting.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/reflections.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/reflections.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/simple.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/simple.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/spot_lighting.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/spot_lighting.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/toon.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/toon.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/vertex_sine.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/vertex_sine.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/wood.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES3/wood.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif



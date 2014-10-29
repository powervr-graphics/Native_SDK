LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES2Shaders
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES2Shaders

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/Shaders/OGLES2/OGLES2Shaders.cpp \
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

$(ASSETDIR)/Basetex.pvr: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/Basetex.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Reflection.pvr: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/Reflection.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Cubemap.pvr: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/Cubemap.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/AnisoMap.pvr: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/AnisoMap.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/anisotropic_lighting.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/anisotropic_lighting.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/directional_lighting.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/directional_lighting.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/envmap.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/envmap.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/fasttnl.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/fasttnl.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/lattice.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/lattice.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/phong_lighting.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/phong_lighting.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/point_lighting.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/point_lighting.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/reflections.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/reflections.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/simple.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/simple.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/spot_lighting.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/spot_lighting.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/toon.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/toon.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/vertex_sine.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/vertex_sine.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/wood.pfx: $(PVRSDKDIR)/Examples/Advanced/Shaders/OGLES2/wood.pfx $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


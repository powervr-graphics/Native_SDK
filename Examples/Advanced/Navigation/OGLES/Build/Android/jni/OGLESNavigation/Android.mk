LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/Navigation/OGLES/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLESNavigation
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLESNavigation

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/Navigation/OGLES/OGLESNavigation.cpp \
                    Shell/PVRShell.cpp \
                    Shell/API/KEGL/PVRShellAPI.cpp \
                    Shell/OS/Android/PVRShellOS.cpp

LOCAL_C_INCLUDES := $(PVRSDKDIR)/Shell \
                    $(PVRSDKDIR)/Shell/API/KEGL \
                    $(PVRSDKDIR)/Shell/OS/Android \
                    $(PVRSDKDIR)/Builds/Include \
                    $(PVRSDKDIR)/Tools \
                    $(PVRSDKDIR)/Tools/OGLES

LOCAL_CFLAGS := -DBUILD_OGLES

LOCAL_LDLIBS := -llog \
                -landroid \
                -lEGL \
                -lGLESv1_CM

LOCAL_STATIC_LIBRARIES := android_native_app_glue \
                          oglestools

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

### Copy our external files to the assets folder, but only do it for the first abi
ifeq ($(TARGET_ARCH_ABI),$(firstword $(NDK_APP_ABI)))

all:  \
	$(ASSETDIR)/cameratrack.pod \
	$(ASSETDIR)/Landmark_meshes.nav \
	$(ASSETDIR)/LandUseA_meshes.nav \
	$(ASSETDIR)/LandUseB_meshes.nav \
	$(ASSETDIR)/MajHwys_meshes.nav \
	$(ASSETDIR)/MajHwyShield_text.nav \
	$(ASSETDIR)/RailRds_meshes.nav \
	$(ASSETDIR)/SecHwys_meshes.nav \
	$(ASSETDIR)/SecHwyShield_text.nav \
	$(ASSETDIR)/Signs_billboards.nav \
	$(ASSETDIR)/Streets_meshes.nav \
	$(ASSETDIR)/Streets_text.nav \
	$(ASSETDIR)/WaterPoly_meshes.nav \
	$(ASSETDIR)/WaterSeg_meshes.nav \
	$(ASSETDIR)/Alphabet.pvr \
	$(ASSETDIR)/Road.pvr

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Alphabet.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/Alphabet.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Road.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/Road.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/cameratrack.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/cameratrack.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Landmark_meshes.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/Landmark_meshes.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/LandUseA_meshes.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/LandUseA_meshes.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/LandUseB_meshes.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/LandUseB_meshes.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/MajHwys_meshes.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/MajHwys_meshes.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/MajHwyShield_text.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/MajHwyShield_text.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/RailRds_meshes.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/RailRds_meshes.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SecHwys_meshes.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/SecHwys_meshes.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SecHwyShield_text.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/SecHwyShield_text.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Signs_billboards.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/Signs_billboards.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Streets_meshes.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/Streets_meshes.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/Streets_text.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/Streets_text.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/WaterPoly_meshes.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/WaterPoly_meshes.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/WaterSeg_meshes.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation/Data/WaterSeg_meshes.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


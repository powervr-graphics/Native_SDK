LOCAL_PATH := $(call my-dir)/../../../../../../../..
PVRSDKDIR := $(realpath $(LOCAL_PATH))

ASSETDIR := $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/Build/Android/assets

CPY := cp
SEPARATOR := /
ifeq ($(HOST_OS),windows)
CPY := copy
SEPARATOR := \\
endif

# Module OGLES2Navigation3D
include $(CLEAR_VARS)

LOCAL_MODULE    := OGLES2Navigation3D

### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES  := Examples/Advanced/Navigation3D/OGLES2/OGLES2Navigation3D.cpp \
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
	$(ASSETDIR)/modelindex.nav \
	$(ASSETDIR)/occlusiondata.nav \
	$(ASSETDIR)/cameratrack.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_008_015_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_008_016_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_009_014_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_009_015_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_009_016_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_009_017_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_009_018_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_010_014_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_010_015_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_010_015_H.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_010_016_H.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_010_016_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_010_017_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_010_017_H.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_010_018_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_011_013_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_011_014_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_011_015_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_011_015_H.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_011_016_H.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_011_016_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_011_017_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_011_017_H.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_011_018_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_012_014_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_012_015_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_012_016_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_012_017_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_013_015_L.DAE.pod \
	$(ASSETDIR)/CM_US_IL_CHICAGO_013_016_L.DAE.pod \
	$(ASSETDIR)/Skybox.pvr \
	$(ASSETDIR)/UIElements.pvr \
	$(ASSETDIR)/006_RUS.PNG.pvr \
	$(ASSETDIR)/007_RUG.PNG.pvr \
	$(ASSETDIR)/008_RUG.PNG.pvr \
	$(ASSETDIR)/009_RUG.PNG.pvr \
	$(ASSETDIR)/011_GIE.PNG.pvr \
	$(ASSETDIR)/012_RSR.PNG.pvr \
	$(ASSETDIR)/016_FOC.PNG.pvr \
	$(ASSETDIR)/016_RTR.PNG.pvr \
	$(ASSETDIR)/017_FOD.PNG.pvr \
	$(ASSETDIR)/018_FOD.PNG.pvr \
	$(ASSETDIR)/019_FOC.PNG.pvr \
	$(ASSETDIR)/019_GOC.PNG.pvr \
	$(ASSETDIR)/019_RZG.PNG.pvr \
	$(ASSETDIR)/020_FOC.PNG.pvr \
	$(ASSETDIR)/021_FOC.PNG.pvr \
	$(ASSETDIR)/022_FOC.PNG.pvr \
	$(ASSETDIR)/022_RUG.PNG.pvr \
	$(ASSETDIR)/023_FOB.PNG.pvr \
	$(ASSETDIR)/023_RUG.PNG.pvr \
	$(ASSETDIR)/024_FOB.PNG.pvr \
	$(ASSETDIR)/025_FOC.PNG.pvr \
	$(ASSETDIR)/025_RUW.PNG.pvr \
	$(ASSETDIR)/026_FOD.PNG.pvr \
	$(ASSETDIR)/026_RUW.PNG.pvr \
	$(ASSETDIR)/027_FOD.PNG.pvr \
	$(ASSETDIR)/027_RUW.PNG.pvr \
	$(ASSETDIR)/028_GOF.PNG.pvr \
	$(ASSETDIR)/029_GCC.PNG.pvr \
	$(ASSETDIR)/030_GOC.PNG.pvr \
	$(ASSETDIR)/031_GOD.PNG.pvr \
	$(ASSETDIR)/032_FOC.PNG.pvr \
	$(ASSETDIR)/032_GOC.PNG.pvr \
	$(ASSETDIR)/033_FOA.PNG.pvr \
	$(ASSETDIR)/033_GOA.PNG.pvr \
	$(ASSETDIR)/034_FOC.PNG.pvr \
	$(ASSETDIR)/034_GOC.PNG.pvr \
	$(ASSETDIR)/035_FOC.PNG.pvr \
	$(ASSETDIR)/035_GOC.PNG.pvr \
	$(ASSETDIR)/036_FOC.PNG.pvr \
	$(ASSETDIR)/036_GOC.PNG.pvr \
	$(ASSETDIR)/037_FOC.PNG.pvr \
	$(ASSETDIR)/037_GOC.PNG.pvr \
	$(ASSETDIR)/041_FRB.PNG.pvr \
	$(ASSETDIR)/041_GRB.PNG.pvr \
	$(ASSETDIR)/044_GRC.PNG.pvr \
	$(ASSETDIR)/046_GRC.PNG.pvr \
	$(ASSETDIR)/054_GRC.PNG.pvr \
	$(ASSETDIR)/055_GRC.PNG.pvr \
	$(ASSETDIR)/056_GRC.PNG.pvr \
	$(ASSETDIR)/059_FRC.PNG.pvr \
	$(ASSETDIR)/060_FRC.PNG.pvr \
	$(ASSETDIR)/061_FRD.PNG.pvr \
	$(ASSETDIR)/063_GRC.PNG.pvr \
	$(ASSETDIR)/064_GRC.PNG.pvr \
	$(ASSETDIR)/066_FCB.PNG.pvr \
	$(ASSETDIR)/066_GCB.PNG.pvr \
	$(ASSETDIR)/067_FCC.PNG.pvr \
	$(ASSETDIR)/067_GCC.PNG.pvr \
	$(ASSETDIR)/068_GCD.PNG.pvr \
	$(ASSETDIR)/069_FCA.PNG.pvr \
	$(ASSETDIR)/069_GCA.PNG.pvr \
	$(ASSETDIR)/070_GOD.PNG.pvr \
	$(ASSETDIR)/071_FRC.PNG.pvr \
	$(ASSETDIR)/072_FRC.PNG.pvr \
	$(ASSETDIR)/073_FRC.PNG.pvr \
	$(ASSETDIR)/074_FRC.PNG.pvr \
	$(ASSETDIR)/075_FRC.PNG.pvr \
	$(ASSETDIR)/076_FRC.PNG.pvr \
	$(ASSETDIR)/077_FRC.PNG.pvr \
	$(ASSETDIR)/080_GCB.PNG.pvr \
	$(ASSETDIR)/082_FCD.PNG.pvr \
	$(ASSETDIR)/083_FRC.PNG.pvr \
	$(ASSETDIR)/085_GRC.PNG.pvr \
	$(ASSETDIR)/086_FOF.PNG.pvr \
	$(ASSETDIR)/086_GOF.PNG.pvr \
	$(ASSETDIR)/087_FCA.PNG.pvr \
	$(ASSETDIR)/087_GCA.PNG.pvr \
	$(ASSETDIR)/087_GCC.PNG.pvr \
	$(ASSETDIR)/088_FRC.PNG.pvr \
	$(ASSETDIR)/089_FRC.PNG.pvr \
	$(ASSETDIR)/092_GCA.PNG.pvr \
	$(ASSETDIR)/094_FOD.PNG.pvr \
	$(ASSETDIR)/095_FOD.PNG.pvr \
	$(ASSETDIR)/US_IL_13443_CHICAGO_35EAST_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13444_CHICAGO_LEOBURNETT_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13447_CHICAGO_REIDMURDOCH_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13448_CHICAGO_CARBIDE_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13449_CHICAGO_CROWNFOUNTAIN_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13451_CHICAGO_CULTURAL_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13453_CHICAGO_PRUDENTIAL_PART1_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13454_CHICAGO_UNITED_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13456_CHICAGO_SEVENTEENTH_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13458_CHICAGO_SMURFIT_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13459_CHICAGO_LASALLE_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13460_CHICAGO_TRUMP_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13461_CHICAGO_UNITRIN_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13462_CHICAGO_WILLOUGHBY_L.PNG.pvr \
	$(ASSETDIR)/US_IL_13490_CHICAGO_PRUDENTIAL_PART2_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_AONCENTER_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_ARTINSTITUTE_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_BOARDOFTHETRADE_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_BOEINGBUILDING_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_CHICAGOTHEATRE_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_CITYHALL_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_DALEY_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_HILTON_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_JAMESTHOMPSON_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_LIBRARY_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_MILLENIUMPARK1_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_MILLENIUMPARK2_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_MMART_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_OGILVIE_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_SEARSTOWER_L.PNG.pvr \
	$(ASSETDIR)/US_IL_CHICAGO_UNIONSTATION_L.PNG.pvr \
	$(ASSETDIR)/US_L_CONCRETE-COLOUR.PNG.pvr \
	$(ASSETDIR)/US_L_CONCRETE-DETAIL.PNG.pvr \
	$(ASSETDIR)/US_L_PARK-COLOUR.PNG.pvr \
	$(ASSETDIR)/US_L_WATER-COLOUR.PNG.pvr \
	$(ASSETDIR)/US_R_CONCRETE.PNG.pvr \
	$(ASSETDIR)/US_R_HIGHWAY-SOLID.PNG.pvr \
	$(ASSETDIR)/US_R_STREET-DASHED.PNG.pvr \
	$(ASSETDIR)/US_R_STREET-INNER-SHOULDER.PNG.pvr \
	$(ASSETDIR)/US_R_STREET-LANE-FILLER.PNG.pvr \
	$(ASSETDIR)/US_R_STREET-SOLID.PNG.pvr \
	$(ASSETDIR)/US_R_STREET-UNMARKED.PNG.pvr \
	$(ASSETDIR)/US_R_WALKWAY-SOLID.PNG.pvr \
	$(ASSETDIR)/US_R_WALKWAY-UNMARKED.PNG.pvr \
	$(ASSETDIR)/US_T_RAILROAD.PNG.pvr \
	$(ASSETDIR)/UIVertShader.vsh \
	$(ASSETDIR)/UIFragShader.fsh \
	$(ASSETDIR)/SkyboxFragShader.fsh \
	$(ASSETDIR)/SkyboxVertShader.vsh \
	$(ASSETDIR)/BuildingFragShader.fsh \
	$(ASSETDIR)/BuildingVertShader.vsh \
	$(ASSETDIR)/FullscreenVertShader.vsh \
	$(ASSETDIR)/FullscreenFragShader.fsh \
	$(ASSETDIR)/ShadowVolVertShader.vsh \
	$(ASSETDIR)/ShadowVolFragShader.fsh

$(ASSETDIR):
	-mkdir "$(ASSETDIR)"

$(ASSETDIR)/Skybox.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/Skybox.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/UIElements.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/UIElements.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/006_RUS.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/006_RUS.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/007_RUG.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/007_RUG.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/008_RUG.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/008_RUG.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/009_RUG.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/009_RUG.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/011_GIE.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/011_GIE.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/012_RSR.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/012_RSR.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/016_FOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/016_FOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/016_RTR.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/016_RTR.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/017_FOD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/017_FOD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/018_FOD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/018_FOD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/019_FOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/019_FOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/019_GOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/019_GOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/019_RZG.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/019_RZG.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/020_FOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/020_FOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/021_FOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/021_FOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/022_FOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/022_FOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/022_RUG.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/022_RUG.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/023_FOB.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/023_FOB.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/023_RUG.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/023_RUG.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/024_FOB.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/024_FOB.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/025_FOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/025_FOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/025_RUW.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/025_RUW.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/026_FOD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/026_FOD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/026_RUW.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/026_RUW.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/027_FOD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/027_FOD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/027_RUW.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/027_RUW.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/028_GOF.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/028_GOF.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/029_GCC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/029_GCC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/030_GOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/030_GOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/031_GOD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/031_GOD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/032_FOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/032_FOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/032_GOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/032_GOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/033_FOA.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/033_FOA.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/033_GOA.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/033_GOA.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/034_FOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/034_FOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/034_GOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/034_GOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/035_FOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/035_FOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/035_GOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/035_GOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/036_FOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/036_FOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/036_GOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/036_GOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/037_FOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/037_FOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/037_GOC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/037_GOC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/041_FRB.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/041_FRB.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/041_GRB.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/041_GRB.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/044_GRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/044_GRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/046_GRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/046_GRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/054_GRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/054_GRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/055_GRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/055_GRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/056_GRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/056_GRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/059_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/059_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/060_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/060_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/061_FRD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/061_FRD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/063_GRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/063_GRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/064_GRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/064_GRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/066_FCB.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/066_FCB.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/066_GCB.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/066_GCB.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/067_FCC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/067_FCC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/067_GCC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/067_GCC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/068_GCD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/068_GCD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/069_FCA.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/069_FCA.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/069_GCA.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/069_GCA.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/070_GOD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/070_GOD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/071_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/071_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/072_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/072_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/073_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/073_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/074_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/074_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/075_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/075_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/076_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/076_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/077_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/077_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/080_GCB.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/080_GCB.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/082_FCD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/082_FCD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/083_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/083_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/085_GRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/085_GRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/086_FOF.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/086_FOF.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/086_GOF.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/086_GOF.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/087_FCA.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/087_FCA.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/087_GCA.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/087_GCA.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/087_GCC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/087_GCC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/088_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/088_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/089_FRC.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/089_FRC.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/092_GCA.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/092_GCA.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/094_FOD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/094_FOD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/095_FOD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/095_FOD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13443_CHICAGO_35EAST_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13443_CHICAGO_35EAST_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13444_CHICAGO_LEOBURNETT_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13444_CHICAGO_LEOBURNETT_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13447_CHICAGO_REIDMURDOCH_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13447_CHICAGO_REIDMURDOCH_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13448_CHICAGO_CARBIDE_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13448_CHICAGO_CARBIDE_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13449_CHICAGO_CROWNFOUNTAIN_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13449_CHICAGO_CROWNFOUNTAIN_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13451_CHICAGO_CULTURAL_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13451_CHICAGO_CULTURAL_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13453_CHICAGO_PRUDENTIAL_PART1_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13453_CHICAGO_PRUDENTIAL_PART1_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13454_CHICAGO_UNITED_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13454_CHICAGO_UNITED_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13456_CHICAGO_SEVENTEENTH_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13456_CHICAGO_SEVENTEENTH_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13458_CHICAGO_SMURFIT_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13458_CHICAGO_SMURFIT_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13459_CHICAGO_LASALLE_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13459_CHICAGO_LASALLE_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13460_CHICAGO_TRUMP_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13460_CHICAGO_TRUMP_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13461_CHICAGO_UNITRIN_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13461_CHICAGO_UNITRIN_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13462_CHICAGO_WILLOUGHBY_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13462_CHICAGO_WILLOUGHBY_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_13490_CHICAGO_PRUDENTIAL_PART2_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_13490_CHICAGO_PRUDENTIAL_PART2_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_AONCENTER_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_AONCENTER_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_ARTINSTITUTE_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_ARTINSTITUTE_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_BOARDOFTHETRADE_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_BOARDOFTHETRADE_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_BOEINGBUILDING_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_BOEINGBUILDING_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_CHICAGOTHEATRE_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_CHICAGOTHEATRE_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_CITYHALL_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_CITYHALL_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_DALEY_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_DALEY_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_HILTON_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_HILTON_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_JAMESTHOMPSON_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_JAMESTHOMPSON_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_LIBRARY_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_LIBRARY_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_MILLENIUMPARK1_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_MILLENIUMPARK1_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_MILLENIUMPARK2_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_MILLENIUMPARK2_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_MMART_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_MMART_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_OGILVIE_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_OGILVIE_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_SEARSTOWER_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_SEARSTOWER_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_IL_CHICAGO_UNIONSTATION_L.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_IL_CHICAGO_UNIONSTATION_L.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_L_CONCRETE-COLOUR.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_L_CONCRETE-COLOUR.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_L_CONCRETE-DETAIL.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_L_CONCRETE-DETAIL.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_L_PARK-COLOUR.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_L_PARK-COLOUR.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_L_WATER-COLOUR.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_L_WATER-COLOUR.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_R_CONCRETE.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_R_CONCRETE.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_R_HIGHWAY-SOLID.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_R_HIGHWAY-SOLID.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_R_STREET-DASHED.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_R_STREET-DASHED.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_R_STREET-INNER-SHOULDER.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_R_STREET-INNER-SHOULDER.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_R_STREET-LANE-FILLER.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_R_STREET-LANE-FILLER.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_R_STREET-SOLID.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_R_STREET-SOLID.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_R_STREET-UNMARKED.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_R_STREET-UNMARKED.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_R_WALKWAY-SOLID.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_R_WALKWAY-SOLID.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_R_WALKWAY-UNMARKED.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_R_WALKWAY-UNMARKED.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/US_T_RAILROAD.PNG.pvr: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/US_T_RAILROAD.PNG.pvr $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/UIVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/UIVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/UIFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/UIFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkyboxFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/SkyboxFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/SkyboxVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/SkyboxVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/BuildingFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/BuildingFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/BuildingVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/BuildingVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FullscreenVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/FullscreenVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/FullscreenFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/FullscreenFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ShadowVolVertShader.vsh: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/ShadowVolVertShader.vsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/ShadowVolFragShader.fsh: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/OGLES2/ShadowVolFragShader.fsh $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/modelindex.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/modelindex.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/occlusiondata.nav: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/occlusiondata.nav $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/cameratrack.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/cameratrack.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_008_015_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_008_015_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_008_016_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_008_016_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_009_014_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_009_014_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_009_015_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_009_015_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_009_016_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_009_016_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_009_017_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_009_017_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_009_018_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_009_018_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_010_014_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_010_014_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_010_015_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_010_015_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_010_015_H.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_010_015_H.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_010_016_H.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_010_016_H.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_010_016_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_010_016_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_010_017_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_010_017_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_010_017_H.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_010_017_H.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_010_018_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_010_018_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_011_013_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_011_013_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_011_014_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_011_014_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_011_015_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_011_015_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_011_015_H.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_011_015_H.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_011_016_H.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_011_016_H.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_011_016_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_011_016_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_011_017_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_011_017_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_011_017_H.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_011_017_H.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_011_018_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_011_018_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_012_014_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_012_014_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_012_015_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_012_015_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_012_016_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_012_016_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_012_017_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_012_017_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_013_015_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_013_015_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

$(ASSETDIR)/CM_US_IL_CHICAGO_013_016_L.DAE.pod: $(PVRSDKDIR)/Examples/Advanced/Navigation3D/Data/CM_US_IL_CHICAGO_013_016_L.DAE.pod $(ASSETDIR)
	$(CPY) $(subst /,$(SEPARATOR),"$<" "$(ASSETDIR)")

endif


#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESGlass
DEPENDS =  PVRCore PVRAssets PVRNativeGles PVRGles PVRShell PVREngineUtils
LIBRARIES =  PVREngineUtils PVRShell PVRGles PVRNativeGles PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESGlass.o

RESOURCES += \
           DefaultVertShader_ES2.vsh\
           DefaultFragShader_ES2.fsh\
           ParaboloidVertShader_ES2.vsh\
           SkyboxVertShader_ES2.vsh\
           SkyboxFragShader_ES2.fsh\
           EffectVertShader_ES2.vsh\
           EffectFragShader_ES2.fsh\

COMMON_RESOURCES += \
           BalloonTex.pvr\
           BalloonTex2.pvr\
           SkyboxTex.pvr\
           Balloon.pod\
           scene.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

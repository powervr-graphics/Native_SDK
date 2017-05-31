#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESSkinning
DEPENDS =  PVRCore PVRAssets PVRNativeGles PVRGles PVRShell PVREngineUtils
LIBRARIES =  PVREngineUtils PVRShell PVRGles PVRNativeGles PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESSkinning.o

RESOURCES += \
           DefaultVertShader_ES2.vsh\
           DefaultFragShader_ES2.fsh\
           DefaultVertShader_ES3.vsh\
           DefaultFragShader_ES3.fsh\
           SkinnedVertShader_ES2.vsh\
           SkinnedFragShader_ES2.fsh\
           SkinnedVertShader_ES3.vsh\
           SkinnedFragShader_ES3.fsh\
           Skinning_es.pfx\

COMMON_RESOURCES += \
           Arm.pvr\
           Grid.pvr\
           Pipes.pvr\
           RobotDiffuse.pvr\
           RobotNormal.pvr\
           Tiles.pvr\
           Robot.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

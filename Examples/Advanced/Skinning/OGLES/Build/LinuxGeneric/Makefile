#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESSkinning
DEPENDS =  PVRCore PVRAssets PVRShell PVRUtilsGles
LIBRARIES =  PVRUtilsGles PVRShell PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESSkinning.o

RESOURCES += \
           DefaultVertShader.vsh\
           DefaultFragShader.fsh\
           SkinnedVertShader.vsh\
           SkinnedFragShader.fsh\

COMMON_RESOURCES += \
           Arm.pvr\
           Grid.pvr\
           Pipes.pvr\
           RobotDiffuse.pvr\
           RobotNormal.pvr\
           Tiles.pvr\
           Robot.pod\




include $(SDKDIR)/Builds/Linux/make_example.mak

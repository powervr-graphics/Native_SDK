#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESIMGFramebufferDownsample
DEPENDS =  PVRCore PVRAssets PVRShell PVRUtilsGles
LIBRARIES =  PVRUtilsGles PVRShell PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESIMGFramebufferDownsample.o

RESOURCES += \
           FragShader.fsh\
           VertShader.vsh\
           BlitFragShader.fsh\
           BlitVertShader.vsh\
           HalfAndHalfFragShader.fsh\
           HalfAndHalfVertShader.vsh\





include $(SDKDIR)/Builds/Linux/make_example.mak

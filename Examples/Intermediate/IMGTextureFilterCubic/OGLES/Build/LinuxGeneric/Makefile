#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESIMGTextureFilterCubic
DEPENDS =  PVRCore PVRAssets PVRShell PVRUtilsGles
LIBRARIES =  PVRUtilsGles PVRShell PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESIMGTextureFilterCubic.o

RESOURCES += \
           FragShader.fsh\
           VertShader.vsh\





include $(SDKDIR)/Builds/Linux/make_example.mak

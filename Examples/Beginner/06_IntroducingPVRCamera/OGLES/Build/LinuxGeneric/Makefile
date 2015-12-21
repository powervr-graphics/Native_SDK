#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESIntroducingPVRCamera
DEPENDS =  PVRCore PVRAssets PVREgl PVRGles PVRShell PVRUIRenderer PVRCamera
LIBRARIES =  PVRCamera PVRUIRenderer PVRShell PVRGles PVREgl PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESIntroducingPVRCamera.o

RESOURCES += \
           VertShader.vsh\
           FragShader.fsh\



include $(SDKDIR)/Builds/Linux/make_example.mak

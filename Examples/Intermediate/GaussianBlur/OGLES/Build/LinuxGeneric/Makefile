#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESGaussianBlur
DEPENDS =  PVRCore PVRAssets PVRShell PVRUtilsGles
LIBRARIES =  PVRUtilsGles PVRShell PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESGaussianBlur.o

RESOURCES += \
           FragShader_ES3.fsh\
           VertShader_ES3.vsh\
           CompShader_ES3.csh\

COMMON_RESOURCES += \
           Lenna.pvr\




include $(SDKDIR)/Builds/Linux/make_example.mak

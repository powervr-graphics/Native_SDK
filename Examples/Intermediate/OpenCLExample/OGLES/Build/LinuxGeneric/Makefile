#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESOpenCLExample
DEPENDS =  PVRCore PVRAssets PVRShell PVRUtilsGles
LIBRARIES =  PVRUtilsGles PVRShell PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESOpenCLExample.o

RESOURCES += \
           QuadFragShader_ES3.fsh\
           QuadVertShader_ES3.vsh\
           ConvolutionKernel.cl\

COMMON_RESOURCES += \
           Lenna.pvr\




include $(SDKDIR)/Builds/Linux/make_example.mak

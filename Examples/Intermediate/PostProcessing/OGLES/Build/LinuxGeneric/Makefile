#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESPostProcessing
DEPENDS =  PVRCore PVRAssets PVRShell PVRUtilsGles
LIBRARIES =  PVRUtilsGles PVRShell PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESPostProcessing.o

RESOURCES += \
           BlurFragShader_ES3.fsh\
           BlurVertShader_ES3.vsh\
           FragShader_ES3.fsh\
           PostBloomFragShader_ES3.fsh\
           PreBloomFragShader_ES3.fsh\
           QuadVertShader_ES3.vsh\
           VertShader_ES3.vsh\

COMMON_RESOURCES += \
           Marble.pvr\
           scene.pod\




include $(SDKDIR)/Builds/Linux/make_example.mak

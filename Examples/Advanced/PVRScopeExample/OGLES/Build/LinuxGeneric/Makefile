#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESPVRScopeExample
DEPENDS =  PVRCore PVRAssets PVRNativeGles PVRGles PVRShell PVREngineUtils
LIBRARIES =  PVRScopeDeveloper dl PVREngineUtils PVRShell PVRGles PVRNativeGles PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS) ../../../../../../Builds/Linux/$(PLAT_SUFFIX)/Lib

OBJECTS +=	\
           OGLES/OGLESPVRScopeExample.o	\
           PVRScopeGraph.o

RESOURCES += \
           FragShader_ES2.fsh\
           FragShader_ES3.fsh\
           VertShader_ES2.vsh\
           VertShader_ES3.vsh\
           GraphFragShader_ES2.fsh\
           GraphFragShader_ES3.fsh\
           GraphVertShader_ES2.vsh\
           GraphVertShader_ES3.vsh\

COMMON_RESOURCES += \
           Marble.pvr\
           scene.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

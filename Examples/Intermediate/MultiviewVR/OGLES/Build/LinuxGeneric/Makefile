#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESMultiviewVR
DEPENDS =  PVRCore PVRAssets PVRNativeGles PVRShell
LIBRARIES =  PVRShell PVRNativeGles PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESMultiviewVR.o

RESOURCES += \
           FragShader_ES3.fsh\
           VertShader_ES3.vsh\
           TexQuadFragShader_ES3.fsh\
           TexQuadVertShader_ES3.vsh\

COMMON_RESOURCES += \
           GnomeWood.pvr\
           DarkWood.pvr\
           ClearWood.pvr\
           GnomeToy.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

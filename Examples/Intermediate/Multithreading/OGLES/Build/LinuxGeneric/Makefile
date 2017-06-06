#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESMultithreading
DEPENDS =  PVRCore PVRAssets PVRNativeGles PVRGles PVRShell PVREngineUtils
LIBRARIES =  PVREngineUtils PVRShell PVRGles PVRNativeGles PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESMultithreading.o

RESOURCES += \
           FragShader_ES3.fsh\
           VertShader_ES3.vsh\

COMMON_RESOURCES += \
           MarbleNormalMap.pvr\
           Marble.pvr\
           scene.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

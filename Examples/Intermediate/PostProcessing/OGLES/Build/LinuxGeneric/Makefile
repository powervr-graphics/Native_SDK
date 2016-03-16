#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESPostProcessing
DEPENDS =  PVRCore PVRAssets PVREgl PVRNativeGles PVRGles PVRShell PVRUIRenderer
LIBRARIES =  PVRUIRenderer PVRShell PVRGles PVRNativeGles PVREgl PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESPostProcessing.o

RESOURCES += \
           PostBloomFragShader_ES2.fsh\
           PostBloomFragShader_ES3.fsh\
           PostBloomVertShader_ES2.vsh\
           PostBloomVertShader_ES3.vsh\
           PreBloomFragShader_ES2.fsh\
           PreBloomFragShader_ES3.fsh\
           PreBloomVertShader_ES2.vsh\
           PreBloomVertShader_ES3.vsh\
           BlurFragShader_ES2.fsh\
           BlurFragShader_ES3.fsh\
           BlurVertShader_ES2.vsh\
           BlurVertShader_ES3.vsh\
           FragShader_ES2.fsh\
           FragShader_ES3.fsh\
           VertShader_ES2.vsh\
           VertShader_ES3.vsh\

COMMON_RESOURCES += \
           Marble.pvr\
           scene.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

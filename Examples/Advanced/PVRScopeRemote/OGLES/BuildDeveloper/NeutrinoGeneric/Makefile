#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESPVRScopeRemote
DEPENDS =  PVRCore PVRAssets PVREgl PVRGles PVRShell PVRUIRenderer
LIBRARIES =  PVRScopeDeveloper socket PVRUIRenderer PVRShell PVRGles PVREgl PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Neutrino_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS) ../../../../../../Builds/Neutrino/$(PLAT_SUFFIX)/Lib

OBJECTS +=	\
           OGLES/OGLESPVRScopeRemote.o

RESOURCES += \
           FragShader_ES2.fsh\
           FragShader_ES3.fsh\
           VertShader_ES2.vsh\
           VertShader_ES3.vsh\

COMMON_RESOURCES += \
           Marble.pvr\
           scene.pod\


include $(SDKDIR)/Builds/Neutrino/make_example.mak

#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESPVRScopeRemote
DEPENDS =  PVRCore PVRAssets PVREgl PVRNativeGles PVRGles PVRShell PVRUIRenderer
LIBRARIES =  PVRScopeDeveloper pthread rt dl PVRUIRenderer PVRShell PVRGles PVRNativeGles PVREgl PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS) ../../../../../../Builds/Linux/$(PLAT_SUFFIX)/Lib

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


include $(SDKDIR)/Builds/Linux/make_example.mak

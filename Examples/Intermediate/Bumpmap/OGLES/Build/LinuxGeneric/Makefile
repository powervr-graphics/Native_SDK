#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESBumpmap
DEPENDS =  PVRCore PVRAssets PVREgl PVRGles PVRShell PVRUIRenderer
LIBRARIES =  PVRUIRenderer PVRShell PVRGles PVREgl PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESBumpmap.o

RESOURCES += \
           FragShader_ES2.fsh\
           FragShader_ES3.fsh\
           VertShader_ES2.vsh\
           VertShader_ES3.vsh\

COMMON_RESOURCES += \
           MarbleNormalMap.pvr\
           Marble.pvr\
           scene.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

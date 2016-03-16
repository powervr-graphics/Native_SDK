#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESIntroducingPVRAssets
DEPENDS =  PVRCore PVRAssets PVREgl PVRNativeGles PVRShell
LIBRARIES =  PVRShell PVRNativeGles PVREgl PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESIntroducingPVRAssets.o

RESOURCES += \
           FragShader_ES2.fsh\
           VertShader_ES2.vsh\

COMMON_RESOURCES += \
           GnomeWood.pvr\
           DarkWood.pvr\
           ClearWood.pvr\
           GnomeToy.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

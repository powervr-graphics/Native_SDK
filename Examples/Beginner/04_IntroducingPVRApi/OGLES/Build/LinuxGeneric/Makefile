#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESIntroducingPVRApi
DEPENDS =  PVRCore PVRAssets PVREgl PVRGles PVRShell PVRUIRenderer
LIBRARIES =  PVRUIRenderer PVRShell PVRGles PVREgl PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESIntroducingPVRApi.o

RESOURCES += \
           effect_ES2.pfx\
           effect_ES3.pfx\

COMMON_RESOURCES += \
           GnomeWood.pvr\
           DarkWood.pvr\
           ClearWood.pvr\
           GnomeToy.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

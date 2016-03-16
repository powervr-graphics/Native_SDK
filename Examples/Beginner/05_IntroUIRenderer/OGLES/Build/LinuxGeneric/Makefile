#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESIntroUIRenderer
DEPENDS =  PVRCore PVRAssets PVREgl PVRNativeGles PVRGles PVRShell PVRUIRenderer
LIBRARIES =  PVRUIRenderer PVRShell PVRGles PVRNativeGles PVREgl PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESIntroUIRenderer.o


COMMON_RESOURCES += \
           arial_36.pvr\
           starjout_60.pvr\
           title_36.pvr\
           title_46.pvr\
           title_56.pvr\
           Text.txt\
           titletext.txt\


include $(SDKDIR)/Builds/Linux/make_example.mak

#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESIntroducingPVRShell
DEPENDS =  PVRCore PVREgl PVRShell
LIBRARIES =  GLESv2 PVRShell PVREgl PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS) ../../../../../../Builds/Linux/$(PLAT_SUFFIX)/Lib

OBJECTS +=	\
           OGLES/OGLESIntroducingPVRShell.o




include $(SDKDIR)/Builds/Linux/make_example.mak

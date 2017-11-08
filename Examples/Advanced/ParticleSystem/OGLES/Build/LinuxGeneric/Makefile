#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESParticleSystem
DEPENDS =  PVRCore PVRAssets PVRShell PVRUtilsGles
LIBRARIES =  PVRUtilsGles PVRShell PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESParticleSystem.o

RESOURCES += \
           FragShader.fsh\
           VertShader.vsh\
           ParticleFragShader.fsh\
           ParticleVertShader.vsh\
           ParticleSolver.csh\

COMMON_RESOURCES += \
           sphere.pod\




include $(SDKDIR)/Builds/Linux/make_example.mak

#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESParticleSystem
DEPENDS =  PVRCore PVRAssets PVREgl PVRGles PVRShell PVRUIRenderer
LIBRARIES =  PVRUIRenderer PVRShell PVRGles PVREgl PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESParticleSystem.o	\
           OGLES/ParticleSystemGPU.o

RESOURCES += \
           FragShader_ES31.fsh\
           VertShader_ES31.vsh\
           ParticleFragShader_ES31.fsh\
           ParticleVertShader_ES31.vsh\
           ParticleSolver_ES31.csh\

COMMON_RESOURCES += \
           sphere.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

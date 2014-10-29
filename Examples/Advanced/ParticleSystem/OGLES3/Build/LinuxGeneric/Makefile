#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES3ParticleSystem
APIS    = OGLES3
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES3ParticleSystem.o	\
           ParticleSystemGPU.o	\
           sphere.o	\
           ParticleGradient.o	\
           FragShader.o	\
           VertShader.o	\
           FloorFragShader.o	\
           FloorVertShader.o	\
           ParticleFragShader.o	\
           ParticleVertShader.o	\
           ParticleSolver.o

VPATH +=	\
         ../../Content

INCLUDES += 	\
         -I../..

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/sphere.cpp ../../Content/ParticleGradient.cpp ../../Content/FragShader.cpp ../../Content/VertShader.cpp ../../Content/FloorFragShader.cpp ../../Content/FloorVertShader.cpp ../../Content/ParticleFragShader.cpp ../../Content/ParticleVertShader.cpp ../../Content/ParticleSolver.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


#--------------------------------------------------------------------------
# Name         : content.mak
# Title        : Makefile to build content files
#
# Copyright    : Copyright (C) by Imagination Technologies Limited.
#
# Description  : Makefile to wrap content files for examples in the PowerVR SDK
#
# Platform     :
#
#--------------------------------------------------------------------------

#############################################################################
## Variables
#############################################################################
FILEWRAP 	= ..\..\..\..\Utilities\Filewrap\Windows_x86_32\Filewrap.exe
CONTENTDIR = Content

#############################################################################
## Instructions
#############################################################################

RESOURCES = \
	$(CONTENTDIR)/sphere.cpp \
	$(CONTENTDIR)/ParticleGradient.cpp \
	$(CONTENTDIR)/FragShader.cpp \
	$(CONTENTDIR)/VertShader.cpp \
	$(CONTENTDIR)/FloorFragShader.cpp \
	$(CONTENTDIR)/FloorVertShader.cpp \
	$(CONTENTDIR)/ParticleFragShader.cpp \
	$(CONTENTDIR)/ParticleVertShader.cpp \
	$(CONTENTDIR)/ParticleSolver.cpp

all: resources
	
help:
	@echo Valid targets are:
	@echo resources, clean
	@echo FILEWRAP can be used to override the default path to the Filewrap utility.

clean:
	@for i in $(RESOURCES); do test -f $$i && rm -vf $$i || true; done

resources: $(RESOURCES)

$(CONTENTDIR):
	-mkdir "$@"

$(CONTENTDIR)/sphere.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./sphere.pod

$(CONTENTDIR)/ParticleGradient.cpp: $(CONTENTDIR) ./ParticleGradient.pvr
	$(FILEWRAP)  -o $@ ./ParticleGradient.pvr

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR) ./FragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR) ./VertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh

$(CONTENTDIR)/FloorFragShader.cpp: $(CONTENTDIR) ./FloorFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./FloorFragShader.fsh

$(CONTENTDIR)/FloorVertShader.cpp: $(CONTENTDIR) ./FloorVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./FloorVertShader.vsh

$(CONTENTDIR)/ParticleFragShader.cpp: $(CONTENTDIR) ./ParticleFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./ParticleFragShader.fsh

$(CONTENTDIR)/ParticleVertShader.cpp: $(CONTENTDIR) ./ParticleVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./ParticleVertShader.vsh

$(CONTENTDIR)/ParticleSolver.cpp: $(CONTENTDIR) ./ParticleSolver.csh
	$(FILEWRAP)  -s  -o $@ ./ParticleSolver.csh

############################################################################
# End of file (content.mak)
############################################################################

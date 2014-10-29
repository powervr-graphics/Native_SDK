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
	$(CONTENTDIR)/ParticleFragShader.cpp \
	$(CONTENTDIR)/ParticleVertShader.cpp

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

$(CONTENTDIR)/ParticleGradient.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./ParticleGradient.pvr

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh
	-$(FILEWRAP)  -oa $@ ./FragShader.fsc

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh
	-$(FILEWRAP)  -oa $@ ./VertShader.vsc

$(CONTENTDIR)/ParticleFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./ParticleFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./ParticleFragShader.fsc

$(CONTENTDIR)/ParticleVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./ParticleVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./ParticleVertShader.vsc

############################################################################
# End of file (content.mak)
############################################################################

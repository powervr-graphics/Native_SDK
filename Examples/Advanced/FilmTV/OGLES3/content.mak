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
	$(CONTENTDIR)/Table.cpp \
	$(CONTENTDIR)/Floor.cpp \
	$(CONTENTDIR)/Wall.cpp \
	$(CONTENTDIR)/TV.cpp \
	$(CONTENTDIR)/TVCase.cpp \
	$(CONTENTDIR)/TVSpeaker.cpp \
	$(CONTENTDIR)/Alum.cpp \
	$(CONTENTDIR)/Skirting.cpp \
	$(CONTENTDIR)/Camera.cpp \
	$(CONTENTDIR)/FragShader.cpp \
	$(CONTENTDIR)/BWFragShader.cpp \
	$(CONTENTDIR)/VertShader.cpp \
	$(CONTENTDIR)/FilmTVScene.cpp

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

$(CONTENTDIR)/Table.cpp: $(CONTENTDIR) ./Table.pvr
	$(FILEWRAP)  -o $@ ./Table.pvr

$(CONTENTDIR)/Floor.cpp: $(CONTENTDIR) ./Floor.pvr
	$(FILEWRAP)  -o $@ ./Floor.pvr

$(CONTENTDIR)/Wall.cpp: $(CONTENTDIR) ./Wall.pvr
	$(FILEWRAP)  -o $@ ./Wall.pvr

$(CONTENTDIR)/TV.cpp: $(CONTENTDIR) ./TV.pvr
	$(FILEWRAP)  -o $@ ./TV.pvr

$(CONTENTDIR)/TVCase.cpp: $(CONTENTDIR) ./TVCase.pvr
	$(FILEWRAP)  -o $@ ./TVCase.pvr

$(CONTENTDIR)/TVSpeaker.cpp: $(CONTENTDIR) ./TVSpeaker.pvr
	$(FILEWRAP)  -o $@ ./TVSpeaker.pvr

$(CONTENTDIR)/Alum.cpp: $(CONTENTDIR) ./Alum.pvr
	$(FILEWRAP)  -o $@ ./Alum.pvr

$(CONTENTDIR)/Skirting.cpp: $(CONTENTDIR) ./Skirting.pvr
	$(FILEWRAP)  -o $@ ./Skirting.pvr

$(CONTENTDIR)/Camera.cpp: $(CONTENTDIR) ./Camera.pvr
	$(FILEWRAP)  -o $@ ./Camera.pvr

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR) ./FragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh

$(CONTENTDIR)/BWFragShader.cpp: $(CONTENTDIR) ./BWFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./BWFragShader.fsh

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR) ./VertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh

$(CONTENTDIR)/FilmTVScene.cpp: $(CONTENTDIR) ./FilmTVScene.pod
	$(FILEWRAP)  -o $@ ./FilmTVScene.pod

############################################################################
# End of file (content.mak)
############################################################################

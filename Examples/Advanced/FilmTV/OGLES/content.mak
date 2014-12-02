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

$(CONTENTDIR)/Table.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Table.pvr

$(CONTENTDIR)/Floor.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Floor.pvr

$(CONTENTDIR)/Wall.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Wall.pvr

$(CONTENTDIR)/TV.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./TV.pvr

$(CONTENTDIR)/TVCase.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./TVCase.pvr

$(CONTENTDIR)/TVSpeaker.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./TVSpeaker.pvr

$(CONTENTDIR)/Alum.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Alum.pvr

$(CONTENTDIR)/Skirting.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Skirting.pvr

$(CONTENTDIR)/Camera.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Camera.pvr

$(CONTENTDIR)/FilmTVScene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./FilmTVScene.pod

############################################################################
# End of file (content.mak)
############################################################################

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
	$(CONTENTDIR)/effect.cpp \
	$(CONTENTDIR)/blend_minmax_scene.cpp \
	$(CONTENTDIR)/skinTex.cpp

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

$(CONTENTDIR)/effect.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./effect.pfx

$(CONTENTDIR)/blend_minmax_scene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./blend_minmax_scene.POD

$(CONTENTDIR)/skinTex.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./skinTex.pvr

############################################################################
# End of file (content.mak)
############################################################################

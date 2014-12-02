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
	$(CONTENTDIR)/scene.cpp \
	$(CONTENTDIR)/pointlight.cpp \
	$(CONTENTDIR)/light_cubemap.cpp \
	$(CONTENTDIR)/mask_texture.cpp \
	$(CONTENTDIR)/mask_bump.cpp

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

$(CONTENTDIR)/scene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./scene.pod

$(CONTENTDIR)/pointlight.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./pointlight.pod

$(CONTENTDIR)/light_cubemap.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./light_cubemap.pvr

$(CONTENTDIR)/mask_texture.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./mask_texture.pvr

$(CONTENTDIR)/mask_bump.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./mask_bump.pvr

############################################################################
# End of file (content.mak)
############################################################################

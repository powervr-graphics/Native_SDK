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
	$(CONTENTDIR)/texture.cpp \
	$(CONTENTDIR)/effect.cpp \
	$(CONTENTDIR)/scene.cpp

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

$(CONTENTDIR)/texture.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./texture.pvr

$(CONTENTDIR)/effect.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./effect.pfx

$(CONTENTDIR)/scene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./scene.pod

############################################################################
# End of file (content.mak)
############################################################################

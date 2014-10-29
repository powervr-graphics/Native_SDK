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
	$(CONTENTDIR)/Marble.cpp \
	$(CONTENTDIR)/Floor.cpp \
	$(CONTENTDIR)/LanternCubemap.cpp \
	$(CONTENTDIR)/MagicLanternShaders.cpp \
	$(CONTENTDIR)/MagicLantern.cpp

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

$(CONTENTDIR)/Marble.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Marble.pvr

$(CONTENTDIR)/Floor.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Floor.pvr

$(CONTENTDIR)/LanternCubemap.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./LanternCubemap.pvr

$(CONTENTDIR)/MagicLanternShaders.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./MagicLanternShaders.pfx

$(CONTENTDIR)/MagicLantern.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./MagicLantern.pod

############################################################################
# End of file (content.mak)
############################################################################

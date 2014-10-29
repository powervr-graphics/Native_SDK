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
	$(CONTENTDIR)/Head_clonespacePVRTC.cpp \
	$(CONTENTDIR)/Head_clonespaceBGRA.cpp \
	$(CONTENTDIR)/Head_diffuse.cpp \
	$(CONTENTDIR)/Head.cpp

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

$(CONTENTDIR)/Head_clonespacePVRTC.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Head_clonespacePVRTC.pvr

$(CONTENTDIR)/Head_clonespaceBGRA.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Head_clonespaceBGRA.pvr

$(CONTENTDIR)/Head_diffuse.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Head_diffuse.pvr

$(CONTENTDIR)/Head.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Head.pod

############################################################################
# End of file (content.mak)
############################################################################

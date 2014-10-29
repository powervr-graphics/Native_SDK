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
	$(CONTENTDIR)/Scene.cpp \
	$(CONTENTDIR)/Basetex.cpp \
	$(CONTENTDIR)/Reflection.cpp

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

$(CONTENTDIR)/Scene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Scene.pod

$(CONTENTDIR)/Basetex.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Basetex.pvr

$(CONTENTDIR)/Reflection.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Reflection.pvr

############################################################################
# End of file (content.mak)
############################################################################

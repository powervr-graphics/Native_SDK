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

$(CONTENTDIR)/effect.cpp: $(CONTENTDIR) ./effect.pfx
	$(FILEWRAP)  -s  -o $@ ./effect.pfx

$(CONTENTDIR)/Scene.cpp: $(CONTENTDIR) ./Scene.pod
	$(FILEWRAP)  -o $@ ./Scene.pod

$(CONTENTDIR)/Basetex.cpp: $(CONTENTDIR) ./Basetex.pvr
	$(FILEWRAP)  -o $@ ./Basetex.pvr

$(CONTENTDIR)/Reflection.cpp: $(CONTENTDIR) ./Reflection.pvr
	$(FILEWRAP)  -o $@ ./Reflection.pvr

############################################################################
# End of file (content.mak)
############################################################################

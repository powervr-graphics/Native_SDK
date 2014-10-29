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
	$(CONTENTDIR)/Flora.cpp \
	$(CONTENTDIR)/Backgrnd.cpp \
	$(CONTENTDIR)/Reflection.cpp \
	$(CONTENTDIR)/Vase.cpp

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

$(CONTENTDIR)/Flora.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Flora.pvr

$(CONTENTDIR)/Backgrnd.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Backgrnd.pvr

$(CONTENTDIR)/Reflection.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Reflection.pvr

$(CONTENTDIR)/Vase.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Vase.pod

############################################################################
# End of file (content.mak)
############################################################################

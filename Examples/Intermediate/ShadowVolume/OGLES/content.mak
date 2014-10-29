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
	$(CONTENTDIR)/scene.cpp \
	$(CONTENTDIR)/Background.cpp \
	$(CONTENTDIR)/Rust.cpp

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

$(CONTENTDIR)/scene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./scene.pod

$(CONTENTDIR)/Background.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Background.pvr

$(CONTENTDIR)/Rust.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Rust.pvr

############################################################################
# End of file (content.mak)
############################################################################

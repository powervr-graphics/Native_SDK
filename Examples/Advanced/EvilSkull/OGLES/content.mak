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
	$(CONTENTDIR)/Iris.cpp \
	$(CONTENTDIR)/Metal.cpp \
	$(CONTENTDIR)/Fire02.cpp \
	$(CONTENTDIR)/Fire03.cpp \
	$(CONTENTDIR)/EvilSkull.cpp

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

$(CONTENTDIR)/Iris.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Iris.pvr

$(CONTENTDIR)/Metal.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Metal.pvr

$(CONTENTDIR)/Fire02.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Fire02.pvr

$(CONTENTDIR)/Fire03.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Fire03.pvr

$(CONTENTDIR)/EvilSkull.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./EvilSkull.pod

############################################################################
# End of file (content.mak)
############################################################################

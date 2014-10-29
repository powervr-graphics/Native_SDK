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
	$(CONTENTDIR)/Text.cpp \
	$(CONTENTDIR)/arial_36.cpp \
	$(CONTENTDIR)/starjout_60.cpp \
	$(CONTENTDIR)/title_36.cpp \
	$(CONTENTDIR)/title_46.cpp \
	$(CONTENTDIR)/title_56.cpp

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

$(CONTENTDIR)/Text.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Text.txt

$(CONTENTDIR)/arial_36.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./arial_36.pvr

$(CONTENTDIR)/starjout_60.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./starjout_60.pvr

$(CONTENTDIR)/title_36.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./title_36.pvr

$(CONTENTDIR)/title_46.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./title_46.pvr

$(CONTENTDIR)/title_56.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./title_56.pvr

############################################################################
# End of file (content.mak)
############################################################################

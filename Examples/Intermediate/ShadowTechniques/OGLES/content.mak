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
	$(CONTENTDIR)/Blob.cpp \
	$(CONTENTDIR)/TableCover.cpp \
	$(CONTENTDIR)/Kettle.cpp \
	$(CONTENTDIR)/Scene.cpp

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

$(CONTENTDIR)/Blob.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Blob.pvr

$(CONTENTDIR)/TableCover.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./TableCover.pvr

$(CONTENTDIR)/Kettle.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Kettle.pvr

$(CONTENTDIR)/Scene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Scene.pod

############################################################################
# End of file (content.mak)
############################################################################

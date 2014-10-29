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
	$(CONTENTDIR)/FragShader.cpp \
	$(CONTENTDIR)/VertShader.cpp \
	$(CONTENTDIR)/Box.cpp \
	$(CONTENTDIR)/base_COLOR.cpp \
	$(CONTENTDIR)/base_NRM.cpp \
	$(CONTENTDIR)/base_DISP.cpp \
	$(CONTENTDIR)/base_SPEC.cpp

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

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh

$(CONTENTDIR)/Box.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Box.pod

$(CONTENTDIR)/base_COLOR.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./base_COLOR.pvr

$(CONTENTDIR)/base_NRM.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./base_NRM.pvr

$(CONTENTDIR)/base_DISP.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./base_DISP.pvr

$(CONTENTDIR)/base_SPEC.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./base_SPEC.pvr

############################################################################
# End of file (content.mak)
############################################################################

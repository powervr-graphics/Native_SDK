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
	$(CONTENTDIR)/man.cpp \
	$(CONTENTDIR)/Body.cpp \
	$(CONTENTDIR)/Legs.cpp \
	$(CONTENTDIR)/Belt.cpp

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

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR) ./FragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh
	-$(FILEWRAP)  -oa $@ ./FragShader.fsc

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR) ./VertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh
	-$(FILEWRAP)  -oa $@ ./VertShader.vsc

$(CONTENTDIR)/man.cpp: $(CONTENTDIR) ./man.pod
	$(FILEWRAP)  -o $@ ./man.pod

$(CONTENTDIR)/Body.cpp: $(CONTENTDIR) ./Body.pvr
	$(FILEWRAP)  -o $@ ./Body.pvr

$(CONTENTDIR)/Legs.cpp: $(CONTENTDIR) ./Legs.pvr
	$(FILEWRAP)  -o $@ ./Legs.pvr

$(CONTENTDIR)/Belt.cpp: $(CONTENTDIR) ./Belt.pvr
	$(FILEWRAP)  -o $@ ./Belt.pvr

############################################################################
# End of file (content.mak)
############################################################################

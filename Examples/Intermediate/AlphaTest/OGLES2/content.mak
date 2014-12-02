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
	$(CONTENTDIR)/TexFragShader.cpp \
	$(CONTENTDIR)/DiscardFragShader.cpp \
	$(CONTENTDIR)/VertShader.cpp \
	$(CONTENTDIR)/Wallwire.cpp

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

$(CONTENTDIR)/TexFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./TexFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./TexFragShader.fsc

$(CONTENTDIR)/DiscardFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./DiscardFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./DiscardFragShader.fsc

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh
	-$(FILEWRAP)  -oa $@ ./VertShader.vsc

$(CONTENTDIR)/Wallwire.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Wallwire.pvr

############################################################################
# End of file (content.mak)
############################################################################

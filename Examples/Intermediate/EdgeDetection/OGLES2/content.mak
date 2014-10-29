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
	$(CONTENTDIR)/PreFragShader.cpp \
	$(CONTENTDIR)/PreVertShader.cpp \
	$(CONTENTDIR)/PostFragShader.cpp \
	$(CONTENTDIR)/PostVertShader.cpp \
	$(CONTENTDIR)/SketchObject.cpp

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

$(CONTENTDIR)/PreFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./PreFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./PreFragShader.fsc

$(CONTENTDIR)/PreVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./PreVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./PreVertShader.vsc

$(CONTENTDIR)/PostFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./PostFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./PostFragShader.fsc

$(CONTENTDIR)/PostVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./PostVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./PostVertShader.vsc

$(CONTENTDIR)/SketchObject.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./SketchObject.pod

############################################################################
# End of file (content.mak)
############################################################################

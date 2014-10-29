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
	$(CONTENTDIR)/VertShader.cpp \
	$(CONTENTDIR)/FragShader.cpp \
	$(CONTENTDIR)/FeedbackVertShader.cpp \
	$(CONTENTDIR)/FeedbackFragShader.cpp

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

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh

$(CONTENTDIR)/FeedbackVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./FeedbackVertShader.vsh

$(CONTENTDIR)/FeedbackFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./FeedbackFragShader.fsh

############################################################################
# End of file (content.mak)
############################################################################

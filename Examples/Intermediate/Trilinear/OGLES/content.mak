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
	$(CONTENTDIR)/Back.cpp \
	$(CONTENTDIR)/Tape.cpp \
	$(CONTENTDIR)/Ball.cpp \
	$(CONTENTDIR)/Info.cpp \
	$(CONTENTDIR)/o_model.cpp

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

$(CONTENTDIR)/Back.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Back.pvr

$(CONTENTDIR)/Tape.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Tape.pvr

$(CONTENTDIR)/Ball.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Ball.pvr

$(CONTENTDIR)/Info.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Info.pvr

$(CONTENTDIR)/o_model.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./o_model.pod

############################################################################
# End of file (content.mak)
############################################################################

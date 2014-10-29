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
	$(CONTENTDIR)/effect.cpp \
	$(CONTENTDIR)/wall_left.cpp \
	$(CONTENTDIR)/wall_right.cpp \
	$(CONTENTDIR)/wall_top.cpp \
	$(CONTENTDIR)/wall_bottom.cpp \
	$(CONTENTDIR)/wall_back.cpp \
	$(CONTENTDIR)/mask.cpp

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

$(CONTENTDIR)/effect.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./effect.pfx

$(CONTENTDIR)/wall_left.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./wall_left.pvr

$(CONTENTDIR)/wall_right.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./wall_right.pvr

$(CONTENTDIR)/wall_top.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./wall_top.pvr

$(CONTENTDIR)/wall_bottom.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./wall_bottom.pvr

$(CONTENTDIR)/wall_back.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./wall_back.pvr

$(CONTENTDIR)/mask.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./mask.pvr

############################################################################
# End of file (content.mak)
############################################################################

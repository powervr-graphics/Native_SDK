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
	$(CONTENTDIR)/tBridge.cpp \
	$(CONTENTDIR)/tGrass.cpp \
	$(CONTENTDIR)/tSkin.cpp \
	$(CONTENTDIR)/tWater.cpp \
	$(CONTENTDIR)/tCloud.cpp \
	$(CONTENTDIR)/tFur.cpp \
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

$(CONTENTDIR)/tBridge.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./tBridge.pvr

$(CONTENTDIR)/tGrass.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./tGrass.pvr

$(CONTENTDIR)/tSkin.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./tSkin.pvr

$(CONTENTDIR)/tWater.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./tWater.pvr

$(CONTENTDIR)/tCloud.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./tCloud.pvr

$(CONTENTDIR)/tFur.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./tFur.pvr

$(CONTENTDIR)/Scene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Scene.pod

############################################################################
# End of file (content.mak)
############################################################################

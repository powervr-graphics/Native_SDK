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
	$(CONTENTDIR)/Balloon.cpp \
	$(CONTENTDIR)/Balloon_pvr.cpp \
	$(CONTENTDIR)/Noise.cpp \
	$(CONTENTDIR)/Skybox.cpp \
	$(CONTENTDIR)/SkyboxMidnight.cpp \
	$(CONTENTDIR)/effects.cpp \
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

$(CONTENTDIR)/Balloon.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Balloon.pvr

$(CONTENTDIR)/Balloon_pvr.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Balloon_pvr.pvr

$(CONTENTDIR)/Noise.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Noise.pvr

$(CONTENTDIR)/Skybox.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Skybox.pvr

$(CONTENTDIR)/SkyboxMidnight.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./SkyboxMidnight.pvr

$(CONTENTDIR)/effects.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./effects.pfx

$(CONTENTDIR)/Scene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Scene.pod

############################################################################
# End of file (content.mak)
############################################################################

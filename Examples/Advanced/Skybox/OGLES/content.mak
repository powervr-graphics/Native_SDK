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
	$(CONTENTDIR)/balloon.cpp \
	$(CONTENTDIR)/skybox1.cpp \
	$(CONTENTDIR)/skybox2.cpp \
	$(CONTENTDIR)/skybox3.cpp \
	$(CONTENTDIR)/skybox4.cpp \
	$(CONTENTDIR)/skybox5.cpp \
	$(CONTENTDIR)/skybox6.cpp \
	$(CONTENTDIR)/HotAirBalloon.cpp

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

$(CONTENTDIR)/balloon.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./balloon.pvr

$(CONTENTDIR)/skybox1.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./skybox1.pvr

$(CONTENTDIR)/skybox2.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./skybox2.pvr

$(CONTENTDIR)/skybox3.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./skybox3.pvr

$(CONTENTDIR)/skybox4.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./skybox4.pvr

$(CONTENTDIR)/skybox5.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./skybox5.pvr

$(CONTENTDIR)/skybox6.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./skybox6.pvr

$(CONTENTDIR)/HotAirBalloon.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./HotAirBalloon.pod

############################################################################
# End of file (content.mak)
############################################################################

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
	$(CONTENTDIR)/IntroducingPOD.cpp \
	$(CONTENTDIR)/tex_base.cpp \
	$(CONTENTDIR)/tex_arm.cpp

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

$(CONTENTDIR)/IntroducingPOD.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./IntroducingPOD.pod

$(CONTENTDIR)/tex_base.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./tex_base.pvr

$(CONTENTDIR)/tex_arm.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./tex_arm.pvr

############################################################################
# End of file (content.mak)
############################################################################

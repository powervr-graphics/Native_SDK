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
	$(CONTENTDIR)/MaskMain.cpp \
	$(CONTENTDIR)/RoomStill.cpp \
	$(CONTENTDIR)/FragShader.cpp \
	$(CONTENTDIR)/SHVertShader.cpp \
	$(CONTENTDIR)/DiffuseVertShader.cpp \
	$(CONTENTDIR)/PhantomMask.cpp

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

$(CONTENTDIR)/MaskMain.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./MaskMain.pvr

$(CONTENTDIR)/RoomStill.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./RoomStill.pvr

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh
	-$(FILEWRAP)  -oa $@ ./FragShader.fsc

$(CONTENTDIR)/SHVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./SHVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./SHVertShader.vsc

$(CONTENTDIR)/DiffuseVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./DiffuseVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./DiffuseVertShader.vsc

$(CONTENTDIR)/PhantomMask.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./PhantomMask.pod

############################################################################
# End of file (content.mak)
############################################################################

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

$(CONTENTDIR)/MaskMain.cpp: $(CONTENTDIR) ./MaskMain.pvr
	$(FILEWRAP)  -o $@ ./MaskMain.pvr

$(CONTENTDIR)/RoomStill.cpp: $(CONTENTDIR) ./RoomStill.pvr
	$(FILEWRAP)  -o $@ ./RoomStill.pvr

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR) ./FragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh

$(CONTENTDIR)/SHVertShader.cpp: $(CONTENTDIR) ./SHVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./SHVertShader.vsh

$(CONTENTDIR)/DiffuseVertShader.cpp: $(CONTENTDIR) ./DiffuseVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./DiffuseVertShader.vsh

$(CONTENTDIR)/PhantomMask.cpp: $(CONTENTDIR) ./PhantomMask.pod
	$(FILEWRAP)  -o $@ ./PhantomMask.pod

############################################################################
# End of file (content.mak)
############################################################################

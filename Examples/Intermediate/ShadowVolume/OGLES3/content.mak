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
	$(CONTENTDIR)/BaseFragShader.cpp \
	$(CONTENTDIR)/BaseVertShader.cpp \
	$(CONTENTDIR)/ConstFragShader.cpp \
	$(CONTENTDIR)/ShadowVolVertShader.cpp \
	$(CONTENTDIR)/FullscreenVertShader.cpp \
	$(CONTENTDIR)/scene.cpp \
	$(CONTENTDIR)/Background.cpp \
	$(CONTENTDIR)/Rust.cpp

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

$(CONTENTDIR)/BaseFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./BaseFragShader.fsh

$(CONTENTDIR)/BaseVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./BaseVertShader.vsh

$(CONTENTDIR)/ConstFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./ConstFragShader.fsh

$(CONTENTDIR)/ShadowVolVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./ShadowVolVertShader.vsh

$(CONTENTDIR)/FullscreenVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./FullscreenVertShader.vsh

$(CONTENTDIR)/scene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./scene.pod

$(CONTENTDIR)/Background.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Background.pvr

$(CONTENTDIR)/Rust.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Rust.pvr

############################################################################
# End of file (content.mak)
############################################################################

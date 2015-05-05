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

$(CONTENTDIR)/BaseFragShader.cpp: $(CONTENTDIR) ./BaseFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./BaseFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./BaseFragShader.fsc

$(CONTENTDIR)/BaseVertShader.cpp: $(CONTENTDIR) ./BaseVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./BaseVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./BaseVertShader.vsc

$(CONTENTDIR)/ConstFragShader.cpp: $(CONTENTDIR) ./ConstFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./ConstFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./ConstFragShader.fsc

$(CONTENTDIR)/ShadowVolVertShader.cpp: $(CONTENTDIR) ./ShadowVolVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./ShadowVolVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./ShadowVolVertShader.vsc

$(CONTENTDIR)/FullscreenVertShader.cpp: $(CONTENTDIR) ./FullscreenVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./FullscreenVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./FullscreenVertShader.vsc

$(CONTENTDIR)/scene.cpp: $(CONTENTDIR) ./scene.pod
	$(FILEWRAP)  -o $@ ./scene.pod

$(CONTENTDIR)/Background.cpp: $(CONTENTDIR) ./Background.pvr
	$(FILEWRAP)  -o $@ ./Background.pvr

$(CONTENTDIR)/Rust.cpp: $(CONTENTDIR) ./Rust.pvr
	$(FILEWRAP)  -o $@ ./Rust.pvr

############################################################################
# End of file (content.mak)
############################################################################

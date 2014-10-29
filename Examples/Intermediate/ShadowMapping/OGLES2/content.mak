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
	$(CONTENTDIR)/FragShader.cpp \
	$(CONTENTDIR)/VertShader.cpp \
	$(CONTENTDIR)/ShadowFragShader.cpp \
	$(CONTENTDIR)/ShadowVertShader.cpp \
	$(CONTENTDIR)/Scene.cpp \
	$(CONTENTDIR)/Mask.cpp \
	$(CONTENTDIR)/TableCover.cpp \
	$(CONTENTDIR)/Torus.cpp

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

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh
	-$(FILEWRAP)  -oa $@ ./FragShader.fsc

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh
	-$(FILEWRAP)  -oa $@ ./VertShader.vsc

$(CONTENTDIR)/ShadowFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./ShadowFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./ShadowFragShader.fsc

$(CONTENTDIR)/ShadowVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./ShadowVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./ShadowVertShader.vsc

$(CONTENTDIR)/Scene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Scene.pod

$(CONTENTDIR)/Mask.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Mask.pvr

$(CONTENTDIR)/TableCover.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./TableCover.pvr

$(CONTENTDIR)/Torus.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Torus.pvr

############################################################################
# End of file (content.mak)
############################################################################

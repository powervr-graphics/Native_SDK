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
	$(CONTENTDIR)/PostBloomFragShader.cpp \
	$(CONTENTDIR)/PostBloomVertShader.cpp \
	$(CONTENTDIR)/PreBloomFragShader.cpp \
	$(CONTENTDIR)/PreBloomVertShader.cpp \
	$(CONTENTDIR)/BlurFragShader.cpp \
	$(CONTENTDIR)/BlurVertShader.cpp \
	$(CONTENTDIR)/FragShader.cpp \
	$(CONTENTDIR)/VertShader.cpp \
	$(CONTENTDIR)/Mask.cpp \
	$(CONTENTDIR)/BaseTex.cpp \
	$(CONTENTDIR)/bloom_mapping.cpp

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

$(CONTENTDIR)/PostBloomFragShader.cpp: $(CONTENTDIR) ./PostBloomFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./PostBloomFragShader.fsh

$(CONTENTDIR)/PostBloomVertShader.cpp: $(CONTENTDIR) ./PostBloomVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./PostBloomVertShader.vsh

$(CONTENTDIR)/PreBloomFragShader.cpp: $(CONTENTDIR) ./PreBloomFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./PreBloomFragShader.fsh

$(CONTENTDIR)/PreBloomVertShader.cpp: $(CONTENTDIR) ./PreBloomVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./PreBloomVertShader.vsh

$(CONTENTDIR)/BlurFragShader.cpp: $(CONTENTDIR) ./BlurFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./BlurFragShader.fsh

$(CONTENTDIR)/BlurVertShader.cpp: $(CONTENTDIR) ./BlurVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./BlurVertShader.vsh

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR) ./FragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR) ./VertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh

$(CONTENTDIR)/Mask.cpp: $(CONTENTDIR) ./Mask.pod
	$(FILEWRAP)  -o $@ ./Mask.pod

$(CONTENTDIR)/BaseTex.cpp: $(CONTENTDIR) ./BaseTex.pvr
	$(FILEWRAP)  -o $@ ./BaseTex.pvr

$(CONTENTDIR)/bloom_mapping.cpp: $(CONTENTDIR) ./bloom_mapping.pvr
	$(FILEWRAP)  -o $@ ./bloom_mapping.pvr

############################################################################
# End of file (content.mak)
############################################################################

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
	$(CONTENTDIR)/FastFragShader.cpp \
	$(CONTENTDIR)/FastVertShader.cpp \
	$(CONTENTDIR)/SlowFragShader.cpp \
	$(CONTENTDIR)/SlowVertShader.cpp \
	$(CONTENTDIR)/Mask.cpp \
	$(CONTENTDIR)/Basetex.cpp

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

$(CONTENTDIR)/FastFragShader.cpp: $(CONTENTDIR) ./FastFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./FastFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./FastFragShader.fsc

$(CONTENTDIR)/FastVertShader.cpp: $(CONTENTDIR) ./FastVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./FastVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./FastVertShader.vsc

$(CONTENTDIR)/SlowFragShader.cpp: $(CONTENTDIR) ./SlowFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./SlowFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./SlowFragShader.fsc

$(CONTENTDIR)/SlowVertShader.cpp: $(CONTENTDIR) ./SlowVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./SlowVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./SlowVertShader.vsc

$(CONTENTDIR)/Mask.cpp: $(CONTENTDIR) ./Mask.pod
	$(FILEWRAP)  -o $@ ./Mask.pod

$(CONTENTDIR)/Basetex.cpp: $(CONTENTDIR) ./Basetex.pvr
	$(FILEWRAP)  -o $@ ./Basetex.pvr

############################################################################
# End of file (content.mak)
############################################################################

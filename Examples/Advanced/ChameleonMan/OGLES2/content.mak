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
	$(CONTENTDIR)/skyline.cpp \
	$(CONTENTDIR)/Wall_diffuse_baked.cpp \
	$(CONTENTDIR)/Tang_space_BodyMap.cpp \
	$(CONTENTDIR)/Tang_space_LegsMap.cpp \
	$(CONTENTDIR)/Tang_space_BeltMap.cpp \
	$(CONTENTDIR)/FinalChameleonManLegs.cpp \
	$(CONTENTDIR)/FinalChameleonManHeadBody.cpp \
	$(CONTENTDIR)/lamp.cpp \
	$(CONTENTDIR)/ChameleonBelt.cpp \
	$(CONTENTDIR)/SkinnedVertShader.cpp \
	$(CONTENTDIR)/SkinnedFragShader.cpp \
	$(CONTENTDIR)/DefaultVertShader.cpp \
	$(CONTENTDIR)/DefaultFragShader.cpp \
	$(CONTENTDIR)/ChameleonScene.cpp

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

$(CONTENTDIR)/skyline.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./skyline.pvr

$(CONTENTDIR)/Wall_diffuse_baked.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Wall_diffuse_baked.pvr

$(CONTENTDIR)/Tang_space_BodyMap.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Tang_space_BodyMap.pvr

$(CONTENTDIR)/Tang_space_LegsMap.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Tang_space_LegsMap.pvr

$(CONTENTDIR)/Tang_space_BeltMap.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Tang_space_BeltMap.pvr

$(CONTENTDIR)/FinalChameleonManLegs.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./FinalChameleonManLegs.pvr

$(CONTENTDIR)/FinalChameleonManHeadBody.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./FinalChameleonManHeadBody.pvr

$(CONTENTDIR)/lamp.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./lamp.pvr

$(CONTENTDIR)/ChameleonBelt.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./ChameleonBelt.pvr

$(CONTENTDIR)/SkinnedVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./SkinnedVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./SkinnedVertShader.vsc

$(CONTENTDIR)/SkinnedFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./SkinnedFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./SkinnedFragShader.fsc

$(CONTENTDIR)/DefaultVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./DefaultVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./DefaultVertShader.vsc

$(CONTENTDIR)/DefaultFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./DefaultFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./DefaultFragShader.fsc

$(CONTENTDIR)/ChameleonScene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./ChameleonScene.pod

############################################################################
# End of file (content.mak)
############################################################################

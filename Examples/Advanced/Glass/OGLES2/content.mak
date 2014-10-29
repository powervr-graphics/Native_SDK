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
	$(CONTENTDIR)/DefaultVertShader.cpp \
	$(CONTENTDIR)/DefaultFragShader.cpp \
	$(CONTENTDIR)/ParaboloidVertShader.cpp \
	$(CONTENTDIR)/SkyboxVertShader.cpp \
	$(CONTENTDIR)/SkyboxFragShader.cpp \
	$(CONTENTDIR)/EffectVertShader.cpp \
	$(CONTENTDIR)/EffectFragShader.cpp \
	$(CONTENTDIR)/Balloon.cpp \
	$(CONTENTDIR)/Ball.cpp \
	$(CONTENTDIR)/BalloonTex.cpp \
	$(CONTENTDIR)/SkyboxTex.cpp

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

$(CONTENTDIR)/DefaultVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./DefaultVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./DefaultVertShader.vsc

$(CONTENTDIR)/DefaultFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./DefaultFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./DefaultFragShader.fsc

$(CONTENTDIR)/ParaboloidVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./ParaboloidVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./ParaboloidVertShader.vsc

$(CONTENTDIR)/SkyboxVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./SkyboxVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./SkyboxVertShader.vsc

$(CONTENTDIR)/SkyboxFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./SkyboxFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./SkyboxFragShader.fsc

$(CONTENTDIR)/EffectVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./EffectVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./EffectVertShader.vsc

$(CONTENTDIR)/EffectFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./EffectFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./EffectFragShader.fsc

$(CONTENTDIR)/Balloon.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Balloon.pod

$(CONTENTDIR)/Ball.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Ball.pod

$(CONTENTDIR)/BalloonTex.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./BalloonTex.pvr

$(CONTENTDIR)/SkyboxTex.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./SkyboxTex.pvr

############################################################################
# End of file (content.mak)
############################################################################

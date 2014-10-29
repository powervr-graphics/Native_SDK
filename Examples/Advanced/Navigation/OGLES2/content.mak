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
DATACONTENTDIR = ../Data/Content

#############################################################################
## Instructions
#############################################################################

RESOURCES = \
	$(CONTENTDIR)/StreetSigns.cpp \
	$(CONTENTDIR)/AlphaMaskFragShader.cpp \
	$(CONTENTDIR)/AntiAliasedLinesFragShader.cpp \
	$(CONTENTDIR)/AntiAliasedLinesVertShader.cpp \
	$(CONTENTDIR)/FragShader.cpp \
	$(CONTENTDIR)/VertShader.cpp \
	$(CONTENTDIR)/PivotQuadFragShader.cpp \
	$(CONTENTDIR)/PivotQuadMaskedFragShader.cpp \
	$(CONTENTDIR)/PivotQuadVertShader.cpp \
	$(DATACONTENTDIR)/Alphabet.cpp \
	$(DATACONTENTDIR)/Road.cpp \
	$(DATACONTENTDIR)/cameratrack.cpp \
	$(DATACONTENTDIR)/Landmark_meshes.cpp \
	$(DATACONTENTDIR)/LandUseA_meshes.cpp \
	$(DATACONTENTDIR)/LandUseB_meshes.cpp \
	$(DATACONTENTDIR)/MajHwys_meshes.cpp \
	$(DATACONTENTDIR)/MajHwyShield_text.cpp \
	$(DATACONTENTDIR)/RailRds_meshes.cpp \
	$(DATACONTENTDIR)/SecHwys_meshes.cpp \
	$(DATACONTENTDIR)/SecHwyShield_text.cpp \
	$(DATACONTENTDIR)/Signs_billboards.cpp \
	$(DATACONTENTDIR)/Streets_meshes.cpp \
	$(DATACONTENTDIR)/Streets_text.cpp \
	$(DATACONTENTDIR)/WaterPoly_meshes.cpp \
	$(DATACONTENTDIR)/WaterSeg_meshes.cpp

all: resources
	
help:
	@echo Valid targets are:
	@echo resources, clean
	@echo FILEWRAP can be used to override the default path to the Filewrap utility.

clean:
	@for i in $(RESOURCES); do test -f $$i && rm -vf $$i || true; done

resources: $(RESOURCES)

$(CONTENTDIR) $(DATACONTENTDIR):
	-mkdir "$@"

$(CONTENTDIR)/StreetSigns.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./StreetSigns.pvr

$(CONTENTDIR)/AlphaMaskFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./AlphaMaskFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./AlphaMaskFragShader.fsc

$(CONTENTDIR)/AntiAliasedLinesFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./AntiAliasedLinesFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./AntiAliasedLinesFragShader.fsc

$(CONTENTDIR)/AntiAliasedLinesVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./AntiAliasedLinesVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./AntiAliasedLinesVertShader.vsc

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh
	-$(FILEWRAP)  -oa $@ ./FragShader.fsc

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh
	-$(FILEWRAP)  -oa $@ ./VertShader.vsc

$(CONTENTDIR)/PivotQuadFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./PivotQuadFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./PivotQuadFragShader.fsc

$(CONTENTDIR)/PivotQuadMaskedFragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./PivotQuadMaskedFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./PivotQuadMaskedFragShader.fsc

$(CONTENTDIR)/PivotQuadVertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./PivotQuadVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./PivotQuadVertShader.vsc

$(DATACONTENTDIR)/Alphabet.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/Alphabet.pvr

$(DATACONTENTDIR)/Road.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/Road.pvr

$(DATACONTENTDIR)/cameratrack.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/cameratrack.pod

$(DATACONTENTDIR)/Landmark_meshes.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/Landmark_meshes.nav

$(DATACONTENTDIR)/LandUseA_meshes.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/LandUseA_meshes.nav

$(DATACONTENTDIR)/LandUseB_meshes.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/LandUseB_meshes.nav

$(DATACONTENTDIR)/MajHwys_meshes.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/MajHwys_meshes.nav

$(DATACONTENTDIR)/MajHwyShield_text.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/MajHwyShield_text.nav

$(DATACONTENTDIR)/RailRds_meshes.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/RailRds_meshes.nav

$(DATACONTENTDIR)/SecHwys_meshes.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/SecHwys_meshes.nav

$(DATACONTENTDIR)/SecHwyShield_text.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/SecHwyShield_text.nav

$(DATACONTENTDIR)/Signs_billboards.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/Signs_billboards.nav

$(DATACONTENTDIR)/Streets_meshes.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/Streets_meshes.nav

$(DATACONTENTDIR)/Streets_text.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/Streets_text.nav

$(DATACONTENTDIR)/WaterPoly_meshes.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/WaterPoly_meshes.nav

$(DATACONTENTDIR)/WaterSeg_meshes.cpp: $(DATACONTENTDIR)
	$(FILEWRAP)  -o $@ ../Data/WaterSeg_meshes.nav

############################################################################
# End of file (content.mak)
############################################################################

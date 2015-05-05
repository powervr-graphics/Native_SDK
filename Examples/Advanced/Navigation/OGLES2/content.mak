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

$(CONTENTDIR)/StreetSigns.cpp: $(CONTENTDIR) ./StreetSigns.pvr
	$(FILEWRAP)  -o $@ ./StreetSigns.pvr

$(CONTENTDIR)/AlphaMaskFragShader.cpp: $(CONTENTDIR) ./AlphaMaskFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./AlphaMaskFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./AlphaMaskFragShader.fsc

$(CONTENTDIR)/AntiAliasedLinesFragShader.cpp: $(CONTENTDIR) ./AntiAliasedLinesFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./AntiAliasedLinesFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./AntiAliasedLinesFragShader.fsc

$(CONTENTDIR)/AntiAliasedLinesVertShader.cpp: $(CONTENTDIR) ./AntiAliasedLinesVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./AntiAliasedLinesVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./AntiAliasedLinesVertShader.vsc

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR) ./FragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh
	-$(FILEWRAP)  -oa $@ ./FragShader.fsc

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR) ./VertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh
	-$(FILEWRAP)  -oa $@ ./VertShader.vsc

$(CONTENTDIR)/PivotQuadFragShader.cpp: $(CONTENTDIR) ./PivotQuadFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./PivotQuadFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./PivotQuadFragShader.fsc

$(CONTENTDIR)/PivotQuadMaskedFragShader.cpp: $(CONTENTDIR) ./PivotQuadMaskedFragShader.fsh
	$(FILEWRAP)  -s  -o $@ ./PivotQuadMaskedFragShader.fsh
	-$(FILEWRAP)  -oa $@ ./PivotQuadMaskedFragShader.fsc

$(CONTENTDIR)/PivotQuadVertShader.cpp: $(CONTENTDIR) ./PivotQuadVertShader.vsh
	$(FILEWRAP)  -s  -o $@ ./PivotQuadVertShader.vsh
	-$(FILEWRAP)  -oa $@ ./PivotQuadVertShader.vsc

$(DATACONTENTDIR)/Alphabet.cpp: $(DATACONTENTDIR) ../Data/Alphabet.pvr
	$(FILEWRAP)  -o $@ ../Data/Alphabet.pvr

$(DATACONTENTDIR)/Road.cpp: $(DATACONTENTDIR) ../Data/Road.pvr
	$(FILEWRAP)  -o $@ ../Data/Road.pvr

$(DATACONTENTDIR)/cameratrack.cpp: $(DATACONTENTDIR) ../Data/cameratrack.pod
	$(FILEWRAP)  -o $@ ../Data/cameratrack.pod

$(DATACONTENTDIR)/Landmark_meshes.cpp: $(DATACONTENTDIR) ../Data/Landmark_meshes.nav
	$(FILEWRAP)  -o $@ ../Data/Landmark_meshes.nav

$(DATACONTENTDIR)/LandUseA_meshes.cpp: $(DATACONTENTDIR) ../Data/LandUseA_meshes.nav
	$(FILEWRAP)  -o $@ ../Data/LandUseA_meshes.nav

$(DATACONTENTDIR)/LandUseB_meshes.cpp: $(DATACONTENTDIR) ../Data/LandUseB_meshes.nav
	$(FILEWRAP)  -o $@ ../Data/LandUseB_meshes.nav

$(DATACONTENTDIR)/MajHwys_meshes.cpp: $(DATACONTENTDIR) ../Data/MajHwys_meshes.nav
	$(FILEWRAP)  -o $@ ../Data/MajHwys_meshes.nav

$(DATACONTENTDIR)/MajHwyShield_text.cpp: $(DATACONTENTDIR) ../Data/MajHwyShield_text.nav
	$(FILEWRAP)  -o $@ ../Data/MajHwyShield_text.nav

$(DATACONTENTDIR)/RailRds_meshes.cpp: $(DATACONTENTDIR) ../Data/RailRds_meshes.nav
	$(FILEWRAP)  -o $@ ../Data/RailRds_meshes.nav

$(DATACONTENTDIR)/SecHwys_meshes.cpp: $(DATACONTENTDIR) ../Data/SecHwys_meshes.nav
	$(FILEWRAP)  -o $@ ../Data/SecHwys_meshes.nav

$(DATACONTENTDIR)/SecHwyShield_text.cpp: $(DATACONTENTDIR) ../Data/SecHwyShield_text.nav
	$(FILEWRAP)  -o $@ ../Data/SecHwyShield_text.nav

$(DATACONTENTDIR)/Signs_billboards.cpp: $(DATACONTENTDIR) ../Data/Signs_billboards.nav
	$(FILEWRAP)  -o $@ ../Data/Signs_billboards.nav

$(DATACONTENTDIR)/Streets_meshes.cpp: $(DATACONTENTDIR) ../Data/Streets_meshes.nav
	$(FILEWRAP)  -o $@ ../Data/Streets_meshes.nav

$(DATACONTENTDIR)/Streets_text.cpp: $(DATACONTENTDIR) ../Data/Streets_text.nav
	$(FILEWRAP)  -o $@ ../Data/Streets_text.nav

$(DATACONTENTDIR)/WaterPoly_meshes.cpp: $(DATACONTENTDIR) ../Data/WaterPoly_meshes.nav
	$(FILEWRAP)  -o $@ ../Data/WaterPoly_meshes.nav

$(DATACONTENTDIR)/WaterSeg_meshes.cpp: $(DATACONTENTDIR) ../Data/WaterSeg_meshes.nav
	$(FILEWRAP)  -o $@ ../Data/WaterSeg_meshes.nav

############################################################################
# End of file (content.mak)
############################################################################

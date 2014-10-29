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
DATACONTENTDIR = ../Data/Content

#############################################################################
## Instructions
#############################################################################

RESOURCES = \
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

$(DATACONTENTDIR):
	-mkdir "$@"

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

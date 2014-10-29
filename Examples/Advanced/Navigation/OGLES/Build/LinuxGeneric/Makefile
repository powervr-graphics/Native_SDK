#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESNavigation
APIS    = OGLES
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLESNavigation.o	\
           Alphabet.o	\
           Road.o	\
           cameratrack.o	\
           Landmark_meshes.o	\
           LandUseA_meshes.o	\
           LandUseB_meshes.o	\
           MajHwys_meshes.o	\
           MajHwyShield_text.o	\
           RailRds_meshes.o	\
           SecHwys_meshes.o	\
           SecHwyShield_text.o	\
           Signs_billboards.o	\
           Streets_meshes.o	\
           Streets_text.o	\
           WaterPoly_meshes.o	\
           WaterSeg_meshes.o

VPATH +=	\
         ../../../Data/Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../../Data/Content/Alphabet.cpp ../../../Data/Content/Road.cpp ../../../Data/Content/cameratrack.cpp ../../../Data/Content/Landmark_meshes.cpp ../../../Data/Content/LandUseA_meshes.cpp ../../../Data/Content/LandUseB_meshes.cpp ../../../Data/Content/MajHwys_meshes.cpp ../../../Data/Content/MajHwyShield_text.cpp ../../../Data/Content/RailRds_meshes.cpp ../../../Data/Content/SecHwys_meshes.cpp ../../../Data/Content/SecHwyShield_text.cpp ../../../Data/Content/Signs_billboards.cpp ../../../Data/Content/Streets_meshes.cpp ../../../Data/Content/Streets_text.cpp ../../../Data/Content/WaterPoly_meshes.cpp ../../../Data/Content/WaterSeg_meshes.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../../Data/Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


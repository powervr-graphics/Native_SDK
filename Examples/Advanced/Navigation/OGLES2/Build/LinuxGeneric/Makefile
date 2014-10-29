#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2Navigation
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2Navigation.o	\
           StreetSigns.o	\
           AlphaMaskFragShader.o	\
           AntiAliasedLinesFragShader.o	\
           AntiAliasedLinesVertShader.o	\
           FragShader.o	\
           VertShader.o	\
           PivotQuadFragShader.o	\
           PivotQuadMaskedFragShader.o	\
           PivotQuadVertShader.o	\
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
         ../../Content : 	\
         ../../../Data/Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/StreetSigns.cpp ../../Content/AlphaMaskFragShader.cpp ../../Content/AntiAliasedLinesFragShader.cpp ../../Content/AntiAliasedLinesVertShader.cpp ../../Content/FragShader.cpp ../../Content/VertShader.cpp ../../Content/PivotQuadFragShader.cpp ../../Content/PivotQuadMaskedFragShader.cpp ../../Content/PivotQuadVertShader.cpp ../../../Data/Content/Alphabet.cpp ../../../Data/Content/Road.cpp ../../../Data/Content/cameratrack.cpp ../../../Data/Content/Landmark_meshes.cpp ../../../Data/Content/LandUseA_meshes.cpp ../../../Data/Content/LandUseB_meshes.cpp ../../../Data/Content/MajHwys_meshes.cpp ../../../Data/Content/MajHwyShield_text.cpp ../../../Data/Content/RailRds_meshes.cpp ../../../Data/Content/SecHwys_meshes.cpp ../../../Data/Content/SecHwyShield_text.cpp ../../../Data/Content/Signs_billboards.cpp ../../../Data/Content/Streets_meshes.cpp ../../../Data/Content/Streets_text.cpp ../../../Data/Content/WaterPoly_meshes.cpp ../../../Data/Content/WaterSeg_meshes.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@

$(PLAT_OBJPATH)/%.o: ../../../Data/Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


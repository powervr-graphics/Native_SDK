#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESNavigation3D
APIS    = OGLES
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLESNavigation3D.o	\
           skybox1.o	\
           skybox2.o	\
           skybox3.o	\
           skybox4.o	\
           skybox5.o	\
           skybox6.o	\
           modelindex.o	\
           occlusiondata.o	\
           cameratrack.o	\
           UIElements.o	\
           CM_US_IL_CHICAGO_008_015_L.DAE.o	\
           CM_US_IL_CHICAGO_008_016_L.DAE.o	\
           CM_US_IL_CHICAGO_009_014_L.DAE.o	\
           CM_US_IL_CHICAGO_009_015_L.DAE.o	\
           CM_US_IL_CHICAGO_009_016_L.DAE.o	\
           CM_US_IL_CHICAGO_009_017_L.DAE.o	\
           CM_US_IL_CHICAGO_009_018_L.DAE.o	\
           CM_US_IL_CHICAGO_010_014_L.DAE.o	\
           CM_US_IL_CHICAGO_010_015_L.DAE.o	\
           CM_US_IL_CHICAGO_010_015_H.DAE.o	\
           CM_US_IL_CHICAGO_010_016_H.DAE.o	\
           CM_US_IL_CHICAGO_010_016_L.DAE.o	\
           CM_US_IL_CHICAGO_010_017_L.DAE.o	\
           CM_US_IL_CHICAGO_010_017_H.DAE.o	\
           CM_US_IL_CHICAGO_010_018_L.DAE.o	\
           CM_US_IL_CHICAGO_011_013_L.DAE.o	\
           CM_US_IL_CHICAGO_011_014_L.DAE.o	\
           CM_US_IL_CHICAGO_011_015_L.DAE.o	\
           CM_US_IL_CHICAGO_011_015_H.DAE.o	\
           CM_US_IL_CHICAGO_011_016_H.DAE.o	\
           CM_US_IL_CHICAGO_011_016_L.DAE.o	\
           CM_US_IL_CHICAGO_011_017_L.DAE.o	\
           CM_US_IL_CHICAGO_011_017_H.DAE.o	\
           CM_US_IL_CHICAGO_011_018_L.DAE.o	\
           CM_US_IL_CHICAGO_012_014_L.DAE.o	\
           CM_US_IL_CHICAGO_012_015_L.DAE.o	\
           CM_US_IL_CHICAGO_012_016_L.DAE.o	\
           CM_US_IL_CHICAGO_012_017_L.DAE.o	\
           CM_US_IL_CHICAGO_013_015_L.DAE.o	\
           CM_US_IL_CHICAGO_013_016_L.DAE.o	\
           006_RUS.PNG.o	\
           007_RUG.PNG.o	\
           008_RUG.PNG.o	\
           009_RUG.PNG.o	\
           011_GIE.PNG.o	\
           012_RSR.PNG.o	\
           016_FOC.PNG.o	\
           016_RTR.PNG.o	\
           017_FOD.PNG.o	\
           018_FOD.PNG.o	\
           019_FOC.PNG.o	\
           019_GOC.PNG.o	\
           019_RZG.PNG.o	\
           020_FOC.PNG.o	\
           021_FOC.PNG.o	\
           022_FOC.PNG.o	\
           022_RUG.PNG.o	\
           023_FOB.PNG.o	\
           023_RUG.PNG.o	\
           024_FOB.PNG.o	\
           025_FOC.PNG.o	\
           025_RUW.PNG.o	\
           026_FOD.PNG.o	\
           026_RUW.PNG.o	\
           027_FOD.PNG.o	\
           027_RUW.PNG.o	\
           028_GOF.PNG.o	\
           029_GCC.PNG.o	\
           030_GOC.PNG.o	\
           031_GOD.PNG.o	\
           032_FOC.PNG.o	\
           032_GOC.PNG.o	\
           033_FOA.PNG.o	\
           033_GOA.PNG.o	\
           034_FOC.PNG.o	\
           034_GOC.PNG.o	\
           035_FOC.PNG.o	\
           035_GOC.PNG.o	\
           036_FOC.PNG.o	\
           036_GOC.PNG.o	\
           037_FOC.PNG.o	\
           037_GOC.PNG.o	\
           041_FRB.PNG.o	\
           041_GRB.PNG.o	\
           044_GRC.PNG.o	\
           046_GRC.PNG.o	\
           054_GRC.PNG.o	\
           055_GRC.PNG.o	\
           056_GRC.PNG.o	\
           059_FRC.PNG.o	\
           060_FRC.PNG.o	\
           061_FRD.PNG.o	\
           063_GRC.PNG.o	\
           064_GRC.PNG.o	\
           066_FCB.PNG.o	\
           066_GCB.PNG.o	\
           067_FCC.PNG.o	\
           067_GCC.PNG.o	\
           068_GCD.PNG.o	\
           069_FCA.PNG.o	\
           069_GCA.PNG.o	\
           070_GOD.PNG.o	\
           071_FRC.PNG.o	\
           072_FRC.PNG.o	\
           073_FRC.PNG.o	\
           074_FRC.PNG.o	\
           075_FRC.PNG.o	\
           076_FRC.PNG.o	\
           077_FRC.PNG.o	\
           080_GCB.PNG.o	\
           082_FCD.PNG.o	\
           083_FRC.PNG.o	\
           085_GRC.PNG.o	\
           086_FOF.PNG.o	\
           086_GOF.PNG.o	\
           087_FCA.PNG.o	\
           087_GCA.PNG.o	\
           087_GCC.PNG.o	\
           088_FRC.PNG.o	\
           089_FRC.PNG.o	\
           092_GCA.PNG.o	\
           094_FOD.PNG.o	\
           095_FOD.PNG.o	\
           US_IL_13443_CHICAGO_35EAST_L.PNG.o	\
           US_IL_13444_CHICAGO_LEOBURNETT_L.PNG.o	\
           US_IL_13447_CHICAGO_REIDMURDOCH_L.PNG.o	\
           US_IL_13448_CHICAGO_CARBIDE_L.PNG.o	\
           US_IL_13449_CHICAGO_CROWNFOUNTAIN_L.PNG.o	\
           US_IL_13451_CHICAGO_CULTURAL_L.PNG.o	\
           US_IL_13453_CHICAGO_PRUDENTIAL_PART1_L.PNG.o	\
           US_IL_13454_CHICAGO_UNITED_L.PNG.o	\
           US_IL_13456_CHICAGO_SEVENTEENTH_L.PNG.o	\
           US_IL_13458_CHICAGO_SMURFIT_L.PNG.o	\
           US_IL_13459_CHICAGO_LASALLE_L.PNG.o	\
           US_IL_13460_CHICAGO_TRUMP_L.PNG.o	\
           US_IL_13461_CHICAGO_UNITRIN_L.PNG.o	\
           US_IL_13462_CHICAGO_WILLOUGHBY_L.PNG.o	\
           US_IL_13490_CHICAGO_PRUDENTIAL_PART2_L.PNG.o	\
           US_IL_CHICAGO_AONCENTER_L.PNG.o	\
           US_IL_CHICAGO_ARTINSTITUTE_L.PNG.o	\
           US_IL_CHICAGO_BOARDOFTHETRADE_L.PNG.o	\
           US_IL_CHICAGO_BOEINGBUILDING_L.PNG.o	\
           US_IL_CHICAGO_CHICAGOTHEATRE_L.PNG.o	\
           US_IL_CHICAGO_CITYHALL_L.PNG.o	\
           US_IL_CHICAGO_DALEY_L.PNG.o	\
           US_IL_CHICAGO_HILTON_L.PNG.o	\
           US_IL_CHICAGO_JAMESTHOMPSON_L.PNG.o	\
           US_IL_CHICAGO_LIBRARY_L.PNG.o	\
           US_IL_CHICAGO_MILLENIUMPARK1_L.PNG.o	\
           US_IL_CHICAGO_MILLENIUMPARK2_L.PNG.o	\
           US_IL_CHICAGO_MMART_L.PNG.o	\
           US_IL_CHICAGO_OGILVIE_L.PNG.o	\
           US_IL_CHICAGO_SEARSTOWER_L.PNG.o	\
           US_IL_CHICAGO_UNIONSTATION_L.PNG.o	\
           US_L_CONCRETE-COLOUR.PNG.o	\
           US_L_CONCRETE-DETAIL.PNG.o	\
           US_L_PARK-COLOUR.PNG.o	\
           US_L_WATER-COLOUR.PNG.o	\
           US_R_CONCRETE.PNG.o	\
           US_R_HIGHWAY-SOLID.PNG.o	\
           US_R_STREET-DASHED.PNG.o	\
           US_R_STREET-INNER-SHOULDER.PNG.o	\
           US_R_STREET-LANE-FILLER.PNG.o	\
           US_R_STREET-SOLID.PNG.o	\
           US_R_STREET-UNMARKED.PNG.o	\
           US_R_WALKWAY-SOLID.PNG.o	\
           US_R_WALKWAY-UNMARKED.PNG.o	\
           US_T_RAILROAD.PNG.o

VPATH +=	\
         ../../Content : 	\
         ../../../Data/Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/skybox1.cpp ../../Content/skybox2.cpp ../../Content/skybox3.cpp ../../Content/skybox4.cpp ../../Content/skybox5.cpp ../../Content/skybox6.cpp ../../../Data/Content/modelindex.cpp ../../../Data/Content/occlusiondata.cpp ../../../Data/Content/cameratrack.cpp ../../../Data/Content/UIElements.cpp ../../../Data/Content/CM_US_IL_CHICAGO_008_015_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_008_016_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_009_014_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_009_015_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_009_016_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_009_017_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_009_018_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_010_014_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_010_015_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_010_015_H.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_010_016_H.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_010_016_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_010_017_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_010_017_H.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_010_018_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_011_013_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_011_014_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_011_015_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_011_015_H.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_011_016_H.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_011_016_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_011_017_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_011_017_H.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_011_018_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_012_014_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_012_015_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_012_016_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_012_017_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_013_015_L.DAE.cpp ../../../Data/Content/CM_US_IL_CHICAGO_013_016_L.DAE.cpp ../../../Data/Content/006_RUS.PNG.cpp ../../../Data/Content/007_RUG.PNG.cpp ../../../Data/Content/008_RUG.PNG.cpp ../../../Data/Content/009_RUG.PNG.cpp ../../../Data/Content/011_GIE.PNG.cpp ../../../Data/Content/012_RSR.PNG.cpp ../../../Data/Content/016_FOC.PNG.cpp ../../../Data/Content/016_RTR.PNG.cpp ../../../Data/Content/017_FOD.PNG.cpp ../../../Data/Content/018_FOD.PNG.cpp ../../../Data/Content/019_FOC.PNG.cpp ../../../Data/Content/019_GOC.PNG.cpp ../../../Data/Content/019_RZG.PNG.cpp ../../../Data/Content/020_FOC.PNG.cpp ../../../Data/Content/021_FOC.PNG.cpp ../../../Data/Content/022_FOC.PNG.cpp ../../../Data/Content/022_RUG.PNG.cpp ../../../Data/Content/023_FOB.PNG.cpp ../../../Data/Content/023_RUG.PNG.cpp ../../../Data/Content/024_FOB.PNG.cpp ../../../Data/Content/025_FOC.PNG.cpp ../../../Data/Content/025_RUW.PNG.cpp ../../../Data/Content/026_FOD.PNG.cpp ../../../Data/Content/026_RUW.PNG.cpp ../../../Data/Content/027_FOD.PNG.cpp ../../../Data/Content/027_RUW.PNG.cpp ../../../Data/Content/028_GOF.PNG.cpp ../../../Data/Content/029_GCC.PNG.cpp ../../../Data/Content/030_GOC.PNG.cpp ../../../Data/Content/031_GOD.PNG.cpp ../../../Data/Content/032_FOC.PNG.cpp ../../../Data/Content/032_GOC.PNG.cpp ../../../Data/Content/033_FOA.PNG.cpp ../../../Data/Content/033_GOA.PNG.cpp ../../../Data/Content/034_FOC.PNG.cpp ../../../Data/Content/034_GOC.PNG.cpp ../../../Data/Content/035_FOC.PNG.cpp ../../../Data/Content/035_GOC.PNG.cpp ../../../Data/Content/036_FOC.PNG.cpp ../../../Data/Content/036_GOC.PNG.cpp ../../../Data/Content/037_FOC.PNG.cpp ../../../Data/Content/037_GOC.PNG.cpp ../../../Data/Content/041_FRB.PNG.cpp ../../../Data/Content/041_GRB.PNG.cpp ../../../Data/Content/044_GRC.PNG.cpp ../../../Data/Content/046_GRC.PNG.cpp ../../../Data/Content/054_GRC.PNG.cpp ../../../Data/Content/055_GRC.PNG.cpp ../../../Data/Content/056_GRC.PNG.cpp ../../../Data/Content/059_FRC.PNG.cpp ../../../Data/Content/060_FRC.PNG.cpp ../../../Data/Content/061_FRD.PNG.cpp ../../../Data/Content/063_GRC.PNG.cpp ../../../Data/Content/064_GRC.PNG.cpp ../../../Data/Content/066_FCB.PNG.cpp ../../../Data/Content/066_GCB.PNG.cpp ../../../Data/Content/067_FCC.PNG.cpp ../../../Data/Content/067_GCC.PNG.cpp ../../../Data/Content/068_GCD.PNG.cpp ../../../Data/Content/069_FCA.PNG.cpp ../../../Data/Content/069_GCA.PNG.cpp ../../../Data/Content/070_GOD.PNG.cpp ../../../Data/Content/071_FRC.PNG.cpp ../../../Data/Content/072_FRC.PNG.cpp ../../../Data/Content/073_FRC.PNG.cpp ../../../Data/Content/074_FRC.PNG.cpp ../../../Data/Content/075_FRC.PNG.cpp ../../../Data/Content/076_FRC.PNG.cpp ../../../Data/Content/077_FRC.PNG.cpp ../../../Data/Content/080_GCB.PNG.cpp ../../../Data/Content/082_FCD.PNG.cpp ../../../Data/Content/083_FRC.PNG.cpp ../../../Data/Content/085_GRC.PNG.cpp ../../../Data/Content/086_FOF.PNG.cpp ../../../Data/Content/086_GOF.PNG.cpp ../../../Data/Content/087_FCA.PNG.cpp ../../../Data/Content/087_GCA.PNG.cpp ../../../Data/Content/087_GCC.PNG.cpp ../../../Data/Content/088_FRC.PNG.cpp ../../../Data/Content/089_FRC.PNG.cpp ../../../Data/Content/092_GCA.PNG.cpp ../../../Data/Content/094_FOD.PNG.cpp ../../../Data/Content/095_FOD.PNG.cpp ../../../Data/Content/US_IL_13443_CHICAGO_35EAST_L.PNG.cpp ../../../Data/Content/US_IL_13444_CHICAGO_LEOBURNETT_L.PNG.cpp ../../../Data/Content/US_IL_13447_CHICAGO_REIDMURDOCH_L.PNG.cpp ../../../Data/Content/US_IL_13448_CHICAGO_CARBIDE_L.PNG.cpp ../../../Data/Content/US_IL_13449_CHICAGO_CROWNFOUNTAIN_L.PNG.cpp ../../../Data/Content/US_IL_13451_CHICAGO_CULTURAL_L.PNG.cpp ../../../Data/Content/US_IL_13453_CHICAGO_PRUDENTIAL_PART1_L.PNG.cpp ../../../Data/Content/US_IL_13454_CHICAGO_UNITED_L.PNG.cpp ../../../Data/Content/US_IL_13456_CHICAGO_SEVENTEENTH_L.PNG.cpp ../../../Data/Content/US_IL_13458_CHICAGO_SMURFIT_L.PNG.cpp ../../../Data/Content/US_IL_13459_CHICAGO_LASALLE_L.PNG.cpp ../../../Data/Content/US_IL_13460_CHICAGO_TRUMP_L.PNG.cpp ../../../Data/Content/US_IL_13461_CHICAGO_UNITRIN_L.PNG.cpp ../../../Data/Content/US_IL_13462_CHICAGO_WILLOUGHBY_L.PNG.cpp ../../../Data/Content/US_IL_13490_CHICAGO_PRUDENTIAL_PART2_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_AONCENTER_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_ARTINSTITUTE_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_BOARDOFTHETRADE_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_BOEINGBUILDING_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_CHICAGOTHEATRE_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_CITYHALL_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_DALEY_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_HILTON_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_JAMESTHOMPSON_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_LIBRARY_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_MILLENIUMPARK1_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_MILLENIUMPARK2_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_MMART_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_OGILVIE_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_SEARSTOWER_L.PNG.cpp ../../../Data/Content/US_IL_CHICAGO_UNIONSTATION_L.PNG.cpp ../../../Data/Content/US_L_CONCRETE-COLOUR.PNG.cpp ../../../Data/Content/US_L_CONCRETE-DETAIL.PNG.cpp ../../../Data/Content/US_L_PARK-COLOUR.PNG.cpp ../../../Data/Content/US_L_WATER-COLOUR.PNG.cpp ../../../Data/Content/US_R_CONCRETE.PNG.cpp ../../../Data/Content/US_R_HIGHWAY-SOLID.PNG.cpp ../../../Data/Content/US_R_STREET-DASHED.PNG.cpp ../../../Data/Content/US_R_STREET-INNER-SHOULDER.PNG.cpp ../../../Data/Content/US_R_STREET-LANE-FILLER.PNG.cpp ../../../Data/Content/US_R_STREET-SOLID.PNG.cpp ../../../Data/Content/US_R_STREET-UNMARKED.PNG.cpp ../../../Data/Content/US_R_WALKWAY-SOLID.PNG.cpp ../../../Data/Content/US_R_WALKWAY-UNMARKED.PNG.cpp ../../../Data/Content/US_T_RAILROAD.PNG.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@

$(PLAT_OBJPATH)/%.o: ../../../Data/Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


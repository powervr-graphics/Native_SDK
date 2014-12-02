#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2ChameleonMan
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2ChameleonMan.o	\
           skyline.o	\
           Wall_diffuse_baked.o	\
           Tang_space_BodyMap.o	\
           Tang_space_LegsMap.o	\
           Tang_space_BeltMap.o	\
           FinalChameleonManLegs.o	\
           FinalChameleonManHeadBody.o	\
           lamp.o	\
           ChameleonBelt.o	\
           SkinnedVertShader.o	\
           SkinnedFragShader.o	\
           DefaultVertShader.o	\
           DefaultFragShader.o	\
           ChameleonScene.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/skyline.cpp ../../Content/Wall_diffuse_baked.cpp ../../Content/Tang_space_BodyMap.cpp ../../Content/Tang_space_LegsMap.cpp ../../Content/Tang_space_BeltMap.cpp ../../Content/FinalChameleonManLegs.cpp ../../Content/FinalChameleonManHeadBody.cpp ../../Content/lamp.cpp ../../Content/ChameleonBelt.cpp ../../Content/SkinnedVertShader.cpp ../../Content/SkinnedFragShader.cpp ../../Content/DefaultVertShader.cpp ../../Content/DefaultFragShader.cpp ../../Content/ChameleonScene.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


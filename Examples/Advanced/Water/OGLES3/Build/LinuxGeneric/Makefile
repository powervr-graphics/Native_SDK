#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES3Water
APIS    = OGLES3
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES3Water.o	\
           normalmap.o	\
           skybox.o	\
           galleon.o	\
           wood.o	\
           coins.o	\
           coins-specular.o	\
           flag.o	\
           crate.o	\
           galleon-sails.o	\
           sand.o	\
           palmleaf.o	\
           FragShader.o	\
           VertShader.o	\
           SkyboxFShader.o	\
           SkyboxVShader.o	\
           ModelFShader.o	\
           ModelVShader.o	\
           Tex2DFShader.o	\
           Tex2DVShader.o	\
           PlaneTexFShader.o	\
           PlaneTexVShader.o	\
           Scene.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/normalmap.cpp ../../Content/skybox.cpp ../../Content/galleon.cpp ../../Content/wood.cpp ../../Content/coins.cpp ../../Content/coins-specular.cpp ../../Content/flag.cpp ../../Content/crate.cpp ../../Content/galleon-sails.cpp ../../Content/sand.cpp ../../Content/palmleaf.cpp ../../Content/FragShader.cpp ../../Content/VertShader.cpp ../../Content/SkyboxFShader.cpp ../../Content/SkyboxVShader.cpp ../../Content/ModelFShader.cpp ../../Content/ModelVShader.cpp ../../Content/Tex2DFShader.cpp ../../Content/Tex2DVShader.cpp ../../Content/PlaneTexFShader.cpp ../../Content/PlaneTexVShader.cpp ../../Content/Scene.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


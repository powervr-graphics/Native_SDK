#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2DeferredShading
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2DeferredShading.o	\
           effect.o	\
           scene.o	\
           pointlight.o	\
           light_cubemap.o	\
           mask_texture.o	\
           mask_bump.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/effect.cpp ../../Content/scene.cpp ../../Content/pointlight.cpp ../../Content/light_cubemap.cpp ../../Content/mask_texture.cpp ../../Content/mask_bump.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2Skybox2
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2Skybox2.o	\
           Balloon.o	\
           Balloon_pvr.o	\
           Noise.o	\
           Skybox.o	\
           SkyboxMidnight.o	\
           effects.o	\
           Scene.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/Balloon.cpp ../../Content/Balloon_pvr.cpp ../../Content/Noise.cpp ../../Content/Skybox.cpp ../../Content/SkyboxMidnight.cpp ../../Content/effects.cpp ../../Content/Scene.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


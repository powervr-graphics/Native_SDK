#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESFilmTV
APIS    = OGLES
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLESFilmTV.o	\
           Table.o	\
           Floor.o	\
           Wall.o	\
           TV.o	\
           TVCase.o	\
           TVSpeaker.o	\
           Alum.o	\
           Skirting.o	\
           Camera.o	\
           FilmTVScene.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/Table.cpp ../../Content/Floor.cpp ../../Content/Wall.cpp ../../Content/TV.cpp ../../Content/TVCase.cpp ../../Content/TVSpeaker.cpp ../../Content/Alum.cpp ../../Content/Skirting.cpp ../../Content/Camera.cpp ../../Content/FilmTVScene.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


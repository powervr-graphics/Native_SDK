#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESSkybox
APIS    = OGLES
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLESSkybox.o	\
           balloon.o	\
           skybox1.o	\
           skybox2.o	\
           skybox3.o	\
           skybox4.o	\
           skybox5.o	\
           skybox6.o	\
           HotAirBalloon.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/balloon.cpp ../../Content/skybox1.cpp ../../Content/skybox2.cpp ../../Content/skybox3.cpp ../../Content/skybox4.cpp ../../Content/skybox5.cpp ../../Content/skybox6.cpp ../../Content/HotAirBalloon.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


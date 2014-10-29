#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES3IntroducingPrint3D
APIS    = OGLES3
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES3IntroducingPrint3D.o	\
           Text.o	\
           arial_36.o	\
           starjout_60.o	\
           title_36.o	\
           title_46.o	\
           title_56.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/Text.cpp ../../Content/arial_36.cpp ../../Content/starjout_60.cpp ../../Content/title_36.cpp ../../Content/title_46.cpp ../../Content/title_56.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


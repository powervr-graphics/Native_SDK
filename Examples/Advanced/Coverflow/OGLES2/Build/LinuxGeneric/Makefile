#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2Coverflow
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2Coverflow.o	\
           Album1.o	\
           Album2.o	\
           Album3.o	\
           Album4.o	\
           Album5.o	\
           Album6.o	\
           Album7.o	\
           Album8.o	\
           Album9.o	\
           Album10.o	\
           Album11.o	\
           Album12.o	\
           Album13.o	\
           Album14.o	\
           Album15.o	\
           Album16.o	\
           FragShader.o	\
           VertShader.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/Album1.cpp ../../Content/Album2.cpp ../../Content/Album3.cpp ../../Content/Album4.cpp ../../Content/Album5.cpp ../../Content/Album6.cpp ../../Content/Album7.cpp ../../Content/Album8.cpp ../../Content/Album9.cpp ../../Content/Album10.cpp ../../Content/Album11.cpp ../../Content/Album12.cpp ../../Content/Album13.cpp ../../Content/Album14.cpp ../../Content/Album15.cpp ../../Content/Album16.cpp ../../Content/FragShader.cpp ../../Content/VertShader.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


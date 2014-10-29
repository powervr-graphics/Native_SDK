#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESTrilinear
APIS    = OGLES
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLESTrilinear.o	\
           Back.o	\
           Tape.o	\
           Ball.o	\
           Info.o	\
           o_model.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/Back.cpp ../../Content/Tape.cpp ../../Content/Ball.cpp ../../Content/Info.cpp ../../Content/o_model.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


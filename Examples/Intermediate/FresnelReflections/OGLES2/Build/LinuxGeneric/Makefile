#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2FresnelReflections
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2FresnelReflections.o	\
           FragShader.o	\
           VertShader.o	\
           Mask.o	\
           Basetex.o	\
           Reflection.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/FragShader.cpp ../../Content/VertShader.cpp ../../Content/Mask.cpp ../../Content/Basetex.cpp ../../Content/Reflection.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


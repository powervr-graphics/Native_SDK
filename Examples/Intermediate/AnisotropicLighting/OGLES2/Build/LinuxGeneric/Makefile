#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2AnisotropicLighting
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2AnisotropicLighting.o	\
           FastFragShader.o	\
           FastVertShader.o	\
           SlowFragShader.o	\
           SlowVertShader.o	\
           Mask.o	\
           Basetex.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/FastFragShader.cpp ../../Content/FastVertShader.cpp ../../Content/SlowFragShader.cpp ../../Content/SlowVertShader.cpp ../../Content/Mask.cpp ../../Content/Basetex.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


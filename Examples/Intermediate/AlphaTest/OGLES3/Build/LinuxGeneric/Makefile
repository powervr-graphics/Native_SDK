#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES3AlphaTest
APIS    = OGLES3
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES3AlphaTest.o	\
           TexFragShader.o	\
           DiscardFragShader.o	\
           VertShader.o	\
           Wallwire.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/TexFragShader.cpp ../../Content/DiscardFragShader.cpp ../../Content/VertShader.cpp ../../Content/Wallwire.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


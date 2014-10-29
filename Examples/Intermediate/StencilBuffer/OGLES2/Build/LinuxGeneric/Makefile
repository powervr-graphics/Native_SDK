#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2StencilBuffer
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2StencilBuffer.o	\
           FragShader.o	\
           VertShader.o	\
           Cylinder.o	\
           Sphere.o	\
           Lattice.o	\
           Stone.o	\
           Tile.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/FragShader.cpp ../../Content/VertShader.cpp ../../Content/Cylinder.cpp ../../Content/Sphere.cpp ../../Content/Lattice.cpp ../../Content/Stone.cpp ../../Content/Tile.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2Shaders
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2Shaders.o	\
           Basetex.o	\
           Reflection.o	\
           Cubemap.o	\
           AnisoMap.o	\
           anisotropic_lighting.o	\
           directional_lighting.o	\
           envmap.o	\
           fasttnl.o	\
           lattice.o	\
           phong_lighting.o	\
           point_lighting.o	\
           reflections.o	\
           simple.o	\
           spot_lighting.o	\
           toon.o	\
           vertex_sine.o	\
           wood.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/Basetex.cpp ../../Content/Reflection.cpp ../../Content/Cubemap.cpp ../../Content/AnisoMap.cpp ../../Content/anisotropic_lighting.cpp ../../Content/directional_lighting.cpp ../../Content/envmap.cpp ../../Content/fasttnl.cpp ../../Content/lattice.cpp ../../Content/phong_lighting.cpp ../../Content/point_lighting.cpp ../../Content/reflections.cpp ../../Content/simple.cpp ../../Content/spot_lighting.cpp ../../Content/toon.cpp ../../Content/vertex_sine.cpp ../../Content/wood.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


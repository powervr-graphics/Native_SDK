#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES3ParallaxBumpMap
APIS    = OGLES3
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES3ParallaxBumpMap.o	\
           FragShader.o	\
           VertShader.o	\
           Box.o	\
           base_COLOR.o	\
           base_NRM.o	\
           base_DISP.o	\
           base_SPEC.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/FragShader.cpp ../../Content/VertShader.cpp ../../Content/Box.cpp ../../Content/base_COLOR.cpp ../../Content/base_NRM.cpp ../../Content/base_DISP.cpp ../../Content/base_SPEC.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


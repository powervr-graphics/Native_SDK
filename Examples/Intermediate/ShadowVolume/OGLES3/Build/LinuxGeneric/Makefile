#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES3ShadowVolume
APIS    = OGLES3
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES3ShadowVolume.o	\
           BaseFragShader.o	\
           BaseVertShader.o	\
           ConstFragShader.o	\
           ShadowVolVertShader.o	\
           FullscreenVertShader.o	\
           scene.o	\
           Background.o	\
           Rust.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/BaseFragShader.cpp ../../Content/BaseVertShader.cpp ../../Content/ConstFragShader.cpp ../../Content/ShadowVolVertShader.cpp ../../Content/FullscreenVertShader.cpp ../../Content/scene.cpp ../../Content/Background.cpp ../../Content/Rust.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES3ColourGrading
APIS    = OGLES3
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES3ColourGrading.o	\
           FragShader.o	\
           VertShader.o	\
           SceneFragShader.o	\
           SceneVertShader.o	\
           BackgroundFragShader.o	\
           Mask.o	\
           MaskTexture.o	\
           Background.o	\
           identity.o	\
           cooler.o	\
           warmer.o	\
           bw.o	\
           sepia.o	\
           inverted.o	\
           highcontrast.o	\
           bluewhitegradient.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/FragShader.cpp ../../Content/VertShader.cpp ../../Content/SceneFragShader.cpp ../../Content/SceneVertShader.cpp ../../Content/BackgroundFragShader.cpp ../../Content/Mask.cpp ../../Content/MaskTexture.cpp ../../Content/Background.cpp ../../Content/identity.cpp ../../Content/cooler.cpp ../../Content/warmer.cpp ../../Content/bw.cpp ../../Content/sepia.cpp ../../Content/inverted.cpp ../../Content/highcontrast.cpp ../../Content/bluewhitegradient.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


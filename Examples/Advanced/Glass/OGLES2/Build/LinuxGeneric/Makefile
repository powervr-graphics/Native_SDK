#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2Glass
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2Glass.o	\
           DefaultVertShader.o	\
           DefaultFragShader.o	\
           ParaboloidVertShader.o	\
           SkyboxVertShader.o	\
           SkyboxFragShader.o	\
           EffectVertShader.o	\
           EffectFragShader.o	\
           Balloon.o	\
           Ball.o	\
           BalloonTex.o	\
           SkyboxTex.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/DefaultVertShader.cpp ../../Content/DefaultFragShader.cpp ../../Content/ParaboloidVertShader.cpp ../../Content/SkyboxVertShader.cpp ../../Content/SkyboxFragShader.cpp ../../Content/EffectVertShader.cpp ../../Content/EffectFragShader.cpp ../../Content/Balloon.cpp ../../Content/Ball.cpp ../../Content/BalloonTex.cpp ../../Content/SkyboxTex.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


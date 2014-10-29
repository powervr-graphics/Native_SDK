#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES3MultipleRenderTargets
APIS    = OGLES3
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES3MultipleRenderTargets.o	\
           texture.o	\
           effect.o	\
           scene.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/texture.cpp ../../Content/effect.cpp ../../Content/scene.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


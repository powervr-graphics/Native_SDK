#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESPVRScopeExample
APIS    = OGLES
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLESPVRScopeExample.o	\
           PVRScopeGraphAPI.o	\
           PVRScopeGraph.o	\
           MaskTex.o	\
           Mask.o

VPATH +=	\
         ../../.. : 	\
         ../../Content

INCLUDES += 	\
         -I../../..	\
         -I$(SDKDIR)/Builds/Include

LINK +=	\
       -L$(SDKDIR)/Builds/Linux/$(PLAT_SUFFIX)/Lib	\
       -lPVRScopeDeveloper	\
       -ldl	\
       -lrt

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/MaskTex.cpp ../../Content/Mask.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


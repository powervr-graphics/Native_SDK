#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2PhantomMask
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2PhantomMask.o	\
           MaskMain.o	\
           RoomStill.o	\
           FragShader.o	\
           SHVertShader.o	\
           DiffuseVertShader.o	\
           PhantomMask.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/MaskMain.cpp ../../Content/RoomStill.cpp ../../Content/FragShader.cpp ../../Content/SHVertShader.cpp ../../Content/DiffuseVertShader.cpp ../../Content/PhantomMask.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


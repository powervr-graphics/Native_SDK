#
# File			Makefile
# Title			Makes the tools
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../..
OUTNAME = libogles3tools.a
APIS = OGLES3

include $(SDKDIR)/Builds/Linux/platform.mak

INCLUDES += -I$(SDKDIR)/Tools -I$(SDKDIR)/Tools/OGLES3 -I$(SDKDIR)/Tools/OGLES2 $(addprefix -I, $(PLAT_INC))

VPATH = $(SDKDIR)/Tools : $(SDKDIR)/Tools/OGLES3 : $(SDKDIR)/Tools/OGLES2

OBJECTS = PVRTTextureAPI.o \
	PVRTgles3Ext.o     \
	PVRTFixedPoint.o   \
	PVRTError.o	   \
	PVRTPFXParser.o    \
	PVRTPFXParserAPI.o \
	PVRTPFXSemantics.o \
	PVRTMatrixX.o      \
	PVRTMatrixF.o      \
	PVRTQuaternionF.o  \
	PVRTQuaternionX.o  \
	PVRTMisc.o         \
	PVRTBackground.o   \
	PVRTTrans.o        \
	PVRTModelPOD.o     \
	PVRTVertex.o       \
	PVRTDecompress.o   \
	PVRTTriStrip.o     \
	PVRTPrint3D.o      \
	PVRTPrint3DAPI.o   \
	PVRTTexture.o      \
	PVRTShader.o   \
	PVRTShadowVol.o    \
	PVRTString.o       \
	PVRTStringHash.o   \
	PVRTResourceFile.o \
	PVRTUnicode.o \
	PVRTVector.o

OBJECTS := $(addprefix $(PLAT_OBJPATH)/, $(OBJECTS))
SOURCE_DEPENDENCIES := $(OBJECTS:.o=.d)

all: $(PLAT_OBJPATH)/$(OUTNAME)

$(PLAT_OBJPATH)/$(OUTNAME) : $(OBJECTS) 
	$(PLAT_AR) -r $@ $(OBJECTS)

$(PLAT_OBJPATH)/%.o: %.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES)  $< -o$@

clean:
	@for i in $(OBJECTS) $(SOURCE_DEPENDENCIES) $(PLAT_OBJPATH)/$(OUTNAME); do test -f $$i && rm -vf $$i || true; done
	@if [ -d $(PLAT_OBJPATH) ]; then rmdir -vp $(PLAT_OBJPATH) --ignore-fail-on-non-empty; fi

ifneq "$(MAKECMDGOALS)" "clean"
sinclude $(SOURCE_DEPENDENCIES)
endif
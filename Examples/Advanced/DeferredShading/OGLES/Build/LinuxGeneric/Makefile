#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESDeferredShading
DEPENDS =  PVRCore PVRAssets PVRShell PVRUtilsGles
LIBRARIES =  PVRUtilsGles PVRShell PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESDeferredShading.o

RESOURCES += \
           AttributelessVertexShader.vsh\
           DirectionalLightFragmentShader.fsh\
           GBufferFloorFragmentShader.fsh\
           GBufferFloorVertexShader.vsh\
           GBufferFragmentShader.fsh\
           GBufferVertexShader.vsh\
           PointLightPass1FragmentShader.fsh\
           PointLightPass1VertexShader.vsh\
           PointLightPass2FragmentShader.fsh\
           PointLightPass2VertexShader.vsh\
           PointLightPass3FragmentShader.fsh\
           PointLightPass3VertexShader.vsh\
           WritePlsToFbo.fsh\

COMMON_RESOURCES += \
           Marble.pvr\
           MarbleNormalMap.pvr\
           Table.pvr\
           scene.pod\
           pointlight.pod\




include $(SDKDIR)/Builds/Linux/make_example.mak

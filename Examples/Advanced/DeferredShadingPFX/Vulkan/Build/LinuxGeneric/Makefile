#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanDeferredShadingPFX
DEPENDS =  PVRCore PVRAssets PVRVk PVRShell PVRUtilsVk
LIBRARIES =  PVRUtilsVk PVRShell PVRVk PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanDeferredShadingPFX.o

RESOURCES += \
           AttributelessVertexShader.vsh.spv\
           GBufferFragmentShader.fsh.spv\
           GBufferVertexShader.vsh.spv\
           GBufferFloorFragmentShader.fsh.spv\
           GBufferFloorVertexShader.vsh.spv\
           DirectionalLightFragmentShader.fsh.spv\
           PointLightPass1FragmentShader.fsh.spv\
           PointLightPass1VertexShader.vsh.spv\
           PointLightPass2FragmentShader.fsh.spv\
           PointLightPass2VertexShader.vsh.spv\
           PointLightPass3FragmentShader.fsh.spv\
           PointLightPass3VertexShader.vsh.spv\
           AttributelessVertexShader.vsh\
           GBufferFragmentShader.fsh\
           GBufferVertexShader.vsh\
           GBufferFloorFragmentShader.fsh\
           GBufferFloorVertexShader.vsh\
           DirectionalLightFragmentShader.fsh\
           PointLightPass1FragmentShader.fsh\
           PointLightPass1VertexShader.vsh\
           PointLightPass2FragmentShader.fsh\
           PointLightPass2VertexShader.vsh\
           PointLightPass3FragmentShader.fsh\
           PointLightPass3VertexShader.vsh\
           recompile.sh\
           recompile.bat\
           effect_MRT_PFX3.pfx\

COMMON_RESOURCES += \
           Marble.pvr\
           MarbleNormalMap.pvr\
           Table.pvr\
           scene.pod\
           pointlight.pod\




include $(SDKDIR)/Builds/Linux/make_example.mak

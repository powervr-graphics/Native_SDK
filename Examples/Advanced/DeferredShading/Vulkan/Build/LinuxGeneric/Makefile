#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanDeferredShading
DEPENDS =  PVRCore PVRAssets PVRNativeVulkan PVRVulkan PVRShell PVREngineUtils
LIBRARIES =  PVREngineUtils PVRShell PVRVulkan PVRNativeVulkan PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanDeferredShading.o

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

COMMON_RESOURCES += \
           Marble.pvr\
           MarbleNormalMap.pvr\
           Table.pvr\
           scene.pod\
           pointlight.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

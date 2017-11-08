#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanDeferredShading
DEPENDS =  PVRCore PVRAssets PVRVk PVRShell PVRUtilsVk
LIBRARIES =  PVRUtilsVk PVRShell PVRVk PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanDeferredShading.o

RESOURCES += \
           AttributelessVertexShader_vk.vsh.spv\
           GBufferFragmentShader_vk.fsh.spv\
           GBufferVertexShader_vk.vsh.spv\
           GBufferFloorFragmentShader_vk.fsh.spv\
           GBufferFloorVertexShader_vk.vsh.spv\
           DirectionalLightFragmentShader_vk.fsh.spv\
           PointLightPass1FragmentShader_vk.fsh.spv\
           PointLightPass1VertexShader_vk.vsh.spv\
           PointLightPass2FragmentShader_vk.fsh.spv\
           PointLightPass2VertexShader_vk.vsh.spv\
           PointLightPass3FragmentShader_vk.fsh.spv\
           PointLightPass3VertexShader_vk.vsh.spv\
           AttributelessVertexShader_vk.vsh\
           GBufferFragmentShader_vk.fsh\
           GBufferVertexShader_vk.vsh\
           GBufferFloorFragmentShader_vk.fsh\
           GBufferFloorVertexShader_vk.vsh\
           DirectionalLightFragmentShader_vk.fsh\
           PointLightPass1FragmentShader_vk.fsh\
           PointLightPass1VertexShader_vk.vsh\
           PointLightPass2FragmentShader_vk.fsh\
           PointLightPass2VertexShader_vk.vsh\
           PointLightPass3FragmentShader_vk.fsh\
           PointLightPass3VertexShader_vk.vsh\
           recompile.sh\
           recompile.bat\

COMMON_RESOURCES += \
           Marble.pvr\
           MarbleNormalMap.pvr\
           Table.pvr\
           scene.pod\
           pointlight.pod\




include $(SDKDIR)/Builds/Linux/make_example.mak

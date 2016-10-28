#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanSkinning
DEPENDS =  PVRCore PVRAssets PVRVulkanGlue PVRNativeVulkan PVRVulkan PVRShell PVRUIRenderer
LIBRARIES =  PVRUIRenderer PVRShell PVRVulkan PVRNativeVulkan PVRVulkanGlue PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanSkinning.o

RESOURCES += \
           DefaultFragShader_vk.fsh\
           DefaultVertShader_vk.vsh\
           SkinnedFragShader_vk.fsh\
           SkinnedVertShader_vk.vsh\
           DefaultVertShader_vk.spv\
           DefaultFragShader_vk.spv\
           SkinnedVertShader_vk.spv\
           SkinnedFragShader_vk.spv\
           Skinning_vk.pfx\

COMMON_RESOURCES += \
           Arm.pvr\
           Grid.pvr\
           Pipes.pvr\
           RobotDiffuse.pvr\
           RobotNormal.pvr\
           Tiles.pvr\
           Robot.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

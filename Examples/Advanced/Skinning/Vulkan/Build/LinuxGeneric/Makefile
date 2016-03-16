#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanSkinning
DEPENDS =  PVRCore PVRAssets PVRVulkanGlue PVRVulkan PVRNativeVulkan PVRShell PVRUIRenderer
LIBRARIES =  PVRUIRenderer PVRShell PVRNativeVulkan PVRVulkan PVRVulkanGlue PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanSkinning.o

RESOURCES += \
           DefaultVertShader_vk.spv\
           DefaultFragShader_vk.spv\
           SkinnedVertShader_vk.spv\
           SkinnedFragShader_vk.spv\

COMMON_RESOURCES += \
           Arm.pvr\
           Grid.pvr\
           Pipes.pvr\
           RobotDiffuse.pvr\
           RobotNormal.pvr\
           Tiles.pvr\
           Robot.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

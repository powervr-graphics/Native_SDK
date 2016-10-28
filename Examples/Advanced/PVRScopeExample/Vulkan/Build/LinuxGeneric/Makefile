#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanPVRScopeExample
DEPENDS =  PVRCore PVRAssets PVRVulkanGlue PVRNativeVulkan PVRVulkan PVRShell PVRUIRenderer
LIBRARIES =  PVRScopeDeveloper dl PVRUIRenderer PVRShell PVRVulkan PVRNativeVulkan PVRVulkanGlue PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS) ../../../../../../Builds/Linux/$(PLAT_SUFFIX)/Lib

OBJECTS +=	\
           Vulkan/VulkanPVRScopeExample.o	\
           PVRScopeGraph.o

RESOURCES += \
           FragShader_vk.fsh\
           VertShader_vk.vsh\
           GraphFragShader_vk.fsh\
           GraphVertShader_vk.vsh\
           FragShader_vk.fsh.spv\
           VertShader_vk.vsh.spv\
           GraphFragShader_vk.fsh.spv\
           GraphVertShader_vk.vsh.spv\

COMMON_RESOURCES += \
           Marble.pvr\
           scene.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

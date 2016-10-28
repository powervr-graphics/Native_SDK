#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanIntroducingPVRApi
DEPENDS =  PVRCore PVRShell PVRAssets PVRVulkanGlue PVRNativeVulkan PVRVulkan PVRUIRenderer
LIBRARIES =  PVRUIRenderer PVRVulkan PVRNativeVulkan PVRVulkanGlue PVRAssets PVRShell PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanIntroducingPVRApi.o

RESOURCES += \
           VertShader_vk.spv\
           FragShader_vk.spv\
           VertShader_vk.vsh\
           FragShader_vk.fsh\

COMMON_RESOURCES += \
           GnomeWood.pvr\
           DarkWood.pvr\
           ClearWood.pvr\
           GnomeToy.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

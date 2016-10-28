#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanIntroducingPVRAssets
DEPENDS =  PVRCore PVRShell PVRAssets PVRVulkanGlue PVRNativeVulkan
LIBRARIES =  PVRNativeVulkan PVRVulkanGlue PVRAssets PVRShell PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanIntroducingPVRAssets.o

RESOURCES += \
           FragShader_vk.fsh\
           VertShader_vk.vsh\
           FragShader_vk.spv\
           VertShader_vk.spv\

COMMON_RESOURCES += \
           GnomeWood.pvr\
           DarkWood.pvr\
           ClearWood.pvr\
           GnomeToy.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

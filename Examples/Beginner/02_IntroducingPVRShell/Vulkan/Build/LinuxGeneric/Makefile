#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanIntroducingPVRShell
DEPENDS =  PVRCore PVRShell PVRVulkanGlue PVRNativeVulkan
LIBRARIES =  PVRNativeVulkan PVRVulkanGlue PVRShell PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanIntroducingPVRShell.o

RESOURCES += \
           FragShader_vk.spv\
           VertShader_vk.spv\
           FragShader_vk.fsh\
           VertShader_vk.vsh\



include $(SDKDIR)/Builds/Linux/make_example.mak

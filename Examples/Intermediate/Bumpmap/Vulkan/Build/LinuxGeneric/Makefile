#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanBumpmap
DEPENDS =  PVRCore PVRAssets PVRVulkanGlue PVRVulkan PVRNativeVulkan PVRShell PVRUIRenderer
LIBRARIES =  PVRUIRenderer PVRShell PVRNativeVulkan PVRVulkan PVRVulkanGlue PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanBumpmap.o

RESOURCES += \
           FragShader_VK.spv\
           VertShader_VK.spv\

COMMON_RESOURCES += \
           MarbleNormalMap.pvr\
           Marble.pvr\
           scene.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

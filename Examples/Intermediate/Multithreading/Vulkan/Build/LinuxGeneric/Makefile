#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanMultithreading
DEPENDS =  PVRCore PVRAssets PVRNativeVulkan PVRVulkan PVRShell PVREngineUtils
LIBRARIES =  PVREngineUtils PVRShell PVRVulkan PVRNativeVulkan PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanMultithreading.o

RESOURCES += \
           FragShader_vk.fsh\
           FragShader_vk.fsh.spv\
           VertShader_vk.vsh\
           VertShader_vk.vsh.spv\

COMMON_RESOURCES += \
           MarbleNormalMap.pvr\
           Marble.pvr\
           scene.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

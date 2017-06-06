#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanPostProcessing
DEPENDS =  PVRCore PVRAssets PVRNativeVulkan PVRVulkan PVRShell PVREngineUtils
LIBRARIES =  PVREngineUtils PVRShell PVRVulkan PVRNativeVulkan PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanPostProcessing.o

RESOURCES += \
           PostBloomFragShader_vk.fsh\
           PostBloomVertShader_vk.vsh\
           PreBloomFragShader_vk.fsh\
           PreBloomVertShader_vk.vsh\
           BlurFragShader_vk.fsh\
           BlurVertShader_vk.vsh\
           FragShader_vk.fsh\
           VertShader_vk.vsh\
           PostBloomFragShader_vk.fsh.spv\
           PostBloomVertShader_vk.vsh.spv\
           PreBloomFragShader_vk.fsh.spv\
           PreBloomVertShader_vk.vsh.spv\
           BlurFragShader_vk.fsh.spv\
           BlurVertShader_vk.vsh.spv\
           FragShader_vk.fsh.spv\
           VertShader_vk.vsh.spv\

COMMON_RESOURCES += \
           Marble.pvr\
           scene.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

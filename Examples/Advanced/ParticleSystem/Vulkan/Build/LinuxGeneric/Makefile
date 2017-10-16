#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanParticleSystem
DEPENDS =  PVRCore PVRAssets PVRNativeVulkan PVRVulkan PVRShell PVREngineUtils
LIBRARIES =  PVREngineUtils PVRShell PVRVulkan PVRNativeVulkan PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanParticleSystem.o	\
           Vulkan/ParticleSystemGPU.o

RESOURCES += \
           FragShader_vk.fsh\
           VertShader_vk.vsh\
           ParticleFragShader_vk.fsh\
           ParticleVertShader_vk.vsh\
           ParticleSolver_vk.csh\
           FragShader_vk.fsh.spv\
           VertShader_vk.vsh.spv\
           FloorVertShader_vk.vsh.spv\
           ParticleFragShader_vk.fsh.spv\
           ParticleVertShader_vk.vsh.spv\
           ParticleSolver_vk.csh.spv\
           FloorVertShader_vk.vsh\

COMMON_RESOURCES += \
           sphere.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanGlass
DEPENDS =  PVRCore PVRAssets PVRNativeVulkan PVRVulkan PVRShell PVREngineUtils
LIBRARIES =  PVREngineUtils PVRShell PVRVulkan PVRNativeVulkan PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanGlass.o

RESOURCES += \
           DefaultFragShader_vk.fsh\
           DefaultFragShader_vk.fsh.spv\
           DefaultVertShader_vk.vsh\
           DefaultVertShader_vk.vsh.spv\
           EffectChromaticDispersion_vk.fsh\
           EffectChromaticDispersion_vk.fsh.spv\
           EffectChromaticDispersion_vk.vsh\
           EffectChromaticDispersion_vk.vsh.spv\
           EffectReflectChromDispersion_vk.fsh\
           EffectReflectChromDispersion_vk.fsh.spv\
           EffectReflectChromDispersion_vk.vsh\
           EffectReflectChromDispersion_vk.vsh.spv\
           EffectReflectFragShader_vk.fsh\
           EffectReflectFragShader_vk.fsh.spv\
           EffectReflectionRefraction_vk.fsh\
           EffectReflectionRefraction_vk.fsh.spv\
           EffectReflectionRefraction_vk.vsh\
           EffectReflectionRefraction_vk.vsh.spv\
           EffectReflectVertShader_vk.vsh\
           EffectReflectVertShader_vk.vsh.spv\
           EffectRefractFragShader_vk.fsh\
           EffectRefractFragShader_vk.fsh.spv\
           EffectRefractVertShader_vk.vsh\
           EffectRefractVertShader_vk.vsh.spv\
           ParaboloidVertShader_vk.vsh\
           ParaboloidVertShader_vk.vsh.spv\
           SkyboxFragShader_vk.fsh\
           SkyboxFragShader_vk.fsh.spv\
           SkyboxVertShader_vk.vsh\
           SkyboxVertShader_vk.vsh.spv\

COMMON_RESOURCES += \
           BalloonTex.pvr\
           BalloonTex2.pvr\
           SkyboxTex.pvr\
           Balloon.pod\
           scene.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

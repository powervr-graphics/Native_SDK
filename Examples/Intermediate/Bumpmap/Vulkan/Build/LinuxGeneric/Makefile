#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanBumpmap
DEPENDS =  PVRCore PVRAssets PVRVk PVRShell PVRUtilsVk
LIBRARIES =  PVRUtilsVk PVRShell PVRVk PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanBumpmap.o

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

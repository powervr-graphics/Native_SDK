#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanIntroducingPVRUtils
DEPENDS =  PVRCore PVRShell PVRAssets PVRVk PVRUtilsVk
LIBRARIES =  PVRUtilsVk PVRVk PVRAssets PVRShell PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanIntroducingPVRUtils.o

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

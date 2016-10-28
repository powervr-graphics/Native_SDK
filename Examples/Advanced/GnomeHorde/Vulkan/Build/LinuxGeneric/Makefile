#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanGnomeHorde
DEPENDS =  PVRCore PVRAssets PVRVulkanGlue PVRNativeVulkan PVRVulkan PVRShell PVRUIRenderer
LIBRARIES =  PVRUIRenderer PVRShell PVRVulkan PVRNativeVulkan PVRVulkanGlue PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanGnomeHorde.o

RESOURCES += \
           Object.vsh.spv\
           Shadow.vsh.spv\
           Solid.fsh.spv\
           Shadow.fsh.spv\
           Plant.fsh.spv\
           Object.vsh\
           Shadow.vsh\
           Solid.fsh\
           Shadow.fsh\
           Plant.fsh\

COMMON_RESOURCES += \
           bigMushroom_shadow.pvr\
           bigMushroom_texture.pvr\
           fern.pvr\
           fern_shadow.pvr\
           gnome_shadow.pvr\
           gnome_texture.pvr\
           mushroom_shadow.pvr\
           mushroom_texture.pvr\
           rocks.pvr\
           bigMushroom0.pod\
           bigMushroom_shadow0.pod\
           fern0.pod\
           fern_shadow0.pod\
           gnome0.pod\
           gnome1.pod\
           gnome2.pod\
           gnome3.pod\
           gnome4.pod\
           gnome5.pod\
           gnome6.pod\
           gnome_shadow0.pod\
           mushroom0.pod\
           mushroom1.pod\
           mushroom_shadow0.pod\
           rocks0.pod\


include $(SDKDIR)/Builds/Linux/make_example.mak

#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = VulkanNavigation2D
DEPENDS =  PVRCore PVRAssets PVRVk PVRShell PVRUtilsVk
LIBRARIES =  PVRUtilsVk PVRShell PVRVk PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           Vulkan/VulkanNavigation2D.o	\
           NavDataProcess.o

RESOURCES += \
           AA_VertShader_vk.vsh\
           AA_VertShader_vk.vsh.spv\
           AA_FragShader_vk.fsh\
           AA_FragShader_vk.fsh.spv\
           VertShader_vk.vsh\
           VertShader_vk.vsh.spv\
           FragShader_vk.fsh\
           FragShader_vk.fsh.spv\

COMMON_RESOURCES += \
           ATM.pvr\
           bank.pvr\
           bar.pvr\
           butcher.pvr\
           cafe.pvr\
           casino.pvr\
           cinema.pvr\
           college.pvr\
           courthouse.pvr\
           dentist.pvr\
           doctors.pvr\
           embassy.pvr\
           fastfood.pvr\
           fire.pvr\
           florist.pvr\
           font.pvr\
           hairdresser.pvr\
           hospital.pvr\
           library.pvr\
           optician.pvr\
           other.pvr\
           parking.pvr\
           petrol.pvr\
           pharmacy.pvr\
           police.pvr\
           postbox.pvr\
           postoffice.pvr\
           pub.pvr\
           restaurant.pvr\
           Road.pvr\
           shop.pvr\
           theatre.pvr\
           toilets.pvr\
           university.pvr\
           vets.pvr\
           worship.pvr\
           map.osm\




include $(SDKDIR)/Builds/Linux/make_example.mak

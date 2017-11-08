#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLESNavigation2D
DEPENDS =  PVRCore PVRAssets PVRShell PVRUtilsGles
LIBRARIES =  PVRUtilsGles PVRShell PVRAssets PVRCore
LIBPATHS =  ../../../../../../Framework/Bin/Linux_$(PLAT_SUFFIX)/$(DEBUG_RELEASE)$(WS)

OBJECTS +=	\
           OGLES/OGLESNavigation2D.o	\
           NavDataProcess.o

RESOURCES += \
           AA_VertShader.vsh\
           AA_FragShader.fsh\
           VertShader.vsh\
           FragShader.fsh\

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

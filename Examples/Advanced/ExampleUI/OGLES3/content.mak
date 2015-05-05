#--------------------------------------------------------------------------
# Name         : content.mak
# Title        : Makefile to build content files
#
# Copyright    : Copyright (C) by Imagination Technologies Limited.
#
# Description  : Makefile to wrap content files for examples in the PowerVR SDK
#
# Platform     :
#
#--------------------------------------------------------------------------

#############################################################################
## Variables
#############################################################################
FILEWRAP 	= ..\..\..\..\Utilities\Filewrap\Windows_x86_32\Filewrap.exe
CONTENTDIR = Content

#############################################################################
## Instructions
#############################################################################

RESOURCES = \
	$(CONTENTDIR)/SpriteShaderF.cpp \
	$(CONTENTDIR)/SpriteShaderV.cpp \
	$(CONTENTDIR)/TexColShaderF.cpp \
	$(CONTENTDIR)/TexColShaderV.cpp \
	$(CONTENTDIR)/ColShaderF.cpp \
	$(CONTENTDIR)/ColShaderV.cpp \
	$(CONTENTDIR)/container-corner.cpp \
	$(CONTENTDIR)/container-vertical.cpp \
	$(CONTENTDIR)/container-horizontal.cpp \
	$(CONTENTDIR)/topbar.cpp \
	$(CONTENTDIR)/background.cpp \
	$(CONTENTDIR)/clock-face.cpp \
	$(CONTENTDIR)/clock-face-small.cpp \
	$(CONTENTDIR)/hand.cpp \
	$(CONTENTDIR)/hand-small.cpp \
	$(CONTENTDIR)/battery.cpp \
	$(CONTENTDIR)/internet-web-browser.cpp \
	$(CONTENTDIR)/mail-message-new.cpp \
	$(CONTENTDIR)/network-wireless.cpp \
	$(CONTENTDIR)/office-calendar.cpp \
	$(CONTENTDIR)/weather-sun-cloud.cpp \
	$(CONTENTDIR)/weather-storm.cpp \
	$(CONTENTDIR)/weather-rain.cpp \
	$(CONTENTDIR)/text1.cpp \
	$(CONTENTDIR)/text2.cpp \
	$(CONTENTDIR)/text-weather.cpp \
	$(CONTENTDIR)/text-fri.cpp \
	$(CONTENTDIR)/text-sat.cpp \
	$(CONTENTDIR)/text-sun.cpp \
	$(CONTENTDIR)/text-mon.cpp \
	$(CONTENTDIR)/weather-sun-cloud-big.cpp \
	$(CONTENTDIR)/window-bottom.cpp \
	$(CONTENTDIR)/window-bottomcorner.cpp \
	$(CONTENTDIR)/window-side.cpp \
	$(CONTENTDIR)/window-top.cpp \
	$(CONTENTDIR)/window-topleft.cpp \
	$(CONTENTDIR)/window-topright.cpp \
	$(CONTENTDIR)/loremipsum.cpp

all: resources
	
help:
	@echo Valid targets are:
	@echo resources, clean
	@echo FILEWRAP can be used to override the default path to the Filewrap utility.

clean:
	@for i in $(RESOURCES); do test -f $$i && rm -vf $$i || true; done

resources: $(RESOURCES)

$(CONTENTDIR):
	-mkdir "$@"

$(CONTENTDIR)/SpriteShaderF.cpp: $(CONTENTDIR) ./SpriteShaderF.fsh
	$(FILEWRAP)  -s  -o $@ ./SpriteShaderF.fsh

$(CONTENTDIR)/SpriteShaderV.cpp: $(CONTENTDIR) ./SpriteShaderV.vsh
	$(FILEWRAP)  -s  -o $@ ./SpriteShaderV.vsh

$(CONTENTDIR)/TexColShaderF.cpp: $(CONTENTDIR) ./TexColShaderF.fsh
	$(FILEWRAP)  -s  -o $@ ./TexColShaderF.fsh

$(CONTENTDIR)/TexColShaderV.cpp: $(CONTENTDIR) ./TexColShaderV.vsh
	$(FILEWRAP)  -s  -o $@ ./TexColShaderV.vsh

$(CONTENTDIR)/ColShaderF.cpp: $(CONTENTDIR) ./ColShaderF.fsh
	$(FILEWRAP)  -s  -o $@ ./ColShaderF.fsh

$(CONTENTDIR)/ColShaderV.cpp: $(CONTENTDIR) ./ColShaderV.vsh
	$(FILEWRAP)  -s  -o $@ ./ColShaderV.vsh

$(CONTENTDIR)/container-corner.cpp: $(CONTENTDIR) ./container-corner.pvr
	$(FILEWRAP)  -o $@ ./container-corner.pvr

$(CONTENTDIR)/container-vertical.cpp: $(CONTENTDIR) ./container-vertical.pvr
	$(FILEWRAP)  -o $@ ./container-vertical.pvr

$(CONTENTDIR)/container-horizontal.cpp: $(CONTENTDIR) ./container-horizontal.pvr
	$(FILEWRAP)  -o $@ ./container-horizontal.pvr

$(CONTENTDIR)/topbar.cpp: $(CONTENTDIR) ./topbar.pvr
	$(FILEWRAP)  -o $@ ./topbar.pvr

$(CONTENTDIR)/background.cpp: $(CONTENTDIR) ./background.pvr
	$(FILEWRAP)  -o $@ ./background.pvr

$(CONTENTDIR)/clock-face.cpp: $(CONTENTDIR) ./clock-face.pvr
	$(FILEWRAP)  -o $@ ./clock-face.pvr

$(CONTENTDIR)/clock-face-small.cpp: $(CONTENTDIR) ./clock-face-small.pvr
	$(FILEWRAP)  -o $@ ./clock-face-small.pvr

$(CONTENTDIR)/hand.cpp: $(CONTENTDIR) ./hand.pvr
	$(FILEWRAP)  -o $@ ./hand.pvr

$(CONTENTDIR)/hand-small.cpp: $(CONTENTDIR) ./hand-small.pvr
	$(FILEWRAP)  -o $@ ./hand-small.pvr

$(CONTENTDIR)/battery.cpp: $(CONTENTDIR) ./battery.pvr
	$(FILEWRAP)  -o $@ ./battery.pvr

$(CONTENTDIR)/internet-web-browser.cpp: $(CONTENTDIR) ./internet-web-browser.pvr
	$(FILEWRAP)  -o $@ ./internet-web-browser.pvr

$(CONTENTDIR)/mail-message-new.cpp: $(CONTENTDIR) ./mail-message-new.pvr
	$(FILEWRAP)  -o $@ ./mail-message-new.pvr

$(CONTENTDIR)/network-wireless.cpp: $(CONTENTDIR) ./network-wireless.pvr
	$(FILEWRAP)  -o $@ ./network-wireless.pvr

$(CONTENTDIR)/office-calendar.cpp: $(CONTENTDIR) ./office-calendar.pvr
	$(FILEWRAP)  -o $@ ./office-calendar.pvr

$(CONTENTDIR)/weather-sun-cloud.cpp: $(CONTENTDIR) ./weather-sun-cloud.pvr
	$(FILEWRAP)  -o $@ ./weather-sun-cloud.pvr

$(CONTENTDIR)/weather-storm.cpp: $(CONTENTDIR) ./weather-storm.pvr
	$(FILEWRAP)  -o $@ ./weather-storm.pvr

$(CONTENTDIR)/weather-rain.cpp: $(CONTENTDIR) ./weather-rain.pvr
	$(FILEWRAP)  -o $@ ./weather-rain.pvr

$(CONTENTDIR)/text1.cpp: $(CONTENTDIR) ./text1.pvr
	$(FILEWRAP)  -o $@ ./text1.pvr

$(CONTENTDIR)/text2.cpp: $(CONTENTDIR) ./text2.pvr
	$(FILEWRAP)  -o $@ ./text2.pvr

$(CONTENTDIR)/text-weather.cpp: $(CONTENTDIR) ./text-weather.pvr
	$(FILEWRAP)  -o $@ ./text-weather.pvr

$(CONTENTDIR)/text-fri.cpp: $(CONTENTDIR) ./text-fri.pvr
	$(FILEWRAP)  -o $@ ./text-fri.pvr

$(CONTENTDIR)/text-sat.cpp: $(CONTENTDIR) ./text-sat.pvr
	$(FILEWRAP)  -o $@ ./text-sat.pvr

$(CONTENTDIR)/text-sun.cpp: $(CONTENTDIR) ./text-sun.pvr
	$(FILEWRAP)  -o $@ ./text-sun.pvr

$(CONTENTDIR)/text-mon.cpp: $(CONTENTDIR) ./text-mon.pvr
	$(FILEWRAP)  -o $@ ./text-mon.pvr

$(CONTENTDIR)/weather-sun-cloud-big.cpp: $(CONTENTDIR) ./weather-sun-cloud-big.pvr
	$(FILEWRAP)  -o $@ ./weather-sun-cloud-big.pvr

$(CONTENTDIR)/window-bottom.cpp: $(CONTENTDIR) ./window-bottom.pvr
	$(FILEWRAP)  -o $@ ./window-bottom.pvr

$(CONTENTDIR)/window-bottomcorner.cpp: $(CONTENTDIR) ./window-bottomcorner.pvr
	$(FILEWRAP)  -o $@ ./window-bottomcorner.pvr

$(CONTENTDIR)/window-side.cpp: $(CONTENTDIR) ./window-side.pvr
	$(FILEWRAP)  -o $@ ./window-side.pvr

$(CONTENTDIR)/window-top.cpp: $(CONTENTDIR) ./window-top.pvr
	$(FILEWRAP)  -o $@ ./window-top.pvr

$(CONTENTDIR)/window-topleft.cpp: $(CONTENTDIR) ./window-topleft.pvr
	$(FILEWRAP)  -o $@ ./window-topleft.pvr

$(CONTENTDIR)/window-topright.cpp: $(CONTENTDIR) ./window-topright.pvr
	$(FILEWRAP)  -o $@ ./window-topright.pvr

$(CONTENTDIR)/loremipsum.cpp: $(CONTENTDIR) ./loremipsum.pvr
	$(FILEWRAP)  -o $@ ./loremipsum.pvr

############################################################################
# End of file (content.mak)
############################################################################

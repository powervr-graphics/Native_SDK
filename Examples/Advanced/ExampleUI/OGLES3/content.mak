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

$(CONTENTDIR)/SpriteShaderF.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./SpriteShaderF.fsh

$(CONTENTDIR)/SpriteShaderV.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./SpriteShaderV.vsh

$(CONTENTDIR)/TexColShaderF.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./TexColShaderF.fsh

$(CONTENTDIR)/TexColShaderV.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./TexColShaderV.vsh

$(CONTENTDIR)/ColShaderF.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./ColShaderF.fsh

$(CONTENTDIR)/ColShaderV.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./ColShaderV.vsh

$(CONTENTDIR)/container-corner.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./container-corner.pvr

$(CONTENTDIR)/container-vertical.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./container-vertical.pvr

$(CONTENTDIR)/container-horizontal.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./container-horizontal.pvr

$(CONTENTDIR)/topbar.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./topbar.pvr

$(CONTENTDIR)/background.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./background.pvr

$(CONTENTDIR)/clock-face.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./clock-face.pvr

$(CONTENTDIR)/clock-face-small.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./clock-face-small.pvr

$(CONTENTDIR)/hand.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./hand.pvr

$(CONTENTDIR)/hand-small.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./hand-small.pvr

$(CONTENTDIR)/battery.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./battery.pvr

$(CONTENTDIR)/internet-web-browser.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./internet-web-browser.pvr

$(CONTENTDIR)/mail-message-new.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./mail-message-new.pvr

$(CONTENTDIR)/network-wireless.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./network-wireless.pvr

$(CONTENTDIR)/office-calendar.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./office-calendar.pvr

$(CONTENTDIR)/weather-sun-cloud.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./weather-sun-cloud.pvr

$(CONTENTDIR)/weather-storm.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./weather-storm.pvr

$(CONTENTDIR)/weather-rain.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./weather-rain.pvr

$(CONTENTDIR)/text1.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./text1.pvr

$(CONTENTDIR)/text2.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./text2.pvr

$(CONTENTDIR)/text-weather.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./text-weather.pvr

$(CONTENTDIR)/text-fri.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./text-fri.pvr

$(CONTENTDIR)/text-sat.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./text-sat.pvr

$(CONTENTDIR)/text-sun.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./text-sun.pvr

$(CONTENTDIR)/text-mon.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./text-mon.pvr

$(CONTENTDIR)/weather-sun-cloud-big.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./weather-sun-cloud-big.pvr

$(CONTENTDIR)/window-bottom.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./window-bottom.pvr

$(CONTENTDIR)/window-bottomcorner.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./window-bottomcorner.pvr

$(CONTENTDIR)/window-side.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./window-side.pvr

$(CONTENTDIR)/window-top.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./window-top.pvr

$(CONTENTDIR)/window-topleft.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./window-topleft.pvr

$(CONTENTDIR)/window-topright.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./window-topright.pvr

$(CONTENTDIR)/loremipsum.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./loremipsum.pvr

############################################################################
# End of file (content.mak)
############################################################################

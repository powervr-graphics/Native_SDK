#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES3ExampleUI
APIS    = OGLES3
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES3ExampleUI.o	\
           SpriteShaderF.o	\
           SpriteShaderV.o	\
           TexColShaderF.o	\
           TexColShaderV.o	\
           ColShaderF.o	\
           ColShaderV.o	\
           container-corner.o	\
           container-vertical.o	\
           container-horizontal.o	\
           topbar.o	\
           background.o	\
           clock-face.o	\
           clock-face-small.o	\
           hand.o	\
           hand-small.o	\
           battery.o	\
           internet-web-browser.o	\
           mail-message-new.o	\
           network-wireless.o	\
           office-calendar.o	\
           weather-sun-cloud.o	\
           weather-storm.o	\
           weather-rain.o	\
           text1.o	\
           text2.o	\
           text-weather.o	\
           text-fri.o	\
           text-sat.o	\
           text-sun.o	\
           text-mon.o	\
           weather-sun-cloud-big.o	\
           window-bottom.o	\
           window-bottomcorner.o	\
           window-side.o	\
           window-top.o	\
           window-topleft.o	\
           window-topright.o	\
           loremipsum.o

VPATH +=	\
         ../../Content

include $(SDKDIR)/Builds/Linux/make_example.mak

../../Content/SpriteShaderF.cpp ../../Content/SpriteShaderV.cpp ../../Content/TexColShaderF.cpp ../../Content/TexColShaderV.cpp ../../Content/ColShaderF.cpp ../../Content/ColShaderV.cpp ../../Content/container-corner.cpp ../../Content/container-vertical.cpp ../../Content/container-horizontal.cpp ../../Content/topbar.cpp ../../Content/background.cpp ../../Content/clock-face.cpp ../../Content/clock-face-small.cpp ../../Content/hand.cpp ../../Content/hand-small.cpp ../../Content/battery.cpp ../../Content/internet-web-browser.cpp ../../Content/mail-message-new.cpp ../../Content/network-wireless.cpp ../../Content/office-calendar.cpp ../../Content/weather-sun-cloud.cpp ../../Content/weather-storm.cpp ../../Content/weather-rain.cpp ../../Content/text1.cpp ../../Content/text2.cpp ../../Content/text-weather.cpp ../../Content/text-fri.cpp ../../Content/text-sat.cpp ../../Content/text-sun.cpp ../../Content/text-mon.cpp ../../Content/weather-sun-cloud-big.cpp ../../Content/window-bottom.cpp ../../Content/window-bottomcorner.cpp ../../Content/window-side.cpp ../../Content/window-top.cpp ../../Content/window-topleft.cpp ../../Content/window-topright.cpp ../../Content/loremipsum.cpp: wrap_content_files 

$(PLAT_OBJPATH)/%.o: ../../Content/%.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@


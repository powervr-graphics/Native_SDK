#
# File			Makefile
# Title			Makes the demo
# Author		PowerVR
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = $(shell cd ../../../../../.. && pwd)
VPATH = ../..

CROSS_COMPILE ?= arm-linux-gnueabihf-

ifdef TOOLCHAIN
PLAT_CC  = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)gcc
PLAT_CPP = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)g++
PLAT_AR  = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)ar
else
PLAT_CC  = $(CROSS_COMPILE)gcc
PLAT_CPP = $(CROSS_COMPILE)g++
PLAT_AR  = $(CROSS_COMPILE)ar
endif

ifeq "$(X11BUILD)" "1"
ifndef X11ROOT
$(error When building an X11 BUILD you must set X11ROOT to point at the location where your X11 headers and libs can be found.)
endif
ifeq "$(EWSBUILD)" "1"
$(error Cannot have both X11BUILD and EWSBUILD enabled at the same time)
endif
WS_LIBS = -L$(X11ROOT)/lib -lX11 -lXau -ldl
WS_RPATH = $(X11ROOT)/lib
WS_INC  = $(X11ROOT)/include
WS = X11
else
ifeq "$(EWSBUILD)" "1"
PLAT_CFLAGS += -DEWS
WS_LIBS = -lews
WS_INC =
WS=EWS
else
ifeq "$(DRMBUILD)" "1"
ifndef DRMROOT
$(error When building a DRM BUILD you must set DRMROOT to point at the location where your DRM headers and libs can be found.)
endif
WS_LIBS = -L$(DRMROOT)/lib -ldrm -lgbm -ludev -ldl -Wl,--rpath-link,$(DRMROOT)/lib
WS_INC = $(DRMROOT)/include $(DRMROOT)/include/libdrm $(DRMROOT)/include/gbm
WS=DRM
else
WS_LIBS =
WS_INC  =
WS = NullWS
endif
endif
endif

LIBDIR ?= $(SDKDIR)/Builds/Linux/armv7hf/Lib
LIBDIR_FLAGS = -L$(LIBDIR) -Wl,--rpath-link,$(LIBDIR):$(WS_RPATH)

ifdef Debug
DEBUG_RELEASE = Debug
PLAT_CFLAGS   += -g
else
DEBUG_RELEASE = Release
PLAT_CFLAGS   += -O2
endif

PLAT_CFLAGS += -Wall -march=armv7 -mfloat-abi=hard
PLAT_INC  = $(SDKDIR)/Builds/Include $(WS_INC)

PLAT_OBJPATH = $(DEBUG_RELEASE)$(WS)
PLAT_LINK = $(LIBDIR_FLAGS) -lEGL -lGLESv2 $(WS_LIBS) -L$(TOOLCHAIN)/lib -march=armv7 -mfloat-abi=hard

ifeq "$(WS)" "X11"
SRCNAME = OGLESHelloAPI_LinuxX11
else
ifeq "$(WS)" "EWS"
SRCNAME = OGLESHelloAPI_EWS
else
ifeq "$(WS)" "DRM"
SRCNAME = OGLESHelloAPI_LinuxDRM
else
SRCNAME = OGLESHelloAPI_NullWS
endif
endif
endif

OUTNAME = OGLESHelloAPI

OBJECTS = $(PLAT_OBJPATH)/$(SRCNAME).o

COMMON_INCLUDES = $(addprefix -I, $(PLAT_INC))

$(PLAT_OBJPATH)/$(OUTNAME) : $(OBJECTS) 
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) -o $(PLAT_OBJPATH)/$(OUTNAME) $(OBJECTS) $(LINK) $(PLAT_LINK)

$(PLAT_OBJPATH)/%.o: %.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CC) $(PLAT_CFLAGS) -c $(COMMON_INCLUDES) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(PLAT_OBJPATH) *.o


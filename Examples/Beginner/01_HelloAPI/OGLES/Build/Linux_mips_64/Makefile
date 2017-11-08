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

CROSS_COMPILE ?= mips-img-linux-gnu-

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
WS_LIBS = -L$(X11ROOT)/lib -lX11 -lXau
WS_INC  = $(X11ROOT)/include
WS = X11
PLAT_CPPFLAGS+=-DBUILD_XLIB
else
ifeq "$(WAYLANDBUILD)" "1"
ifndef WAYLANDROOT
$(error When building an WAYLAND BUILD you must set WAYLANDROOT to point at the location where your WAYLAND headers and libs can be found.)
endif
WS_LIBS = -L$(WAYLANDROOT)/lib -lwayland-egl -lwayland-client -lffi
WS_INC  = $(WAYLANDROOT)/include
WS = Wayland
PLAT_CPPFLAGS+=-DBUILD_WAYLAND
else
WS_LIBS =
WS_INC  =
WS = NullWS
endif
endif

LIBDIR ?= $(SDKDIR)/Builds/Linux/mips_64/Lib
LIBDIR_FLAGS = -L$(LIBDIR) -Wl,--rpath-link,$(LIBDIR):$(WS_RPATH)

ifdef Debug
DEBUG_RELEASE = Debug
PLAT_CFLAGS   += -g
else
DEBUG_RELEASE = Release
PLAT_CFLAGS   += -O2
endif

PLAT_CFLAGS += -Wall -march=mips64r6 -mabi=64 -EL

#enable C++11 if available
GCC_CPP11 = $(shell OUTPUT=`$(PLAT_CPP) -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'`;if [ $$OUTPUT -gt 40600 ];then echo 1;else echo 0;fi;)

PLAT_CPPFLAGS += -Wno-deprecated-declarations

ifeq "$(GCC_CPP11)" "1"
PLAT_CPPFLAGS+= -std=c++11
else
PLAT_CPPFLAGS+= -std=c++0x
endif

PLAT_INC  = $(SDKDIR)/Builds/Include $(WS_INC)

PLAT_OBJPATH = $(DEBUG_RELEASE)$(WS)
PLAT_LINK = $(LIBDIR_FLAGS) -ldl -march=mips64r6 -mabi=64 -EL $(WS_LIBS) -L$(TOOLCHAIN)/lib

ifeq "$(WS)" "X11"
SRCNAME = OGLESHelloAPI_LinuxX11
else
ifeq "$(WS)" "EWS"
SRCNAME = OGLESHelloAPI_EWS
else
ifeq "$(WS)" "Wayland"
SRCNAME = OGLESHelloAPI_LinuxWayland
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
	$(PLAT_CC) $(PLAT_CPPFLAGS) $(PLAT_CFLAGS) -c $(COMMON_INCLUDES) $(INCLUDES) $^ -o $@

clean:
	-rm -rf $(PLAT_OBJPATH) *.o


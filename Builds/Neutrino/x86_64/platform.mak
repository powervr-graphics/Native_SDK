#
# File			platform.mak
# Title			Platform specific makefile
# Author		PowerVR
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#

CROSS_COMPILE ?= ntox86_64-

ifdef TOOLCHAIN
PLAT_CC  = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)gcc
PLAT_CPP = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)g++
PLAT_AR  = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)ar
else
PLAT_CC  = $(CROSS_COMPILE)gcc
PLAT_CPP = $(CROSS_COMPILE)g++
PLAT_AR  = $(CROSS_COMPILE)ar
endif

PLAT_CFLAGS += -m64
PLAT_LINK += -m64

ifeq "$(shell OUTPUT=`$(PLAT_CPP) -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'`;if [ $$OUTPUT -ge 50400 ];then echo 1;else echo 0;fi;)" "1"
# Use gnu++11 for this version
PLAT_CFLAGS += -Wno-ignored-attributes
PLAT_CPPFLAGS += -std=gnu++11
else
PLAT_CFLAGS  += -D_GLIBCXX_USE_NANOSLEEP -D__EXT
PLAT_CPPFLAGS+= -std=c++11
endif



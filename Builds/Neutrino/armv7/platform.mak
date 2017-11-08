#
# File			platform.mak
# Title			Platform specific makefile
# Author		PowerVR
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#

CROSS_COMPILE ?= ntoarmv7-

ifdef TOOLCHAIN
PLAT_CC  = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)gcc
PLAT_CPP = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)g++
PLAT_AR  = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)ar
else
PLAT_CC  = $(CROSS_COMPILE)gcc
PLAT_CPP = $(CROSS_COMPILE)g++
PLAT_AR  = $(CROSS_COMPILE)ar
endif

PLAT_CFLAGS += -pipe
PLAT_LINK += -pipe

ifeq "$(shell OUTPUT=`$(PLAT_CPP) -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'`;if [ $$OUTPUT -ge 50400 ];then echo 1;else echo 0;fi;)" "1"
# Use gnu++11 for this version
PLAT_CFLAGS += -Wno-ignored-attributes
PLAT_CPPFLAGS += -std=gnu++11
else
PLAT_CFLAGS  += -D_GLIBCXX_USE_NANOSLEEP -D__EXT
PLAT_CPPFLAGS+= -std=c++11
endif

ifdef DINKUM
GCC_VERSION := $(shell $(PLAT_CC) -dumpversion)
GCC_LIB	:= $(shell $(PLAT_CC) -print-libgcc-file-name)

PLAT_CFLAGS += \
-nostdinc  \
-isystem $(QNX_TARGET)/usr/include \
-isystem $(QNX_TARGET)/usr/include/cpp/c \
-isystem $(QNX_TARGET)/usr/include/cpp

PLAT_CPPFLAGS += -nostdinc++

PLAT_LINK += \
--sysroot=$(QNX_TARGET)/armle-v7 \
-nodefaultlibs \
-L$(QNX_TARGET)/armle-v7/lib/gcc/$(GCC_VERSION) \
-lcpp \
-lc \
$(GCC_LIB)
endif


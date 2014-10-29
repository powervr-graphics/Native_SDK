#
# File			make_platform.mak
# Title			Platform specific makefile
# Author		PowerVR
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#
CROSS_COMPILE ?= mips-linux-gnu-

ifdef TOOLCHAIN
PLAT_CC  = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)gcc
PLAT_CPP = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)g++
PLAT_AR  = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)ar
else
PLAT_CC  = $(CROSS_COMPILE)gcc
PLAT_CPP = $(CROSS_COMPILE)g++
PLAT_AR  = $(CROSS_COMPILE)ar
endif

PLAT_CFLAGS += -march=mips32r2 -mabi=32 -EL
PLAT_LINK += -march=mips32r2 -mabi=32 -EL
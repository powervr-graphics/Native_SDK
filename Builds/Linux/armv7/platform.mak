#
# File			make_platform.mak
# Title			Platform specific makefile
# Author		PowerVR
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#

CROSS_COMPILE ?= arm-none-linux-gnueabi-

ifdef TOOLCHAIN
PLAT_CC  = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)gcc
PLAT_CPP = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)g++
PLAT_AR  = $(TOOLCHAIN)/bin/$(CROSS_COMPILE)ar
else
PLAT_CC  = $(CROSS_COMPILE)gcc
PLAT_CPP = $(CROSS_COMPILE)g++
PLAT_AR  = $(CROSS_COMPILE)ar
endif

ifneq (,$(filter PVRSHELL,$(DEPENDS)))
PLAT_CFLAGS += -DKEYPAD_INPUT="\"/dev/input/event0\""
endif

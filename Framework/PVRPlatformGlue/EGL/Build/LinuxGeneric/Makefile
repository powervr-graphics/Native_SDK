#
# File			Makefile
# Title			Makes PVRAssets
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#

#All the magic happens in $(SDKDIR)/Builds/Linux/make_framework.mak
#Required symbols: SDKDIR, OUTNAME, DIRECTORIES, OBJECTS
#OBJECTS can be omitted if COMPILE_ALL_CPP_IN_DIRECTORIES is set, in which case
#OBJECTS will be set to all .cpp files in the listed directories.

SDKDIR  = ../../../../..
OUTNAME = libPVREgl.a
VPATH = ../../..
BUILDTOAPP = ../../..

DIRECTORIES := EGL/ EGL/$(WS)

#This ... 
COMPILE_ALL_CPP_IN_DIRECTORIES := 1

#is equivalent to:
# OBJECTS := Model/Animation.o \

PLAT_INC     += $(SDKDIR)/Framework/PVRPlatformGlue/EGL

include $(SDKDIR)/Builds/Linux/make_framework.mak


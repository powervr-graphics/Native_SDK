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
OUTNAME = libPVRVulkanGlue.a
VPATH = ../../..
BUILDTOAPP = ../../..

DIRECTORIES := Vulkan/ Vulkan/$(WS)

#This ... 
COMPILE_ALL_CPP_IN_DIRECTORIES := 1

#is equivalent to:
# OBJECTS := Model/Animation.o \

PLAT_INC     += $(SDKDIR)/Framework/PVRPlatformGlue/Vulkan

include $(SDKDIR)/Builds/Linux/make_framework.mak


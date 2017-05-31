#
# File			Makefile
# Title			Makes PVRNativeGles
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#

#All the magic happens in $(SDKDIR)/Builds/Linux/make_framework.mak
#Required symbols: SDKDIR, OUTNAME, DIRECTORIES, OBJECTS
#OBJECTS can be omitted if COMPILE_ALL_CPP_IN_DIRECTORIES is set, in which case
#OBJECTS will be set to all .cpp files in the listed directories.

PLAT_CFLAGS += -Wno-deprecated-declarations

SDKDIR  = ../../../../..
OUTNAME = libPVRNativeGles.a
VPATH = ../../..
APIS = OGLES
BUILDTOAPP = ../../..

DIRECTORIES = OGLES/ EGL/

COMPILE_ALL_CPP_IN_DIRECTORIES:=1

include $(SDKDIR)/Builds/Linux/make_framework.mak

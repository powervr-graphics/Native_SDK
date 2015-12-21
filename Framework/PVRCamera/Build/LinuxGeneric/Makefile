#
# File			Makefile
# Title			Makes PVRCamera
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#

#All the magic happens in $(SDKDIR)/Builds/Linux/make_framework.mak
#Required symbols: SDKDIR, OUTNAME, DIRECTORIES, OBJECTS
#OBJECTS can be omitted if COMPILE_ALL_CPP_IN_DIRECTORIES is set, in which case
#OBJECTS will be set to all .cpp files in the listed directories.


SDKDIR  = ../../../..
OUTNAME = libPVRCamera.a
VPATH = ../..

#We only want the DUMMY version for linux. Otherwise, unused files will be compiled.
#COMPILE_ALL_CPP_IN_DIRECTORIES := 0

OBJECTS := CameraInterface_Dummy.o

include $(SDKDIR)/Builds/Linux/make_framework.mak

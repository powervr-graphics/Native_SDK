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


SDKDIR  = ../../../..
OUTNAME = libPVRAssets.a
VPATH = ../..

DIRECTORIES := FileIO/ Model/ Texture/ ../../External/pugixml

#This ... 
COMPILE_ALL_CPP_IN_DIRECTORIES := 1

#is equivalent to:
# OBJECTS := Model/Animation.o \
           # Model/Camera.o \
           # Model/Light.o \
           # Model/Mesh.o \
           # Model/Model.o \
           # Texture/MetaData.o \
           # Texture/PixelFormat.o \
           # Texture/PVRTDecompress.o \
           # Texture/Texture.o \
           # Texture/TextureHeader.o \
           # Texture/TextureHeaderWithMetaData.o \
           # FileIO/PODLoader.o \
           # FileIO/PaletteExpander.o \
           # FileIO/TextureReaderDDS.o \
           # FileIO/TextureWriterDDS.o \
           # FileIO/TextureReaderKTX.o \
           # FileIO/TextureWriterKTX.o \
           # FileIO/TextureReaderPVR.o \
           # FileIO/TextureWriterPVR.o \
           # FileIO/TextureReaderBMP.o \
           # FileIO/TextureReaderTGA.o \
           # ShadowVolume.o \
		   # Volume.o \

include $(SDKDIR)/Builds/Linux/make_framework.mak

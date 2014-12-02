#
# File			Makefile
# Title			Example makefile
#
# Copyright		Copyright (C) Imagination Technologies Limited.
#

.PHONY: clean

SDKDIR  = ../../../../../..
OUTNAME = OGLES2MultiThreading
APIS    = OGLES2
DEPENDS = PVRSHELL PVRTOOLS

OBJECTS +=	\
           OGLES2MultiThreading_Posix.o

LINK +=	\
       -L../..	\
       -lpthread

include $(SDKDIR)/Builds/Linux/make_example.mak


#
# File			Makefile
# Title			Makes PVRShell
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#

SDKDIR  = ../../../..
OUTNAME = libPVRShell.a
VPATH = ../..

INCLUDES += -I$(SDKDIR)/Framework/PVRShell/OS/$(WS)/

DIRECTORIES += EntryPoint/main/ OS/$(WS)/

COMPILE_ALL_CPP_IN_DIRECTORIES := 1

# OBJECTS += \
    # CommandLine.o \
    # EventManager.o \
    # Shell.o \
    # StateMachine.o \
	# EntryPoint/main/main.o\
	# OS/$(WS)/ShellOS.o

include $(SDKDIR)/Builds/Linux/make_framework.mak

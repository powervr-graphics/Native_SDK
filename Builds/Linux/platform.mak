
ifdef SDKDIR
SDKDIR   := $(shell cd $(SDKDIR) && pwd)
endif

ifdef Debug
DEBUG_RELEASE = Debug
PLAT_CFLAGS   += -DDEBUG -g
else
DEBUG_RELEASE = Release
PLAT_CFLAGS   += -DRELEASE -O2
endif

PLAT_SUFFIX = $(patsubst Linux_%,%,$(PLATFORM))
include $(SDKDIR)/Builds/Linux/$(PLAT_SUFFIX)/platform.mak
include $(SDKDIR)/Builds/Linux/$(PLAT_SUFFIX)/api.mak

ifndef WS
ifeq "$(X11BUILD)" "1"
ifndef X11ROOT
$(error When building an X11 BUILD you must set X11ROOT to point at the location where your X11 headers and libs can be found.)
endif

ifeq "$(EWSBUILD)" "1"
$(error Cannot have both X11BUILD and EWSBUILD enabled at the same time)
endif
endif

include $(SDKDIR)/Builds/Linux/$(PLAT_SUFFIX)/ws.mak
endif

PLAT_OBJPATH ?= ../$(PLATFORM)/$(DEBUG_RELEASE)$(WS)

API_INC ?= $(APIS)
API_LINK ?= $(APIS)

ifneq (,$(filter OGL,$(APIS)))

ifneq (,$(filter OGL,$(API_LINK)))
PLAT_LINK 	+= -lGL -lpthread
endif

ifneq (,$(filter OGL,$(API_INC)))
PLAT_INC  	+= $(SDKDIR)/Builds/Include
endif

PLAT_CFLAGS += -DBUILD_OGL

ifeq "$(EGLBUILD)" "1"

ifneq (,$(filter OGL,$(API_LINK)))
PLAT_LINK += -lEGL
endif

PLAT_CFLAGS += -DBUILD_EGL
endif

ifneq (,$(filter PVRTOOLS,$(DEPENDS)))
PLAT_LINK += -L$(SDKDIR)/Tools/OGL/Build/LinuxGeneric/$(PLAT_OBJPATH) -logltools
PLAT_INC  += $(SDKDIR)/Tools/OGL

.PHONY: libogltools
build_tools:  libogltools

libogltools:
ifeq "$(MAKECMDGOALS)" "clean"
	$(MAKE) clean -C $(SDKDIR)/Tools/OGL/Build/LinuxGeneric/
else
	$(MAKE) -C $(SDKDIR)/Tools/OGL/Build/LinuxGeneric/
endif

endif
endif

ifneq (,$(filter OGLES,$(APIS)))

ifneq (,$(filter OGLES,$(API_LINK)))

ifndef V1BUILD
PLAT_LINK 	+= -lGLES_CM
else
PLAT_LINK	+= -lGLESv1_CM -lEGL
endif

endif

ifneq (,$(filter OGLES,$(API_INC)))
PLAT_INC  	+= $(SDKDIR)/Builds/Include
endif

PLAT_CFLAGS += -DBUILD_OGLES

ifneq (,$(filter PVRTOOLS,$(DEPENDS)))
PLAT_LINK += -L$(SDKDIR)/Tools/OGLES/Build/LinuxGeneric/$(PLAT_OBJPATH) -loglestools
PLAT_INC  += $(SDKDIR)/Tools/OGLES

.PHONY: liboglestools
build_tools:  liboglestools

liboglestools:
ifeq "$(MAKECMDGOALS)" "clean"
	$(MAKE) clean -C $(SDKDIR)/Tools/OGLES/Build/LinuxGeneric/
else
	$(MAKE) -C $(SDKDIR)/Tools/OGLES/Build/LinuxGeneric/
endif
	
endif
endif

ifneq (,$(filter OGLES2,$(APIS)))

ifneq (,$(filter OGLES2,$(API_LINK)))
PLAT_LINK 	+= -lGLESv2 -lEGL
endif

ifneq (,$(filter OGLES2,$(API_INC)))
PLAT_INC  	+= $(SDKDIR)/Builds/Include
endif

PLAT_CFLAGS += -DBUILD_OGLES2

ifneq (,$(filter PVRTOOLS,$(DEPENDS)))
PLAT_LINK += -L$(SDKDIR)/Tools/OGLES2/Build/LinuxGeneric/$(PLAT_OBJPATH) -logles2tools
PLAT_INC  += $(SDKDIR)/Tools/OGLES2

.PHONY: libogles2tools
build_tools:  libogles2tools

libogles2tools:
ifeq "$(MAKECMDGOALS)" "clean"
	$(MAKE) clean -C $(SDKDIR)/Tools/OGLES2/Build/LinuxGeneric/
else
	$(MAKE) -C $(SDKDIR)/Tools/OGLES2/Build/LinuxGeneric/
endif
	
endif
endif

ifneq (,$(filter OGLES3 OGLES31,$(APIS)))

ifneq (,$(filter OGLES3 OGLES31,$(API_LINK)))
PLAT_LINK 	+= -lGLESv2 -lEGL
endif

ifneq (,$(filter OGLES3 OGLES31,$(API_INC)))
PLAT_INC  	+= $(SDKDIR)/Builds/Include
endif

PLAT_CFLAGS += -DBUILD_OGLES3

ifneq (,$(filter OGLES31,$(APIS)))
PLAT_CFLAGS += -DBUILD_OGLES31
endif

ifneq (,$(filter PVRTOOLS,$(DEPENDS)))
PLAT_LINK += -L$(SDKDIR)/Tools/OGLES3/Build/LinuxGeneric/$(PLAT_OBJPATH) -logles3tools
PLAT_INC  += $(SDKDIR)/Tools/OGLES3 $(SDKDIR)/Tools/OGLES2

.PHONY: libogles3tools
build_tools:  libogles3tools

libogles3tools:
ifeq "$(MAKECMDGOALS)" "clean"
	$(MAKE) clean -C $(SDKDIR)/Tools/OGLES3/Build/LinuxGeneric/
else
	$(MAKE) -C $(SDKDIR)/Tools/OGLES3/Build/LinuxGeneric/
endif
	
endif
endif

ifneq (,$(filter OCL,$(APIS)))

ifneq (,$(filter OCL,$(API_LINK)))
PLAT_LINK 	+= -lOpenCL
endif

ifneq (,$(filter OCL,$(API_INC)))
PLAT_INC  	+= $(SDKDIR)/Builds/Include
endif

PLAT_CFLAGS += -DBUILD_OCL

ifneq (,$(filter PVRTOOLS,$(DEPENDS)))
PLAT_LINK += -L$(SDKDIR)/Tools/OCL/Build/LinuxGeneric/$(PLAT_OBJPATH) -locltools
PLAT_INC  += $(SDKDIR)/Tools/OCL

.PHONY: libocltools
build_tools:  libocltools

libocltools:
ifeq "$(MAKECMDGOALS)" "clean"
	$(MAKE) clean -C $(SDKDIR)/Tools/OCL/Build/LinuxGeneric/
else
	$(MAKE) -C $(SDKDIR)/Tools/OCL/Build/LinuxGeneric/
endif
	
endif
endif

ifneq (,$(filter PVRSHELL,$(DEPENDS)))
SHELLOS ?= Linux$(WS)

ifndef SHELLAPI
$(error Please define SHELLAPI.)
endif

OBJECTS += PVRShell.o PVRShellAPI.o PVRShellOS.o	
PLAT_INC += $(SDKDIR)/Shell $(SDKDIR)/Shell/OS/$(SHELLOS) $(SDKDIR)/Shell/API/$(SHELLAPI)
VPATH += $(SDKDIR)/Shell $(SDKDIR)/Shell/OS/$(SHELLOS) $(SDKDIR)/Shell/API/$(SHELLAPI)
endif

ifdef LIBDIR
PLAT_LINK += -L$(LIBDIR) -Wl,--rpath-link,$(LIBDIR)
endif

# Remove any duplicates
PLAT_INC 	:= $(sort $(PLAT_INC))
PLAT_CFLAGS := $(sort $(PLAT_CFLAGS))

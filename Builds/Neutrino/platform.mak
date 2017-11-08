#Define a newline for future use
define nl

endef

message = \
	$(info $(nl)**********************************************************************)\
	$(info $(nl)**  $(1)) \
	$(info $(nl)**********************************************************************$(nl))
ifdef Debug
DEBUG_RELEASE = Debug
PLAT_CFLAGS   += -DDEBUG -g
else
DEBUG_RELEASE = Release
PLAT_CFLAGS   += -DRELEASE -O2
endif
PLAT_CFLAGS   += -Wno-psabi

PLAT_SUFFIX = $(patsubst Neutrino_%,%,$(PLATFORM))
include $(SDKDIR)/Builds/Neutrino/$(PLAT_SUFFIX)/platform.mak

API_INC ?= $(APIS)
API_LINK ?= 

PLAT_CPPFLAGS += -Wno-deprecated-declarations

ifndef WS
ifeq "$(X11BUILD)" "1"
ifndef X11ROOT
X11ROOT ?= /usr
$(info ***** Warning -- X11 BUILD is set, but X11ROOT is not pointing at the location of your X11 headers and libs. Defaulting to "/usr/")
endif

ifeq "$(EWSBUILD)" "1"
$(error Cannot have both X11BUILD and EWSBUILD enabled at the same time)
endif
endif

ifeq "$(DRMBUILD)" "1"
ifndef DRMROOT
$(error When building a DRM BUILD you must set DRMROOT to point at the location where your Direct Rendering Manager headers and libs can be found.)
endif
endif

include $(SDKDIR)/Builds/Neutrino/$(PLAT_SUFFIX)/ws.mak
endif

PLAT_OBJPATH ?= ../$(PLATFORM)/$(DEBUG_RELEASE)$(WS)
PLAT_INC  	 += $(SDKDIR)/Builds/Include
PLAT_INC     += $(SDKDIR)/Framework
PLAT_INC     += ../../..

#BUILDING AND LINKING PROJECT DEPENDENCIES: PVRCore, PVRAssets, PVRShell, PVR[API], PVRUtils

.PHONY: clean cleanexample $(CLEAN_FRAMEWORK)

#Link in Assets and, finally Core
LINK += $(addprefix -L,$(LIBPATHS))
LINK += $(addprefix -l,$(LIBRARIES))
PLAT_LINK += $(WS_LIBS)

PLAT_FRAMEWORKLIBPATH ?= $(SDKDIR)/Framework/Bin/$(PLATFORM)/$(DEBUG_RELEASE)$(WS)
PVRCore:
	$(MAKE) -C $(SDKDIR)/Framework/PVRCore/Build/NeutrinoGeneric/
PVRAssets: 
	$(MAKE) -C $(SDKDIR)/Framework/PVRAssets/Build/NeutrinoGeneric/
PVRShell:
	$(MAKE) -C $(SDKDIR)/Framework/PVRShell/Build/NeutrinoGeneric/
PVRVk:
	$(MAKE) -C $(SDKDIR)/Framework/PVRVk/Build/NeutrinoGeneric/
PVRVkRT:
	$(MAKE) -C $(SDKDIR)/Framework/PVRVk/BuildRT/NeutrinoGeneric/
PVRUtilsGles:
	$(MAKE) -C $(SDKDIR)/Framework/PVRUtils/OGLES/Build/NeutrinoGeneric/
PVRUtilsGlesRT:
	$(MAKE) -C $(SDKDIR)/Framework/PVRUtils/OGLESRT/Build/NeutrinoGeneric/
PVRUtilsVk:
	$(MAKE) -C $(SDKDIR)/Framework/PVRUtils/Vulkan/Build/NeutrinoGeneric/
PVRUtilsVkRT:
	$(MAKE) -C $(SDKDIR)/Framework/PVRUtils/Vulkan/BuildRT/NeutrinoGeneric/
PVRCamera:
	$(MAKE) -C $(SDKDIR)/Framework/PVRCamera/Build/NeutrinoGeneric/

CLEAN_PVRCore:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRCore/Build/NeutrinoGeneric/
CLEAN_PVRAssets:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRAssets/Build/NeutrinoGeneric/
CLEAN_PVRShell:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRShell/Build/NeutrinoGeneric/
CLEAN_PVRVk:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRVk/Build/NeutrinoGeneric/
CLEAN_PVRVkRT:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRVk/BuildRT/NeutrinoGeneric/	
CLEAN_PVRUtilsGles:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRUtils/OGLES/Build/NeutrinoGeneric/
CLEAN_PVRUtilsGlesRT:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRUtils/OGLESRT/Build/NeutrinoGeneric
CLEAN_PVRUtilsVk:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRUtils/Vulkan/Build/NeutrinoGeneric/
CLEAN_PVRUtilsVkRT:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRUtils/Vulkan/BuildRT/NeutrinoGeneric/
CLEAN_PVRCamera:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRCamera/Build/NeutrinoGeneric/


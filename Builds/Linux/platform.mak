#Define a newline for future use
define nl

endef

message = \
	$(info $(nl)**********************************************************************)\
	$(info $(nl)**  $(1)) \
	$(info $(nl)**********************************************************************$(nl))
	
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
PLAT_CFLAGS   += -Wno-psabi

PLAT_SUFFIX = $(patsubst Linux_%,%,$(PLATFORM))
include $(SDKDIR)/Builds/Linux/$(PLAT_SUFFIX)/platform.mak
include $(SDKDIR)/Builds/Linux/$(PLAT_SUFFIX)/api.mak

API_INC ?= $(APIS)

#enable C++11 if available
GCC_CPP11 = $(shell OUTPUT=`$(PLAT_CPP) -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'`;if [ $$OUTPUT -gt 40600 ];then echo 1;else echo 0;fi;)

PLAT_CPPFLAGS += -Wno-deprecated-declarations

ifeq "$(GCC_CPP11)" "1"
PLAT_CPPFLAGS+= -std=c++11
else
PLAT_CPPFLAGS+= -std=c++0x
endif

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

include $(SDKDIR)/Builds/Linux/$(PLAT_SUFFIX)/ws.mak
endif

PLAT_OBJPATH ?= ../$(PLATFORM)/$(DEBUG_RELEASE)$(WS)
PLAT_INC  	 += $(SDKDIR)/Builds/Include
PLAT_INC     += $(SDKDIR)/Framework
PLAT_INC     += ../../..

ifneq (,$(filter OGLES,$(APIS)))
ifneq (,$(filter OGLES,$(API_LINK)))
ifndef V1BUILD
PLAT_LINK 	+= -lGLES_CM
else
PLAT_LINK	+= -lGLESv1_CM -lEGL
endif
endif
PLAT_CFLAGS += -DBUILD_OGLES
endif

ifneq (,$(filter OGLES2,$(APIS)))
ifneq (,$(filter OGLES2,$(API_LINK)))
PLAT_LINK 	+= -lGLESv2 -lEGL
endif
PLAT_CFLAGS += -DBUILD_OGLES2
endif

ifneq (,$(filter OGLES3,$(APIS)))
ifneq (,$(filter OGLES3,$(API_LINK)))
PLAT_LINK 	+= -lGLESv2 -lEGL
endif

PLAT_CFLAGS += -DBUILD_OGLES3
endif

ifneq (,$(filter OCL,$(APIS)))
ifneq (,$(filter OCL,$(API_LINK)))
PLAT_LINK 	+= -lOpenCL
endif
PLAT_CFLAGS += -DBUILD_OCL
endif

ifneq (,$(filter VULKAN,$(APIS)))
ifneq (,$(filter VULKAN,$(API_LINK)))
PLAT_LINK 	+= -lvulkan
endif
PLAT_CFLAGS += -DBUILD_VULKAN
endif

#BUILDING AND LINKING PROJECT DEPENDENCIES: PVRCore, PVRAssets, PVRShell, PVR[API], PVRUIRenderer

.PHONY: clean cleanexample $(CLEAN_FRAMEWORK)

#Link in Assets and, finally Core
LINK += $(addprefix -L,$(LIBPATHS))
LINK += $(addprefix -l,$(LIBRARIES))
PLAT_LINK += $(WS_LIBS) -lpthread -lrt -ldl

PLAT_FRAMEWORKLIBPATH ?= $(SDKDIR)/Framework/Bin/$(PLATFORM)/$(DEBUG_RELEASE)$(WS)
PVRCore:
	$(MAKE) -C $(SDKDIR)/Framework/PVRCore/Build/LinuxGeneric/
PVRAssets: 
	$(MAKE) -C $(SDKDIR)/Framework/PVRAssets/Build/LinuxGeneric/
PVRShell:
	$(MAKE) -C $(SDKDIR)/Framework/PVRShell/Build/LinuxGeneric/
PVRGles:
	$(MAKE) -C $(SDKDIR)/Framework/PVRApi/OGLES/Build/LinuxGeneric/
PVRVulkan:
	$(MAKE) -C $(SDKDIR)/Framework/PVRApi/Vulkan/Build/LinuxGeneric/
PVRNativeVulkan:
	$(MAKE) -C $(SDKDIR)/Framework/PVRNativeApi/Vulkan/Build/LinuxGeneric/
PVRVulkanGlue:
	$(MAKE) -C $(SDKDIR)/Framework/PVRPlatformGlue/Vulkan/Build/LinuxGeneric/
PVRNativeGlesRT:
	$(MAKE) -C $(SDKDIR)/Framework/PVRNativeApi/OGLESRT/Build/LinuxGeneric/
PVRNativeGles:
	$(MAKE) -C $(SDKDIR)/Framework/PVRNativeApi/OGLES/Build/LinuxGeneric/
PVREgl:
	$(MAKE) -C $(SDKDIR)/Framework/PVRPlatformGlue/EGL/Build/LinuxGeneric/
PVRUIRenderer:
	$(MAKE) -C $(SDKDIR)/Framework/PVRUIRenderer/Build/LinuxGeneric/
PVRCamera:
	$(MAKE) -C $(SDKDIR)/Framework/PVRCamera/Build/LinuxGeneric/

CLEAN_PVRCore:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRCore/Build/LinuxGeneric/
CLEAN_PVRAssets:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRAssets/Build/LinuxGeneric/
CLEAN_PVRShell:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRShell/Build/LinuxGeneric/
CLEAN_PVRGles:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRApi/OGLES/Build/LinuxGeneric/
CLEAN_PVRVulkan:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRApi/Vulkan/Build/LinuxGeneric/
CLEAN_PVRNativeVulkan:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRNativeApi/Vulkan/Build/LinuxGeneric/
CLEAN_PVRVulkanGlue:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRPlatformGlue/Vulkan/Build/LinuxGeneric/
CLEAN_PVRNativeGlesRT:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRNativeApi/OGLESRT/Build/LinuxGeneric/
CLEAN_PVRNativeGles:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRNativeApi/OGLES/Build/LinuxGeneric/
CLEAN_PVREgl:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRPlatformGlue/EGL/Build/LinuxGeneric/
CLEAN_PVRUIRenderer:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRUIRenderer/Build/LinuxGeneric/
CLEAN_PVRCamera:
	$(MAKE) clean -C $(SDKDIR)/Framework/PVRCamera/Build/LinuxGeneric/

ifdef LIBDIR
PLAT_LINK := -L$(LIBDIR) -Wl,--rpath-link,$(LIBDIR) $(PLAT_LINK)
endif

# Remove any duplicates
PLAT_INC 	:= $(sort $(PLAT_INC))
PLAT_CFLAGS := $(sort $(PLAT_CFLAGS))

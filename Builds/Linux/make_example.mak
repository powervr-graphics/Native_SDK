#
# File			make_example.mak
# Title			Used to build an example
# Author		PowerVR
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#
#---------------------------------------------------------------------

.PHONY: all print_info build_tools wrap_content_files clean

ifndef PLATFORM
$(error Error building application. You must define the PLATFORM variable to be the value of the target platform you want to build for. )
endif

all: print_info

include $(SDKDIR)/Builds/Linux/platform.mak

SHELLOS ?= Linux$(WS)
SHELLAPI ?= KEGL

SHELLOSPATH = $(SDKDIR)/Shell/OS/$(SHELLOS)
SHELLAPIPATH = $(SDKDIR)/Shell/API/$(SHELLAPI)

OBJECTS := $(addprefix $(PLAT_OBJPATH)/, $(OBJECTS))
SOURCE_DEPENDENCIES := $(OBJECTS:.o=.d)

INCLUDES += -I$(SDKDIR)/Builds/Include 	\
			$(addprefix -I, $(PLAT_INC))
			
INCLUDES := $(sort $(INCLUDES))
			
VPATH += ../..             : \
	 ../../..

ARCH := $(shell getconf LONG_BIT)
FILEWRAP_PATH =  $(SDKDIR)/Utilities/Filewrap/Linux_x86_$(ARCH)/Filewrap

#---------------------------------------------------------------------

all: $(PLAT_OBJPATH)/$(OUTNAME)

$(PLAT_OBJPATH)/$(OUTNAME) : build_tools $(OBJECTS) 
	$(PLAT_CPP) -o $@ $(OBJECTS) $(LINK) $(PLAT_LINK)

$(PLAT_OBJPATH)/%.o: %.c
	mkdir -p $(PLAT_OBJPATH)
# Magically generate dependency rules from:
# http://stackoverflow.com/questions/97338/gcc-dependency-generation-for-a-different-output-directory
# combined with "Tromey's Way" from 'Managing Projects with GNU Make'
	$(PLAT_CC) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CC) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@

$(PLAT_OBJPATH)/%.o: %.cpp
	mkdir -p $(PLAT_OBJPATH)
	$(PLAT_CPP) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@
	
print_info:
	@echo ""
	@echo "******************************************************"
	@echo "*"
	@echo "* Name:         $(OUTNAME)"
	@echo "* PWD:          $(shell pwd)"
	@echo "* MAKECMDGOALS: $(MAKECMDGOALS)"
	@echo "* Binary path:  $(shell pwd)/$(PLAT_OBJPATH)"
	@echo "* Library path: $(shell cd $(LIBDIR) && pwd)"
	@echo "* WS:           $(WS)"
	@echo "*"
	@echo "******************************************************"

build_tools: 

wrap_content_files:
ifeq "$(MAKECMDGOALS)" "clean"
	test -f ../../content.mak && $(MAKE) clean -C ../.. -f content.mak || true
else
	test -f ../../content.mak && $(MAKE) -C ../.. -f content.mak FILEWRAP=$(FILEWRAP_PATH) || true
endif
			
clean: print_info wrap_content_files build_tools
	@for i in $(OBJECTS) $(SOURCE_DEPENDENCIES) $(PLAT_OBJPATH)/$(OUTNAME); do test -f $$i && rm -vf $$i || true; done
	@if [ -d $(PLAT_OBJPATH) ]; then rmdir -vp $(PLAT_OBJPATH) --ignore-fail-on-non-empty; fi

ifneq "$(MAKECMDGOALS)" "clean"
sinclude $(SOURCE_DEPENDENCIES)
endif
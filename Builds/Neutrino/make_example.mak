#
# File			make_example.mak
# Title			Used to build an example
# Author		PowerVR
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#
#---------------------------------------------------------------------

.PHONY: all print_info clean cleanexample build_example

ifndef PLATFORM
$(error Error building application. You must define the PLATFORM variable to be the value of the target platform you want to build for. )
endif

all: print_info build_example

PERCENT=%

include $(SDKDIR)/Builds/Neutrino/platform.mak

OBJECTS_PREFIXED := $(addprefix $(PLAT_OBJPATH)/BuildTemp/, $(OBJECTS))
RESOURCES_PREFIXED := $(addprefix $(PLAT_OBJPATH)/Assets/, $(notdir $(RESOURCES)))
COMMON_RESOURCES_PREFIXED := $(addprefix $(PLAT_OBJPATH)/Assets/, $(notdir $(COMMON_RESOURCES)))
DIRECTORIES := $(addprefix $(PLAT_OBJPATH)/,$(DIRECTORIES))

FRAMEWORK_LIBS := $(addprefix $(PLAT_FRAMEWORKLIBPATH)/lib,$(addsuffix .a,$(DEPENDS)))
CLEAN_FRAMEWORK := $(addprefix CLEAN_,$(DEPENDS))

SOURCE_DEPENDENCIES := $(OBJECTS_PREFIXED:.o=.d)

INCLUDES += -I$(SDKDIR)/Builds/Include 	\
			$(addprefix -I, $(PLAT_INC))
			
INCLUDES := $(sort $(INCLUDES))
			
VPATH += ../../..

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
ARCH := $(shell getconf LONG_BIT)
endif

build_example: $(PLAT_OBJPATH)/$(OUTNAME) $(RESOURCES_PREFIXED) $(COMMON_RESOURCES_PREFIXED)

#--------------LINK THE FINAL BINARY--------------
$(PLAT_OBJPATH)/$(OUTNAME) : $(DEPENDS) $(OBJECTS_PREFIXED)
	$(call message, Linking Application: Binary is $(PLAT_OBJPATH)/$(OUTNAME))
	@mkdir -p $(@D)
	$(PLAT_CPP) -o $@ $(OBJECTS_PREFIXED) $(PLAT_LINK) $(LINK) $(PLAT_LINK)

#--------------COMPILE SOURCE FILES--------------
$(PLAT_OBJPATH)/BuildTemp/%.o: %.c
# Magically generate dependency rules from:
# http://stackoverflow.com/questions/97338/gcc-dependency-generation-for-a-different-output-directory
# combined with "Tromey's Way" from 'Managing Projects with GNU Make'
	@mkdir -p $(@D)
	$(PLAT_CC) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CC) -c $(PLAT_CFLAGS) $(INCLUDES) $< -o$@

$(PLAT_OBJPATH)/BuildTemp/%.o: %.cpp
	@mkdir -p $(@D)
	$(PLAT_CPP) $(PLAT_CPPFLAGS) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CPPFLAGS) $(PLAT_CFLAGS) $(INCLUDES) $< -o$@
	
#--------------COPY THE GENERIC ASSETS THAT ARE FOUND IN THE COMMON FOLDER "PROCESSASSETS"--------------
$(PLAT_OBJPATH)/Assets/%: $(abspath $(PLAT_OBJPATH)/../../../../ProcessedAssets)/%
	@mkdir -p $(@D)
	cp -f $(abspath $<) $(abspath $@)

#--------------COPY THE ASSETS THAT ARE FOUND IN THE PER-API FOLDER--------------
$(PLAT_OBJPATH)/Assets/%: $(abspath $(PLAT_OBJPATH)/../../..)/%
	@mkdir -p $(@D)
	cp -f $(abspath $<) $(abspath $@)

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

clean: cleanexample $(CLEAN_FRAMEWORK)

cleanexample: print_info
	@for i in $(OBJECTS_PREFIXED) $(SOURCE_DEPENDENCIES) $(RESOURCES) $(COMMON_RESOURCES) $(PLAT_OBJPATH)/$(OUTNAME); do test -f $$i && rm -vf $$i || true; done
	@for i in $(OBJECTS_PREFIXED) $(SOURCE_DEPENDENCIES) $(RESOURCES_PREFIXED) $(COMMON_RESOURCES_PREFIXED) $(PLAT_OBJPATH)/$(OUTNAME); do test -f $$i && rm -vf $$i || true; done
	@for i in $(filter-out ./,$(sort $(dir $(OBJECTS_PREFIXED) $(SOURCE_DEPENDENCIES) $(RESOURCES_PREFIXED) $(COMMON_RESOURCES_PREFIXED) $(PLAT_OBJPATH)/$(OUTNAME)))); do if [ -d $$i ]; then rmdir -vp $$i --ignore-fail-on-non-empty; fi; done
	@for i in $(DIRECTORIES); do if [ -d $$i ]; then rmdir -vp $$i --ignore-fail-on-non-empty; fi; done
	@if [ -d $(PLAT_OBJPATH) ]; then rmdir -vp $(PLAT_OBJPATH) --ignore-fail-on-non-empty; fi

ifneq "$(MAKECMDGOALS)" "clean"
sinclude $(SOURCE_DEPENDENCIES)
endif

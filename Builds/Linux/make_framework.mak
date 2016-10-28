#
# File			make_framework.mak
# Title			Used to make a framework project
# Author		PowerVR
#
# Copyright		Copyright (C) by Imagination Technologies Limited.
#
#---------------------------------------------------------------------

.PHONY: clean build_framework
all: build_framework

include $(SDKDIR)/Builds/Linux/platform.mak

BUILDTOAPP ?= ../..

#If COMPILE_ALL_CPP_IN_DIRECTORIES is defined then 
ifdef COMPILE_ALL_CPP_IN_DIRECTORIES
COMPILE_SOURCES_FROM_DIRECTORIES := $(DIRECTORIES)
endif

DIRECTORIES := $(PLAT_OBJPATH)/BuildTemp/ $(addprefix $(PLAT_OBJPATH)/BuildTemp/, $(DIRECTORIES))

#BUT it may also have been defined directly as a list of objects, relative to the project folder, so append, do not replace
ifdef COMPILE_ALL_CPP_IN_DIRECTORIES
SRCs := $(patsubst %.cpp, %.o, $(wildcard $(BUILDTOAPP)/*.cpp))
endif

SKIP_FILES?=
ifneq ($(SKIP_FILES),)
SKIP_FILES:=$(patsubst %.cpp, %.o, $(SKIP_FILES))
SRCs += $(filter-out $(addprefix $(BUILDTOAPP)/,$(SKIP_FILES)), $(foreach directory, $(COMPILE_SOURCES_FROM_DIRECTORIES), $(patsubst %.cpp, %.o, $(wildcard $(BUILDTOAPP)/$(directory)*.cpp))))
else
SRCs += $(foreach directory, $(COMPILE_SOURCES_FROM_DIRECTORIES), $(patsubst %.cpp, %.o, $(wildcard $(BUILDTOAPP)/$(directory)*.cpp)))
endif

OBJECTS += $(SRCs:$(BUILDTOAPP)/%=%)

#Bring it into the required form and path, relative to the makefile.
OBJECTS := $(addprefix $(PLAT_OBJPATH)/BuildTemp/, $(OBJECTS))

DIRECTORIES = $(sort $(dir $(OBJECTS)))

INCLUDES += -I$(SDKDIR)/Framework/ $(addprefix -I, $(PLAT_INC))

SOURCE_DEPENDENCIES := $(OBJECTS:.o=.d)

################## ACTUAL BUILDING RULES ##################
build_framework: $(PLAT_FRAMEWORKLIBPATH)/$(OUTNAME)

$(PLAT_FRAMEWORKLIBPATH)/$(OUTNAME): $(PLAT_OBJPATH)/$(OUTNAME)
	@mkdir -p $(@D)
	cp -f "$<" "$(PLAT_FRAMEWORKLIBPATH)/$(OUTNAME)"

$(PLAT_OBJPATH)/$(OUTNAME): $(OBJECTS)
	$(PLAT_AR) -r $@ $(OBJECTS)

$(PLAT_OBJPATH)/BuildTemp/External/%.o: ../../../../External/%.cpp
	@mkdir -p $(@D)
	$(PLAT_CPP) $(PLAT_CPPFLAGS) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CPPFLAGS) $(PLAT_CFLAGS) $(INCLUDES)  $< -o$@

$(PLAT_OBJPATH)/BuildTemp/%.o: %.cpp
	@mkdir -p $(@D)
	$(PLAT_CPP) $(PLAT_CPPFLAGS) $(PLAT_CFLAGS) $(INCLUDES) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)" -MT"$@" "$<"
	$(PLAT_CPP) -c $(PLAT_CPPFLAGS) $(PLAT_CFLAGS) $(INCLUDES)  $< -o$@

clean:
	@for i in $(OBJECTS) $(SOURCE_DEPENDENCIES) $(PLAT_OBJPATH)/$(OUTNAME) $(PLAT_FRAMEWORKLIBPATH)/$(OUTNAME); do test -f $$i && rm -vf $$i || true; done
	@for i in $(DIRECTORIES) $(PLAT_FRAMEWORKLIBPATH) $(PLAT_OBJPATH); do if [ -d $$i ]; then rmdir -vp $$i --ignore-fail-on-non-empty; fi; done

ifneq "$(MAKECMDGOALS)" "clean"
sinclude $(SOURCE_DEPENDENCIES)
endif
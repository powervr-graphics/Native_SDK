ifneq (,$(filter OGLES OGLES2 OGLES3 OGLES31 OCL,$(APIS)))
LIBDIR ?= "$(SDKDIR)/Builds/Linux/armv7hf/Lib"
endif

ifneq (,$(filter PVRSHELL,$(DEPENDS)))
SHELLAPI ?= KEGL
endif
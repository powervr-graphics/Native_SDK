ifneq (,$(filter OGLES OGLES2 OGLES3 OGLES31 OCL,$(APIS)))
LIBDIR ?= "$(SDKDIR)/Builds/Linux/armv8_64/Lib"
endif

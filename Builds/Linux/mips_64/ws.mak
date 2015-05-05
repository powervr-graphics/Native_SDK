ifeq "$(X11BUILD)" "1"

WS_LIBS = -L$(X11ROOT)/lib -lX11 -lXau
WS_INC  = $(X11ROOT)/include
WS = X11
else

ifeq "$(EWSBUILD)" "1"

PLAT_CFLAGS += -DEWS
WS_LIBS = -lews
WS_INC =
WS=EWS
LIBDIR ?= "$(SDKDIR)/Builds/Linux/mips_64/Lib"

else

ifeq "$(DRMBUILD)" "1"

WS_LIBS = -L$(DRMROOT)/lib -ldrm -lgbm -ludev -ldl -Wl,--rpath-link,$(DRMROOT)/lib
WS_INC = $(DRMROOT)/include $(DRMROOT)/include/libdrm $(DRMROOT)/include/gbm
WS=DRM

else

WS_LIBS =
WS_INC  =
WS = NullWS

endif

endif

endif

PLAT_LINK += $(WS_LIBS)
PLAT_INC  += $(WS_INC)
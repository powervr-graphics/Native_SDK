ifeq "$(SCREENBUILD)" "1"

WS_LIBS = -lscreen
WS_INC =
WS = Screen

else

WS_LIBS =
WS_INC  =
WS = NullWS

endif

PLAT_LINK += $(WS_LIBS)
PLAT_INC  += $(WS_INC)

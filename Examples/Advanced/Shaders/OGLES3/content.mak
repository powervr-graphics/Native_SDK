#--------------------------------------------------------------------------
# Name         : content.mak
# Title        : Makefile to build content files
#
# Copyright    : Copyright (C) by Imagination Technologies Limited.
#
# Description  : Makefile to wrap content files for examples in the PowerVR SDK
#
# Platform     :
#
#--------------------------------------------------------------------------

#############################################################################
## Variables
#############################################################################
FILEWRAP 	= ..\..\..\..\Utilities\Filewrap\Windows_x86_32\Filewrap.exe
CONTENTDIR = Content

#############################################################################
## Instructions
#############################################################################

RESOURCES = \
	$(CONTENTDIR)/Basetex.cpp \
	$(CONTENTDIR)/Reflection.cpp \
	$(CONTENTDIR)/Cubemap.cpp \
	$(CONTENTDIR)/AnisoMap.cpp \
	$(CONTENTDIR)/anisotropic_lighting.cpp \
	$(CONTENTDIR)/directional_lighting.cpp \
	$(CONTENTDIR)/envmap.cpp \
	$(CONTENTDIR)/fasttnl.cpp \
	$(CONTENTDIR)/lattice.cpp \
	$(CONTENTDIR)/phong_lighting.cpp \
	$(CONTENTDIR)/point_lighting.cpp \
	$(CONTENTDIR)/reflections.cpp \
	$(CONTENTDIR)/simple.cpp \
	$(CONTENTDIR)/spot_lighting.cpp \
	$(CONTENTDIR)/toon.cpp \
	$(CONTENTDIR)/vertex_sine.cpp \
	$(CONTENTDIR)/wood.cpp

all: resources
	
help:
	@echo Valid targets are:
	@echo resources, clean
	@echo FILEWRAP can be used to override the default path to the Filewrap utility.

clean:
	@for i in $(RESOURCES); do test -f $$i && rm -vf $$i || true; done

resources: $(RESOURCES)

$(CONTENTDIR):
	-mkdir "$@"

$(CONTENTDIR)/Basetex.cpp: $(CONTENTDIR) ./Basetex.pvr
	$(FILEWRAP)  -o $@ ./Basetex.pvr

$(CONTENTDIR)/Reflection.cpp: $(CONTENTDIR) ./Reflection.pvr
	$(FILEWRAP)  -o $@ ./Reflection.pvr

$(CONTENTDIR)/Cubemap.cpp: $(CONTENTDIR) ./Cubemap.pvr
	$(FILEWRAP)  -o $@ ./Cubemap.pvr

$(CONTENTDIR)/AnisoMap.cpp: $(CONTENTDIR) ./AnisoMap.pvr
	$(FILEWRAP)  -o $@ ./AnisoMap.pvr

$(CONTENTDIR)/anisotropic_lighting.cpp: $(CONTENTDIR) ./anisotropic_lighting.pfx
	$(FILEWRAP)  -s  -o $@ ./anisotropic_lighting.pfx

$(CONTENTDIR)/directional_lighting.cpp: $(CONTENTDIR) ./directional_lighting.pfx
	$(FILEWRAP)  -s  -o $@ ./directional_lighting.pfx

$(CONTENTDIR)/envmap.cpp: $(CONTENTDIR) ./envmap.pfx
	$(FILEWRAP)  -s  -o $@ ./envmap.pfx

$(CONTENTDIR)/fasttnl.cpp: $(CONTENTDIR) ./fasttnl.pfx
	$(FILEWRAP)  -s  -o $@ ./fasttnl.pfx

$(CONTENTDIR)/lattice.cpp: $(CONTENTDIR) ./lattice.pfx
	$(FILEWRAP)  -s  -o $@ ./lattice.pfx

$(CONTENTDIR)/phong_lighting.cpp: $(CONTENTDIR) ./phong_lighting.pfx
	$(FILEWRAP)  -s  -o $@ ./phong_lighting.pfx

$(CONTENTDIR)/point_lighting.cpp: $(CONTENTDIR) ./point_lighting.pfx
	$(FILEWRAP)  -s  -o $@ ./point_lighting.pfx

$(CONTENTDIR)/reflections.cpp: $(CONTENTDIR) ./reflections.pfx
	$(FILEWRAP)  -s  -o $@ ./reflections.pfx

$(CONTENTDIR)/simple.cpp: $(CONTENTDIR) ./simple.pfx
	$(FILEWRAP)  -s  -o $@ ./simple.pfx

$(CONTENTDIR)/spot_lighting.cpp: $(CONTENTDIR) ./spot_lighting.pfx
	$(FILEWRAP)  -s  -o $@ ./spot_lighting.pfx

$(CONTENTDIR)/toon.cpp: $(CONTENTDIR) ./toon.pfx
	$(FILEWRAP)  -s  -o $@ ./toon.pfx

$(CONTENTDIR)/vertex_sine.cpp: $(CONTENTDIR) ./vertex_sine.pfx
	$(FILEWRAP)  -s  -o $@ ./vertex_sine.pfx

$(CONTENTDIR)/wood.cpp: $(CONTENTDIR) ./wood.pfx
	$(FILEWRAP)  -s  -o $@ ./wood.pfx

############################################################################
# End of file (content.mak)
############################################################################

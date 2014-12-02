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

$(CONTENTDIR)/Basetex.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Basetex.pvr

$(CONTENTDIR)/Reflection.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Reflection.pvr

$(CONTENTDIR)/Cubemap.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Cubemap.pvr

$(CONTENTDIR)/AnisoMap.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./AnisoMap.pvr

$(CONTENTDIR)/anisotropic_lighting.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./anisotropic_lighting.pfx

$(CONTENTDIR)/directional_lighting.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./directional_lighting.pfx

$(CONTENTDIR)/envmap.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./envmap.pfx

$(CONTENTDIR)/fasttnl.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./fasttnl.pfx

$(CONTENTDIR)/lattice.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./lattice.pfx

$(CONTENTDIR)/phong_lighting.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./phong_lighting.pfx

$(CONTENTDIR)/point_lighting.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./point_lighting.pfx

$(CONTENTDIR)/reflections.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./reflections.pfx

$(CONTENTDIR)/simple.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./simple.pfx

$(CONTENTDIR)/spot_lighting.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./spot_lighting.pfx

$(CONTENTDIR)/toon.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./toon.pfx

$(CONTENTDIR)/vertex_sine.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./vertex_sine.pfx

$(CONTENTDIR)/wood.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./wood.pfx

############################################################################
# End of file (content.mak)
############################################################################

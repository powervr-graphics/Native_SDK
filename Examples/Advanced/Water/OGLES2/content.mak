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
	$(CONTENTDIR)/normalmap.cpp \
	$(CONTENTDIR)/skybox.cpp \
	$(CONTENTDIR)/galleon.cpp \
	$(CONTENTDIR)/wood.cpp \
	$(CONTENTDIR)/coins.cpp \
	$(CONTENTDIR)/coins-specular.cpp \
	$(CONTENTDIR)/flag.cpp \
	$(CONTENTDIR)/crate.cpp \
	$(CONTENTDIR)/galleon-sails.cpp \
	$(CONTENTDIR)/sand.cpp \
	$(CONTENTDIR)/palmleaf.cpp \
	$(CONTENTDIR)/FragShader.cpp \
	$(CONTENTDIR)/VertShader.cpp \
	$(CONTENTDIR)/SkyboxFShader.cpp \
	$(CONTENTDIR)/SkyboxVShader.cpp \
	$(CONTENTDIR)/ModelFShader.cpp \
	$(CONTENTDIR)/ModelVShader.cpp \
	$(CONTENTDIR)/Tex2DFShader.cpp \
	$(CONTENTDIR)/Tex2DVShader.cpp \
	$(CONTENTDIR)/PlaneTexFShader.cpp \
	$(CONTENTDIR)/PlaneTexVShader.cpp \
	$(CONTENTDIR)/Scene.cpp

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

$(CONTENTDIR)/normalmap.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./normalmap.pvr

$(CONTENTDIR)/skybox.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./skybox.pvr

$(CONTENTDIR)/galleon.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./galleon.pvr

$(CONTENTDIR)/wood.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./wood.pvr

$(CONTENTDIR)/coins.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./coins.pvr

$(CONTENTDIR)/coins-specular.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./coins-specular.pvr

$(CONTENTDIR)/flag.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./flag.pvr

$(CONTENTDIR)/crate.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./crate.pvr

$(CONTENTDIR)/galleon-sails.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./galleon-sails.pvr

$(CONTENTDIR)/sand.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./sand.pvr

$(CONTENTDIR)/palmleaf.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./palmleaf.pvr

$(CONTENTDIR)/FragShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./FragShader.fsh
	-$(FILEWRAP)  -oa $@ ./FragShader.fsc

$(CONTENTDIR)/VertShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./VertShader.vsh
	-$(FILEWRAP)  -oa $@ ./VertShader.vsc

$(CONTENTDIR)/SkyboxFShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./SkyboxFShader.fsh
	-$(FILEWRAP)  -oa $@ ./SkyboxFShader.fsc

$(CONTENTDIR)/SkyboxVShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./SkyboxVShader.vsh
	-$(FILEWRAP)  -oa $@ ./SkyboxVShader.vsc

$(CONTENTDIR)/ModelFShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./ModelFShader.fsh
	-$(FILEWRAP)  -oa $@ ./ModelFShader.fsc

$(CONTENTDIR)/ModelVShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./ModelVShader.vsh
	-$(FILEWRAP)  -oa $@ ./ModelVShader.vsc

$(CONTENTDIR)/Tex2DFShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./Tex2DFShader.fsh
	-$(FILEWRAP)  -oa $@ ./Tex2DFShader.fsc

$(CONTENTDIR)/Tex2DVShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./Tex2DVShader.vsh
	-$(FILEWRAP)  -oa $@ ./Tex2DVShader.vsc

$(CONTENTDIR)/PlaneTexFShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./PlaneTexFShader.fsh
	-$(FILEWRAP)  -oa $@ ./PlaneTexFShader.fsc

$(CONTENTDIR)/PlaneTexVShader.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -s  -o $@ ./PlaneTexVShader.vsh
	-$(FILEWRAP)  -oa $@ ./PlaneTexVShader.vsc

$(CONTENTDIR)/Scene.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Scene.pod

############################################################################
# End of file (content.mak)
############################################################################

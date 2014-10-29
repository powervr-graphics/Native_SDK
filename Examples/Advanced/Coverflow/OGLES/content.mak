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
	$(CONTENTDIR)/Album1.cpp \
	$(CONTENTDIR)/Album2.cpp \
	$(CONTENTDIR)/Album3.cpp \
	$(CONTENTDIR)/Album4.cpp \
	$(CONTENTDIR)/Album5.cpp \
	$(CONTENTDIR)/Album6.cpp \
	$(CONTENTDIR)/Album7.cpp \
	$(CONTENTDIR)/Album8.cpp \
	$(CONTENTDIR)/Album9.cpp \
	$(CONTENTDIR)/Album10.cpp \
	$(CONTENTDIR)/Album11.cpp \
	$(CONTENTDIR)/Album12.cpp \
	$(CONTENTDIR)/Album13.cpp \
	$(CONTENTDIR)/Album14.cpp \
	$(CONTENTDIR)/Album15.cpp \
	$(CONTENTDIR)/Album16.cpp

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

$(CONTENTDIR)/Album1.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album1.pvr

$(CONTENTDIR)/Album2.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album2.pvr

$(CONTENTDIR)/Album3.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album3.pvr

$(CONTENTDIR)/Album4.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album4.pvr

$(CONTENTDIR)/Album5.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album5.pvr

$(CONTENTDIR)/Album6.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album6.pvr

$(CONTENTDIR)/Album7.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album7.pvr

$(CONTENTDIR)/Album8.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album8.pvr

$(CONTENTDIR)/Album9.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album9.pvr

$(CONTENTDIR)/Album10.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album10.pvr

$(CONTENTDIR)/Album11.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album11.pvr

$(CONTENTDIR)/Album12.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album12.pvr

$(CONTENTDIR)/Album13.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album13.pvr

$(CONTENTDIR)/Album14.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album14.pvr

$(CONTENTDIR)/Album15.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album15.pvr

$(CONTENTDIR)/Album16.cpp: $(CONTENTDIR)
	$(FILEWRAP)  -o $@ ./Album16.pvr

############################################################################
# End of file (content.mak)
############################################################################

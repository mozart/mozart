# -*- Makefile -*-
#---------------------------------------------------------------------
# Bootstraping
#	use the local emulator and local libraries
#---------------------------------------------------------------------

URL=http\\://www.ps.uni-sb.de/ozhome
BUILDSHARE=$(BUILDTOP)/share
BUILDLIB=$(BUILDSHARE)/lib
BUILDTOOLS=$(BUILDSHARE)/tools

SOURCELIB=$(SRCTOP)/share/lib
SOURCETOOLS=$(SRCTOP)/share/tools

BOOTEMU=$(BUILDTOP)/platform/emulator/oz.emulator.bin
BOOTCOM=$(BOOTEMU) -u $(BUILDLIB)/ozc --
BOOTENG=$(SRCTOP)/share/ozengine.sh

ifdef OZC
export OZC
endif

ifdef OZINIT
export OZINIT
endif

ifdef OZ_LOAD
export OZ_LOAD
endif

ifdef OZEMULATOR
export OZEMULATOR
endif

ifdef OZPATH
export OZPATH
endif

boot-%:
	$(MAKE) $* \
	OZEMULATOR=$(BOOTEMU) \
	OZENGINE=$(BOOTENG) \
	OZC="$(BOOTCOM)" \
	OZINIT=$(BUILDLIB)/Init.ozp \
	OZPATH=.:$(BUILDLIB):$(BUILDTOOLS):$(SOURCELIB):$(SOURCETOOLS) \
	OZ_LOAD=root=.:prefix=/=/:prefix=$(URL)/lib/=$(BUILDLIB)/:prefix=$(URL)/tools/=$(BUILDTOOLS)/:=

# stage1-all: create the components using the BUILDTOP/share/lib/stage1
STAGE1_LIB=$(BUILDLIB)/stage1
STAGE1_TOOLS=$(BUILDTOOLS)/stage1
stage1-%:
	$(MAKE) $* \
	OZ_LOAD=root=.:prefix=$(URL)=$(STAGE1_LIB):prefix=/=/:= \
	OZINIT=$(STAGE1_LIB)/lib/Init.ozp \
	OZC=$(STAGE1_LIB)/bin/ozc

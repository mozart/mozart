# -*- Makefile -*-
#---------------------------------------------------------------------
# Bootstraping
#       use the local emulator and local libraries
#---------------------------------------------------------------------

URL=http\\://www.ps.uni-sb.de/ozhome
BUILDSHARE=$(BUILDTOP)/share
BUILDLIB=$(BUILDSHARE)/lib
BUILDTOOLS=$(BUILDSHARE)/tools

SOURCELIB=$(SRCTOP)/share/lib
SOURCETOOLS=$(SRCTOP)/share/tools

BOOTEMU=$(BUILDTOP)/platform/emulator/oz.emulator.bin
BOOTCOM=$(BOOTEMU) -u $(BUILDLIB)/ozbatch --
BOOTENG=$(SRCTOP)/share/ozengine.sh

ifdef OZBATCH
export OZBATCH
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
        OZBATCH="$(BOOTCOM)" \
        OZINIT=$(BUILDLIB)/Init.ozc \
        OZPATH=.:$(BUILDLIB):$(BUILDTOOLS):$(SOURCELIB):$(SOURCETOOLS) \
        OZ_LOAD=root=.:prefix=/=/:prefix=$(URL)/=$(BUILDLIB)/tyc/:prefix=$(URL)/lib/=$(BUILDLIB)/:=

# stage1-all: create the components using the BUILDTOP/share/lib/stage1
STAGE1_LIB=$(BUILDLIB)/stage1
STAGE1_TOOLS=$(BUILDTOOLS)/stage1
stage1-%:
        $(MAKE) $* \
        OZ_LOAD=root=.:prefix=$(URL)/tools/=$(STAGE1_TOOLS)/tyc/:prefix=$(URL)=$(STAGE1_LIB):prefix=/=/:= \
        OZINIT=$(STAGE1_LIB)/lib/Init.ozc \
        OZBATCH=$(STAGE1_LIB)/bin/ozbatch

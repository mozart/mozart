# -*- Makefile -*-
#---------------------------------------------------------------------
# Bootstraping
#       use the local emulator and local libraries
#---------------------------------------------------------------------

BUILDSHARE=$(BUILDTOP)/share
BUILDLIB=$(BUILDSHARE)/lib
BUILDTOOLS=$(BUILDSHARE)/tools
BUILDCONTRIB=$(BUILDTOP)/contrib
BUILDGDBM=$(BUILDCONTRIB)/gdbm
BUILDOS=$(BUILDCONTRIB)/os

SOURCELIB=$(SRCTOP)/share/lib
SOURCETOOLS=$(SRCTOP)/share/tools

BOOTEMU=$(BUILDTOP)/platform/emulator/emulator.exe
BOOTCOM=$(BOOTEMU) -u $(BUILDLIB)/ozc --
BOOTENG=$(SRCTOP)/share/ozengine.sh
BOOTOZL=$(BOOTENG) $(BUILDTOP)/share/lib/ozl

ifdef OZC
export OZC
endif

ifdef OZL
export OZL
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

ifdef OZDOC_HOME
export OZDOC_HOME
endif

ifdef OZDOC_AUTHOR_PATH
export OZDOC_AUTHOR_PATH
endif

ifdef OZDOC_BIB_PATH
export OZDOC_BIB_PATH
endif

ifdef OZDOC_BST_PATH
export OZDOC_BST_PATH
endif

ifdef OZDOC_ELISP_PATH
export OZDOC_ELISP_PATH
endif

ifdef OZDOC_SBIN_PATH
export OZDOC_SBIN_PATH
endif

boot-%:
        $(MAKE) $* \
        OZEMULATOR=$(BOOTEMU) \
        OZENGINE=$(BOOTENG) \
        OZC="$(BOOTCOM)" \
        OZINIT=$(BUILDLIB)/Init.ozf \
        OZPATH=.:$(BUILDLIB):$(BUILDTOOLS):$(SOURCELIB):$(SOURCETOOLS) \
        OZ_LOAD=root=.:prefix=/=/:prefix=./=./:prefix=$(HOMEURL)/share/=$(BUILDLIB)/:prefix=$(HOMEURL)/share/=$(BUILDTOOLS)/:prefix=$(HOMEURL)/contrib/=$(BUILDCONTRIB)/:prefix=$(HOMEURL)/contrib/=$(BUILDGDBM)/:prefix=$(HOMEURL)/contrib/os/=$(BUILDOS)/:= \
        OZL="$(BOOTOZL)" \
        OZDOC_HOME="$(SRCTOP)/doc/utilities" \
        OZDOC_AUTHOR_PATH="$(SRCDIR):$(SRCTOP)/doc" \
        OZDOC_BIB_PATH="$(SRCDIR)" \
        OZDOC_BST_PATH="$(SRCDIR):$(SRCTOP)/doc/utilities" \
        OZDOC_ELISP_PATH="$(BUILDDIR):$(BUILDTOP)/doc:$(BUILDTOP)/doc/utilities:$(BUILDTOP)/share/elisp:$(BUILDTOP)/contrib/doc/code" \
        OZDOC_SBIN_PATH="$(SRCTOP)/doc/utilities" \
        OZDOC_CATALOG="$(BUILDTOP)/doc/bootcatalog"

# stage1-all: create the components using the BUILDTOP/share/lib/stage1
STAGE1_LIB=$(BUILDLIB)/stage1
STAGE1_TOOLS=$(BUILDTOOLS)/stage1
stage1-%:
        $(MAKE) $* \
        OZ_LOAD=root=.:prefix=$(HOMEURL)=$(STAGE1_LIB):prefix=$(HOMEURL)/share=.:prefix=/=/:= \
        OZINIT=$(STAGE1_LIB)/lib/Init.ozf \
        OZC=$(STAGE1_LIB)/bin/ozc

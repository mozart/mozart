# -*- Makefile -*-
#---------------------------------------------------------------------
# Bootstraping
#	use the local emulator and local libraries
#---------------------------------------------------------------------

BUILDSHARE=$(BUILDTOP)/share
BUILDLIB=$(BUILDSHARE)/lib
BUILDTOOLS=$(BUILDSHARE)/tools
BUILDCONTRIB=$(BUILDTOP)/contrib
BUILDGDBM=$(BUILDCONTRIB)/gdbm
BUILDOS=$(BUILDCONTRIB)/os
BUILDGUMP=$(BUILDTOP)/platform/tools/gump
BUILDBISON=$(BUILDGUMP)/ozbison
BUILDEMU=$(BUILDTOP)/platform/emulator

SOURCELIB=$(SRCTOP)/share/lib
SOURCETOOLS=$(SRCTOP)/share/tools

BOOTEMU=$(BUILDTOP)/platform/emulator/emulator.exe
BOOTCOM=$(BOOTEMU) -u $(BUILDLIB)/ozc --
BOOTENG=$(SRCTOP)/share/ozengine.sh
BOOTOZL=$(BOOTENG) $(BUILDTOP)/share/lib/ozl
BOOTOZTOOL="/bin/sh $(BUILDTOP)/platform/emulator/oztool.sh"
BOOTOZTOOLINC="-I$(SRCTOP)/platform/emulator -I$(SRCTOP)/platform/tools/gump"
BOOTOZFLEX=$(BUILDTOP)/platform/tools/gump/ozflex/flex

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

ifdef OZTOOL
export OZTOOL
endif

ifdef OZTOOL_INCLUDES
export OZTOOL_INCLUDES
endif

ifdef OZFLEX
ifneq (OZFLEX,)
export OZFLEX
endif
endif

boot-%:
	$(MAKE) $* \
	OZEMULATOR=$(BOOTEMU) \
	OZENGINE=$(BOOTENG) \
	OZC="$(BOOTCOM)" \
	OZINIT=$(BUILDLIB)/Init.ozf \
	OZPATH=.:$(BUILDLIB):$(BUILDTOOLS):$(SOURCELIB):$(SOURCETOOLS) \
	OZ_LOAD=root=.:prefix=/=/:prefix=./=./:prefix=x-oz://system/=$(BUILDLIB)/:prefix=x-oz://system/=$(BUILDTOOLS)/:prefix=x-oz://contrib/=$(BUILDCONTRIB)/:prefix=x-oz://contrib/=$(BUILDGDBM)/:prefix=x-oz://contrib/os/=$(BUILDOS)/:prefix=x-oz://boot/=$(BUILDEMU)/:prefix=x-oz://system/=$(BUILDGUMP)/:prefix=x-oz://system/=$(BUILDBISON)/:= \
	OZL="$(BOOTOZL)" \
	OZDOC_HOME="$(SRCTOP)/doc/utilities" \
	OZDOC_AUTHOR_PATH="$(SRCDIR):$(SRCTOP)/doc" \
	OZDOC_BIB_PATH="$(SRCDIR)" \
	OZDOC_BST_PATH="$(SRCDIR):$(SRCTOP)/doc/utilities" \
	OZDOC_ELISP_PATH=".:$(SRCDIR):$(BUILDTOP)/doc:$(BUILDTOP)/doc/utilities:$(BUILDTOP)/share/elisp:$(BUILDTOP)/contrib/doc/code" \
	OZDOC_SBIN_PATH="$(SRCTOP)/doc/utilities" \
	OZDOC_CATALOG="$(BUILDTOP)/doc/bootcatalog" \
	OZTOOL=$(BOOTOZTOOL) \
	OZTOOL_INCLUDES=$(BOOTOZTOOLINC) \
	OZFLEX=$(BOOTOZFLEX)

cboot-%:
	$(MAKE) $* \
	OZTOOL=$(BOOTOZTOOL) \
	OZTOOL_INCLUDES=$(BOOTOZTOOLINC)

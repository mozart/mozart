#---------------------------------------------------------------------
# Bootstraping
#       use the local emulator and local libraries
#---------------------------------------------------------------------

URL=http\\://www.ps.uni-sb.de/ozhome
BUILDLIB=$(BUILDTOP)/share/lib
BUILDTOOLS=$(BUILDTOP)/share/tools
SOURCELIB=$(TOPDIR)/share/lib
SOURCETOOLS=$(TOPDIR)/share/tools
BOOTEMU=$(BUILDTOP)/platform/emulator/oz.emulator.bin
BOOTCOM=$(BOOTEMU) -u $(BUILDLIB)/ozbatch -a
BOOTENG=$(TOPDIR)/share/ozengine.sh
export OZBATCH
export OZINIT
export OZ_LOAD
export OZEMULATOR
export OZPATH

bootstrap:
        $(MAKE) all \
        OZEMULATOR=$(BOOTEMU) \
        OZENGINE=$(BOOTENG) \
        OZBATCH="$(BOOTCOM)" \
        OZINIT=$(BUILDLIB)/Init.ozc \
        OZPATH=.:$(BUILDLIB):$(BUILDTOOLS):$(SOURCELIB):$(SOURCETOOLS) \
        OZ_LOAD=root=.:prefix=/=/:prefix=$(URL)/=$(BUILDLIB)/tyc/:prefix=$(URL)/lib/=$(BUILDLIB)/:=

boot-%:
        $(MAKE) $* \
        OZEMULATOR=$(BOOTEMU) \
        OZENGINE=$(BOOTENG) \
        OZBATCH="$(BOOTCOM)" \
        OZINIT=$(BUILDLIB)/Init.ozc \
        OZPATH=.:$(BUILDLIB):$(BUILDTOOLS):$(SOURCELIB):$(SOURCETOOLS) \
        OZ_LOAD=root=.:prefix=/=/:prefix=$(URL)/=$(BUILDLIB)/tyc/:prefix=$(URL)/lib/=$(BUILDLIB)/:=

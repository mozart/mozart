# Makefile to package the a Mozart distribution for Mac OS X as a bundle 
# inside a disk image.
# 
#
# Author: Gustavo Gutierrez
# Last modified: $Date$
#
# Credits: 
# Build file for creating DMG files.
#
# The DMG packager looks for a template.dmg.bz2 for using as its 
# DMG template. If it doesn't find one, it generates a clean one.
#
# If you create a DMG template, you should make one containing all
# the files listed in $(SOURCE_FILES) below, and arrange everything to suit
# your style. The contents of the files themselves does not matter, so
# they can be empty (they will be overwritten later). 
#
# Remko Troncon 
# http://el-tramo.be/about
#

###
# Variables for external tools and directories.
###

# Platpus commanline tool
PLAT=/usr/local/bin/platypus 

## Cocoa dialog
CD=/Users/ggutierrez/Desktop/CocoaDialog.app

##
# Mozart Installation
##
MOZ_INST=/Users/ggutierrez/Work/moz140

## Mozart documentation
MOZ_DOC=/Users/ggutierrez/Desktop/Documentation

##----------------------------------------------------------------
## Do not modify anything bellow this line

## Directories of mozart installation to be bundled.
MOZ_DIRS= platform bin cache share include

NAME=Mozart
APP=$(NAME).app

## Directories inside the bundle
BND_CONTENTS=$(APP)/Contents

## The Resources directory is where mozart installation files will be.
BND_RESOURCES=$(BND_CONTENTS)/Resources

## The Frameworks directory is where shared libraries used by the emulator 
## will be placed. For instance, libgmp.dylib

BND_FRAMEWORKS=$(BND_CONTENTS)/Frameworks


## The path to the emulator inside the bundle
PLATFORM=`$(MOZ_INST)/bin/oztool platform`
EMULATOR=$(APP)/Contents/Resources/platform/$(PLATFORM)/emulator.exe

## Platypus call
PLATCALL = $(PLAT) -i 'appIcon.icns' script.sh ./$(APP)

##
#Create a bundle from skel
##
all: bundle fixpaths 

bundle:
	$(PLATCALL)
	for i in $(MOZ_DIRS); do  \
		cp -R $(MOZ_INST)/$$i $(BND_RESOURCES); \
	done
	cp -R $(CD) $(BND_RESOURCES)
	mkdir -p $(BND_FRAMEWORKS)/


fixpaths:
	./bundlelibs.sh $(EMULATOR) $(APP)


diskimage:
	-rm -rf tmpdir
	mkdir -p tmpdir/
	mv Mozart.app tmpdir/
	cp -r $(MOZ_DOC) tmpdir/
	./make-dmg.sh $(NAME).dmg tmpdir/ $(NAME)
	-rm -rf tmpdir
	@echo finished


.PHONY: bundskel fixpaths

clean:
	-rm -rf $(APP)

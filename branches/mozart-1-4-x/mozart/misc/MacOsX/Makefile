# Script to package the mozart system into an application bundle for
# Mac Os X.
#
# Author: Gustavo Gutierrez
# Date: May 9 2007

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

###
# Platypus is required to build the application bundle. Please download it from:
# http://www.sveinbjorn.org/platypus and install the command lines tools. Then, set the
# following variable accordingly.

# Platpus commanline tool
PLAT=/usr/local/bin/platypus 

## Cocoa dialog : download it from http://cocoadialog.sourceforge.net/
CD=/Users/ggutierrez/Desktop/CocoaDialog.app


##
# Mozart Installation
##
MOZ_INST=/Users/ggutierrez/Work/moz140
## Directories of mozart installation to be bundled.
MOZ_DIRS= platform bin cache share include


##Mozart documentation folder (html)
#MOZ_DOC_HTML=/Users/ggutierrez/doc

##Mozart standard library documentation dir (html)
#MOZ_STDLIB_HTML=$(MOZ_HOME)/doc


#INTOOL=/usr/bin/install_name_tool
#OTOOL=/usr/bin/otool -L
NAME=Mozart
#VERSION=1.4.0


### do not change anything!!

APP=$(NAME).app

## Directories inside the bundle
BND_CONTENTS=$(APP)/Contents

## The Resources directory is where mozart installation files will be.
BND_RESOURCES=$(BND_CONTENTS)/Resources

## The Frameworks directory is where shared libraries used by the emulator will be.
## For instance, libgmp.dylib
BND_FRAMEWORKS=$(BND_CONTENTS)/Frameworks


#EXT_LIBS=ext_libs

## The path to the emulator inside the bundle
EMULATOR=$(APP)/Contents/Resources/platform/darwin-i386/emulator.exe

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
#	cp $(EXT_LIBS)/* $(BND_FRAMEWORKS)/

fixpaths:
	./bundlelibs.sh $(EMULATOR) $(APP)

.PHONY: bundskel

clean:
	-rm -rf $(APP)

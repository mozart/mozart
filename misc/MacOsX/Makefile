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
# Mozart Install
##
MOZ_HOME=/Users/ggutierrez/Work/moz132
## Directories of mozart installation to be bundled.
MOZ_DIRS= platform bin cache share include


##Mozart documentation folder (html)
MOZ_DOC_HTML=/Users/ggutierrez/doc

##Mozart standard library documentation dir (html)
MOZ_STDLIB_HTML=$(MOZ_HOME)/doc

################################################################################
# Customizable variables
################################################################################
INTOOL=/usr/bin/install_name_tool
OTOOL=/usr/bin/otool -L
NAME=Mozart
VERSION=1.3.2

APP=$(NAME).app
EXT_LIBS=$(APP)/Contents/Resources/ext_libs
EXEC=$(APP)/Contents/Resources/platform/i486-darwin/emulator.exe

## Platypus call
PLATCALL=$(PLAT) -a '$(NAME)' -t 'Shell' -o 'None' -u 'Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>' -i 'appIcon.icns'  -V '$(VERSION)' -s '????' -I 'org.Mozart' 'script' '$(APP)'
 

# External libraries thet mozart uses (i.e. gmp)
MOZ_LIBS_DIR=/Users/ggutierrez/Work/ext_libs


##
# Some dirs in the bundle
##
BCONT=$(APP)/Contents
BRSRCS=$(BCONT)/Resources
BFRWRKS=$(BCONT)/Frameworks

##
# Bundle related files
##
InfPL=Info.plist
ICN=appIcon.icns
BBIN=Mozart.sh

SOURCE_FILES=$(APP) 
OZHOME_FILES=README LICENSE.rtf LICENSE.html
OZHOME_DIRS= examples

TEMPLATE_DMG=template.dmg


################################################################################
# DMG building. No editing should be needed beyond this point.
################################################################################

MASTER_DMG=$(NAME)-$(VERSION).dmg
WC_DMG=wc.dmg
WC_DIR=wc

.PHONY: all
all: bundskel fixpaths $(MASTER_DMG)

$(TEMPLATE_DMG): $(TEMPLATE_DMG).bz2
	bunzip2 -k $<

$(TEMPLATE_DMG).bz2: 
	@echo
	@echo --------------------- Generating empty template --------------------
	mkdir template
	hdiutil create -size  40m "$(TEMPLATE_DMG)" -srcfolder template -format UDRW -volname "$(NAME)" -quiet
	rmdir template
	bzip2 "$(TEMPLATE_DMG)"
	@echo

$(WC_DMG): $(TEMPLATE_DMG)
	cp $< $@

$(MASTER_DMG): $(WC_DMG) $(SOURCE_FILES)
	@echo
	@echo --------------------- Creating Disk Image --------------------
	mkdir -p $(WC_DIR)
	hdiutil attach "$(WC_DMG)" -noautoopen -quiet -mountpoint "$(WC_DIR)"
	for i in $(SOURCE_FILES); do  \
		rm -rf "$(WC_DIR)/$$i"; \
		ditto -rsrc "$$i" "$(WC_DIR)/$$i"; \
	done
	for i in $(OZHOME_FILES); do  \
		rm -rf "$(WC_DIR)/$$i"; \
		ditto -rsrc "$(MOZ_HOME)/$$i" "$(WC_DIR)"; \
	done
	for i in $(OZHOME_DIRS); do  \
		rm -rf "$(WC_DIR)/$$i"; \
		ditto -rsrc "$(MOZ_HOME)/$$i" "$(WC_DIR)/$$i"; \
	done
	
	rm -rf "$(WC_DIR)/doc"
	ditto -rsrc "$(MOZ_DOC_HTML)" "$(WC_DIR)/doc/"; 
	
	rm -rf "$(WC_DIR)/stdlib-doc"
	ditto -rsrc "$(MOZ_STDLIB_HTML)/mozart-ozmake" "$(WC_DIR)/stdlib-doc/mozart-ozmake";
	ditto -rsrc "$(MOZ_STDLIB_HTML)/mozart-stdlib" "$(WC_DIR)/stdlib-doc/mozart-stdlib";
	
	WC_DEV=`hdiutil info | grep "$(WC_DIR)" | grep "Apple_HFS" | awk '{print $$1}'` && \
	hdiutil detach $$WC_DEV -quiet -force
	rm -f "$(MASTER_DMG)"
	hdiutil convert "$(WC_DMG)" -quiet -format UDZO -imagekey zlib-level=9 -o "$@"
	rm -rf $(WC_DIR)
	@echo

##
# App creation
##
fixpaths: 
	@echo
	@echo --------------------- Fixing libgmp --------------------

	$(INTOOL) -change /opt/local/lib/libgmp.3.dylib \
	@executable_path/../../../Frameworks/libgmp.3.dylib  $(EXEC)
	$(INTOOL) -change /opt/local/lib/libz.1.dylib \
	@executable_path/../../../Frameworks/libz.1.dylib  $(EXEC)
	@echo
	@echo --------------------- Path results --------------------
	$(OTOOL) $(EXEC)


##
#Create a bundle from skel
##
bundskel:
	$(shell $(PLATCALL))
	@echo --------------------- Populating the bundle with mozart dirs ------------
	for i in $(MOZ_DIRS); do  \
		cp -R $(MOZ_HOME)/$$i $(BRSRCS); \
	done
	@echo --------------------- Populating the bundle with CocoaDialog ------------
	cp -R $(CD) $(BRSRCS)
	@echo --------------------- Done. ---------------
	@echo --------------------- Populating the bundle with External Libs ------------
	mkdir -p $(BFRWRKS)/
	cp $(MOZ_LIBS_DIR)/* $(BFRWRKS)/
	@echo --------------------- Done. ---------------

.PHONY: clean veryclean
clean:
	-rm -rf $(TEMPLATE_DMG) $(MASTER_DMG) $(WC_DMG)  $(TEMPLATE_DMG).bz2

veryclean: clean
	-rm -rf Mozart.app
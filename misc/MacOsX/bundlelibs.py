#!/usr/bin/env python

## This script was taken from http://clam-project.org/clam/trunk/CLAM/scons/sconstools/addDependentLibsToBundle.py and adapted
## to patch the mozart binary bundle for Mac Os X. The clam project uses GPL Licene, see the following link for more details. 
## http://clam-project.org/wiki/Frequenly_Asked_Questions#Which_is_the_license_for_CLAM.3F_Is_it_free.3F_May_I_use_it_in_a_non-free_project.3F


import os, glob
import sys

def run(command) :
	print "\033[32m:: ", command, "\033[0m"
	return os.system(command)
def norun(command) :
	print "\033[31mXX ", command, "\033[0m"


def needsChange(binary, blacklist) :
	#with python2.5 we could just return all([not binary.startswith(blacksheep) for blacksheep in blacklist])
	for blacksheep in blacklist :
		if binary.startswith( blacksheep ) : 
#			print "found blackseep", binary
			return False
	return True

def libDependencies(binary, visited, blacklist) :
#	print "examining", binary
	for line in os.popen("otool -L "+binary).readlines()[1:] :
		entry = line.split()[0]
		if entry in visited : continue
		if not needsChange( entry, blacklist ) : continue
		visited.append( entry )
		libDependencies( entry, visited, blacklist )

def addDependentLibsToBundle( bundle ) :
	binaries = glob.glob(bundle+"/Contents/Resources/platform/darwin-i386/emulator.exe")
	doNotChange = [
		"/System/",
		"/usr/lib/",
		"@executable_path/",
	]
	libsPath = []
	for binary in binaries :
		libDependencies(binary, libsPath, doNotChange)
#	print libsPath

	libs = [ (os.path.basename(path), path) for path in libsPath ] 
	run("mkdir -p %(bundle)s/Contents/Frameworks/" % locals() )

	vars = {}
	# copy all dependent libs to the bundle and change its id (relative path to the bundle)
	for lib, path in libs :
		run("cp %(path)s %(bundle)s/Contents/Frameworks/%(lib)s" % locals() )
		run("install_name_tool -id @executable_path/../../../Frameworks/%(lib)s %(bundle)s/Contents/Frameworks/%(lib)s" % locals() )
	# fix binary dependencies
	for current in binaries :
		for lib, libpath in libs :
			run("install_name_tool -change %(libpath)s @executable_path/../../../Frameworks/%(lib)s %(current)s" % locals() )
	# fix libs dependencies
	for current, _ in libs :
		for lib, libpath in libs :
			run("install_name_tool -change %(libpath)s @executable_path/../../../Frameworks/%(lib)s %(bundle)s/Contents/Frameworks/%(current)s" % locals() )

if __name__ == "__main__":
	addDependentLibsToBundle( "Mozart.app" )

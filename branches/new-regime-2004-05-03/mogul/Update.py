#! /usr/bin/python

import os,re,cPickle,sys

# CONSTANTS


VERBOSE         = True
OZMAKE_130      = "/opt/mozart/bin/ozengine /opt/mozart/bin/ozmake"
OZMAKE_125      = "/home/denys/src/Mozart/release/install/bin/ozengine /home/denys/src/Mozart/release/install/bin/ozmake"
OZMAKE          = OZMAKE_130

FULL_ROOT       = os.path.abspath("tmp")

DIR_UPLOAD      = "upload"
DIR_POPULATE    = "populate"
DIR_PKG         = "pkg"
DIR_SPECIFIC    = "specific"

FULL_UPLOAD     = "%s/%s" % (FULL_ROOT,DIR_UPLOAD)
FULL_POPULATE   = "%s/%s" % (FULL_ROOT,DIR_POPULATE)
FULL_PKG        = "%s/%s" % (FULL_ROOT,DIR_PKG)
FULL_SPECIFIC   = "%s/%s" % (FULL_ROOT,DIR_SPECIFIC)
FULL_EXTRACT    = "/tmp/MOGUL.EXTRACT"

FULL_POPULATE_N = len(FULL_POPULATE)+1

MARGIN = ""

def TRACE(msg):
    if VERBOSE:
        print "%s%s" % (MARGIN,msg)

def INC():
    global MARGIN
    MARGIN += "  "

def DEC():
    global MARGIN
    MARGIN = MARGIN[:-2]

class Uploaded:
    def __init__(self,src,dst,ispkg,format,platform,version):
        self.src = src
        self.dst = dst
        self.ispkg = ispkg
        self.format = format
        self.platform = platform
        self.version = version

uploaded_items = []

def SYSTEM(cmd):
    TRACE(cmd)
    return os.system(cmd)

FORMAT_CACHE = {}
FORMAT_CACHE_FILE = "%s/FORMAT.CACHE" % FULL_UPLOAD

# we process the DIR_UPLOAD/ area, and build up a list of the items
# it contains.  For each *.pkg file, we determine also its format.
# Since determining the format may involve attempting to unpack the
# package using the 1.3.0 ozmake, this can be costly, and we keep a
# a cache as a python pickle mapping file paths to triple of
# (inode,mtime,format); if the inode and mtime haven't changed, then
# we just reuse the cached format.

TRACE("Processing .../%s/ area" % DIR_UPLOAD)
INC()

def uploaded(fulldir,dir,file):
    base = os.path.basename(dir)
    dirname = os.path.dirname(dir)
    if base.startswith("__"):
        empty,format,platform,version = base.split("__")
        if file.endswith(".pkg"):
            id = dirname.replace("/","-")
            if format=="xxx":
                ## try the fast way using the cache
                key = "%s/%s" % (fulldir,file)
                TRACE("determing format of %s" % key)
                nformat = None
                if FORMAT_CACHE.has_key(key):
                    inode,mtime,zformat = FORMAT_CACHE[key]
                    st = os.stat(key)
                    if st.st_ino==inode and st.st_mtime==mtime:
                        nformat = zformat
                        TRACE("cached: %s" % nformat)
                if not nformat:
                    INC()
                    if SYSTEM("%s --dir=%s -xnp '%s/%s' >/dev/null 2>/dev/null" % (OZMAKE,FULL_EXTRACT,fulldir,file)) == 0:
                        nformat = "1.3.0"
                    else:
                        nformat = "1.2.5"
                    DEC()
                    TRACE("found: %s" % nformat)
                    st = os.stat(key)
                    inode = st.st_ino
                    mtime = st.st_mtime
                    FORMAT_CACHE[key] = (inode,mtime,nformat)
            else:
                nformat = format
            nname = "%s__%s__%s__%s.pkg" % (id,nformat,platform,version)
            src = "%s/%s" % (dir,file)
            dst = "%s/%s" % (dirname,nname)
            uploaded_items.append(Uploaded(src,dst,True,format,platform,version))
        else:
            src = "%s/%s" % (dir,file)
            dst = "%s/%s" % (dirname,file)
            uploaded_items.append(Uploaded(src,dst,False,format,platform,version))

def read_uploaded(uploaddir):
    global FORMAT_CACHE
    uploaddir = os.path.normpath(uploaddir)
    if os.path.exists(FORMAT_CACHE_FILE):
        TRACE("Loading format cache")
        FORMAT_CACHE = cPickle.load(open(FORMAT_CACHE_FILE,"r"))
    n = len(uploaddir)+1
    for (dir,dirs,files) in os.walk(uploaddir):
        for file in files:
            uploaded(dir,dir[n:],file)
    TRACE("Saving format cache")
    cPickle.dump(FORMAT_CACHE,open(FORMAT_CACHE_FILE,"w"))

read_uploaded(FULL_UPLOAD)

DEC()
TRACE("...done")

#sys.exit(0)

IS_UPLOADED = {}

TRACE("Linking .../%s/ area from .../%s/ area" % (DIR_POPULATE,DIR_UPLOAD))
INC()

for item in uploaded_items:
    src = "%s/%s" % (FULL_UPLOAD  ,item.src)
    dst = "%s/%s" % (FULL_POPULATE,item.dst)
    IS_UPLOADED[dst] = True
    dstdir = os.path.dirname(dst)
    needlink = False
    if not os.path.exists(dstdir):
        os.makedirs(dstdir)
        needlink = True
    elif os.path.exists(dst):
        srcmtime = os.stat(src).st_mtime
        dstmtime = os.stat(dst).st_mtime
        if dstmtime < srcmtime:
            needlink = True
    else:
        needlink = True
    if needlink:
        if os.path.exists(dst):
            TRACE("outdated: rm %s" % dst)
            os.unlink(dst)
        TRACE("ln %s %s" % (src,dst))
        os.link(src,dst)

DEC()
TRACE("...done")

#sys.exit(0)

# process the FULL_POPULATE area and provide format conversions

PKG_RE = re.compile("^(.*)__([^_]+)__([^_]+)__([^_]+)\.pkg$")

FORMAT_TO_OZMAKE = {
    '1.2.5' : OZMAKE_125,
    '1.3.0' : OZMAKE_130,
    }

ALL_FORMATS = FORMAT_TO_OZMAKE.keys()

TRACE("Providing format conversions in .../%s/ area" % DIR_POPULATE)
INC()

for (dir,dirs,files) in os.walk(FULL_POPULATE):
    for file in files:
        match = PKG_RE.match(file)
        if match:
            main,format,platform,version = match.group(1,2,3,4)
            if FORMAT_TO_OZMAKE.has_key(format):
                if os.path.exists(FULL_EXTRACT):
                    SYSTEM("rm -rf %s" % FULL_EXTRACT)
                cmd = FORMAT_TO_OZMAKE[format]
                filename = "%s/%s" % (dir,file)
                extracted = False
                for newformat in ALL_FORMATS:
                    if newformat != format:
                        new = "%s/%s__%s__%s__%s.pkg" % (dir,main,newformat,platform,version)
                        doit = False
                        if not os.path.exists(new):
                            doit = True
                        elif IS_UPLOADED.has_key(filename) and \
                             (os.stat(filename).st_mtime > os.stat(new).st_mtime):
                            # we only convert things that are uploaded and out of date
                            doit = True
                        if doit:
                            if extracted is False:
                                if SYSTEM("%s --dir=%s -xqp %s" % (cmd,FULL_EXTRACT,filename)) == 0:
                                    extracted = True
                                else:
                                    extracted = None
                            if extracted:
                                newcmd = FORMAT_TO_OZMAKE[newformat]
                                SYSTEM("%s --dir=%s -cp %s" % (newcmd,FULL_EXTRACT,new))

DEC()
TRACE("...done")

#sys.exit(0)

# lookup up all the files now present in the FULL_POPULATE area
# and create objects for them

class Available:
    def __init__(self,path,fullpath):
        self.fullpath = fullpath
        self.path = path
        self.dir = os.path.dirname(path)
        self.ispkg = path.endswith(".pkg")
        base = os.path.basename(path)
        match = PKG_RE.match(base)
        if match:
            self.main,self.format,self.platform,self.version = match.group(1,2,3,4)
        else:
            self.main=self.format=self.platform=self.version=None
        self.mtime = os.stat(fullpath).st_mtime

AVAILABLE = []

TRACE("Collecting files in .../%s/ area" % DIR_POPULATE)
INC()

for (dir,dirs,files) in os.walk(FULL_POPULATE):
    pdir = dir[FULL_POPULATE_N:]
    for file in files:
        pfile = "%s/%s" % (pdir,file)
        ffile = "%s/%s" % (dir,file)
        AVAILABLE.append(Available(pfile,ffile))

DEC()
TRACE("...done")

#sys.exit(0)

# now, we should populate the specific areas.  When we populate
# an area for FORMAT==FFF and PLATFORM==PPP, we copy
# SECTION-PACKAGE__FFF__PPP__VERSION.pkg to
# SECTION-PACKAGE-VERSION.pkg
# and the highest version we also make available as
# SECTION-PACKAGE.pkg
#
# in order to discover the highest version for each package
# we will keep a table mapping DIR/SECTION-PACKAGE to highest version
# so far.

def version_lt(v1,v2):
    l1 = map(int,v1.split("."))
    l2 = map(int,v2.split("."))
    return version_lt_aux(l1,l2)

def version_lt_aux(l1,l2):
    if not l1:
        if l2:
            return True
        else:
            return False
    elif not l2:
        return False
    else:
        if l1[0] < l2[0]:
            return True
        elif l1[0] == l2[0]:
            return version_lt_aux(l1[1:],l2[1:])
        else:
            return False

def populate_specific(format,platform):
    TRACE("Populating .../%s/%s/%s" % (DIR_SPECIFIC,format,platform))
    INC()
    area = "%s/%s/%s" % (FULL_SPECIFIC,format,platform)
    highest = {}
    for item in AVAILABLE:
        if not item.ispkg:
            populate_link(item.fullpath,("%s/%s" % (area,item.path)))
        elif item.format==format and item.platform==platform:
            key = "%s/%s" % (item.dir,item.main)
            dst = "%s/%s-%s.pkg" % (area,key,item.version)
            populate_link(item.fullpath,dst)
            if not highest.has_key(key):
                highest[key] = item.version
            elif version_lt(highest[key],item.version):
                highest[key] = item.version
    for (key,version) in highest.iteritems():
        src = "%s/%s-%s.pkg" % (area,key,version)
        dst = "%s/%s.pkg" % (area,key)
        populate_link(src,dst)
    DEC()
    TRACE("...done")

def populate_link(src,dst):
    if not os.path.exists(dst):
        dir = os.path.dirname(dst)
        if not os.path.exists(dir):
            os.makedirs(dir)
        TRACE("ln %s %s" % (src,dst))
        os.link(src,dst)
    elif os.stat(src).st_ino != os.stat(dst).st_ino:
        TRACE("outdated: rm %s" % dst)
        os.unlink(dst)
        TRACE("ln %s %s" % (src,dst))
        os.link(src,dst)

ALL_PLATFORMS = ['source']

for format in ALL_FORMATS:
    for platform in ALL_PLATFORMS:
        populate_specific(format,platform)

#sys.exit(0)

# finally make sure that symbolic link FULL_PKG points to
# FULL_SPECIFIC/1.3.0/source

FULL_PKG_ORIG = "%s/%s/%s" % (FULL_SPECIFIC,"1.3.0","source")

TRACE("Updating .../%s/ area" % DIR_PKG)
INC()

if not os.path.exists(FULL_PKG):
    TRACE("ln -s %s %s" % (FULL_PKG_ORIG,FULL_PKG))
    os.symlink(FULL_PKG_ORIG,FULL_PKG)
elif os.stat(FULL_PKG).st_ino != os.stat(FULL_PKG_ORIG):
    TRACE("ln -s %s %s" % (FULL_PKG_ORIG,FULL_PKG))
    os.unlink(FULL_PKG)
    os.symlink(FULL_PKG_ORIG,FULL_PKG)

# also add symlinks to othe specific areas

for format in ALL_FORMATS:
    src = "%s/%s" % (FULL_SPECIFIC,format)
    dst = "%s/%s" % (FULL_PKG,format)
    if not os.path.exists(dst):
        TRACE("ln -s %s %s" % (src,dst))
        os.symlink(src,dst)

DEC()
TRACE("...done")

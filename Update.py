#! /usr/bin/python

import os,re,cPickle,sys

VERBOSE = True

def TRACE(msg):
    if VERBOSE:
        print msg

class Uploaded:
    def __init__(self,src,dst,ispkg,format,platform,version):
        self.src = src
        self.dst = dst
        self.ispkg = ispkg
        self.format = format
        self.platform = platform
        self.version = version

OZMAKE = "/opt/mozart/bin/ozengine /opt/mozart/bin/ozmake"
OZMAKE_130 = OZMAKE
OZMAKE_125 = "/home/denys/src/Mozart/release/install/bin/ozengine /home/denys/src/Mozart/release/install/bin/ozmake"

uploaded_items = []

def SYSTEM(cmd):
    TRACE(cmd)
    return os.system(cmd)

FORMAT_CACHE = {}
FORMAT_CACHE_FILE = "FORMAT.CACHE"

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
                nformat = None
                if FORMAT_CACHE.has_key(key):
                    inode,mtime,zformat = FORMAT_CACHE[key]
                    st = os.stat(key)
                    if st.st_ino==inode and st.st_mtime==mtime:
                        nformat = zformat
                if not nformat:
                    if SYSTEM("%s --dir=/tmp/MOGUL.NONE -xnp '%s/%s' >/dev/null 2>/dev/null" % (OZMAKE,fulldir,file)) == 0:
                        nformat = "1.3.0"
                    else:
                        nformat = "1.2.5"
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
            #print "%s/%s -> %s/%s" % (dir,file,dirname,nname)
        else:
            src = "%s/%s" % (dir,file)
            dst = "%s/%s" % (dirname,file)
            uploaded_items.append(Uploaded(src,dst,False,format,platform,version))
            #print "%s/%s -> %s/%s" % (dir,file,dirname,file)

def read_uploaded(uploaddir):
    global FORMAT_CACHE
    uploaddir = os.path.normpath(uploaddir)
    cache = "%s/%s" % (UPLOAD_DIR,FORMAT_CACHE_FILE)
    if os.path.exists(cache):
        FORMAT_CACHE = cPickle.load(open(cache,"r"))
    n = len(uploaddir)+1
    for (dir,dirs,files) in os.walk(uploaddir):
        for file in files:
            uploaded(dir,dir[n:],file)
    cPickle.dump(FORMAT_CACHE,open(cache,"w"))

UPLOAD_DIR = "tmp/upload"

read_uploaded(UPLOAD_DIR)

#sys.exit(0)

POPULATE_DIR = "tmp/populate"
POPULATE_DIR_N = len(POPULATE_DIR)+1

IS_UPLOADED = {}

for item in uploaded_items:
    src = "%s/%s" % (UPLOAD_DIR,item.src)
    dst = "%s/%s" % (POPULATE_DIR,item.dst)
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
            os.unlink(dst)
        os.link(src,dst)

#sys.exit(0)

# now, we should process the POPULATE_DIR area and provide
# format conversions

PKG_RE = re.compile("^(.*)__([^_]+)__([^_]+)__([^_]+)\.pkg$")

FORMAT_TO_OZMAKE = {
    '1.2.5' : OZMAKE_125,
    '1.3.0' : OZMAKE_130,
    }

ALL_FORMATS = FORMAT_TO_OZMAKE.keys()

EXTRACT_DIR = "/tmp/MOGUL.EXTRACT"

for (dir,dirs,files) in os.walk(POPULATE_DIR):
    for file in files:
        match = PKG_RE.match(file)
        if match:
            main,format,platform,version = match.group(1,2,3,4)
            if FORMAT_TO_OZMAKE.has_key(format):
                if os.path.exists(EXTRACT_DIR):
                    SYSTEM("rm -rf %s" % EXTRACT_DIR)
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
                            # we only convert things that are uploaded
                            # and out of date
                            doit = True
                        if doit:
                            if extracted is False:
                                if SYSTEM("%s --dir=%s -xp %s" % (cmd,EXTRACT_DIR,filename)) == 0:
                                    extracted = True
                                else:
                                    extracted = None
                            if extracted:
                                newcmd = FORMAT_TO_OZMAKE[newformat]
                                SYSTEM("%s --dir=%s -cp %s" % (newcmd,EXTRACT_DIR,new))

#sys.exit(0)

# lookup up all the files now present in the POPULATE_DIR
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

for (dir,dirs,files) in os.walk(POPULATE_DIR):
    pdir = dir[POPULATE_DIR_N:]
    for file in files:
        pfile = "%s/%s" % (pdir,file)
        ffile = "%s/%s" % (dir,file)
        AVAILABLE.append(Available(pfile,ffile))

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

def populate_specific(area,format,platform):
    TRACE("")
    TRACE("POPULATE SPECIFIC: %s" % area)
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

def populate_link(src,dst):
    if not os.path.exists(dst):
        dir = os.path.dirname(dst)
        if not os.path.exists(dir):
            os.makedirs(dir)
        TRACE("  POPULATE LINK: %s -> %s" % (src,dst))
        os.link(src,dst)
    elif os.stat(src).st_ino != os.stat(dst).st_ino:
        os.unlink(dst)
        TRACE("  POPULATE LINK: %s -> %s" % (src,dst))
        os.link(src,dst)

ALL_PLATFORMS = ['source']

SPECIFIC_DIR = "tmp/specific"

for format in ALL_FORMATS:
    for platform in ALL_PLATFORMS:
        populate_specific(("%s/%s/%s" % (SPECIFIC_DIR,format,platform)),format,platform)

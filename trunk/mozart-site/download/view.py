#! /usr/bin/python
import string,os,re,fpformat
import cgi,cgitb
cgitb.enable()
FORM = cgi.FieldStorage()

# turn a file containing "KEY : VAL" lines into a list of
# (KEY,VAL) pairs.  Ignore white space around KEY and VAL,
# and also ignore blank lines

def tokenize_file(file):
    f = open(file)
    lines = f.readlines()
    f.close()
    tokens = []
    for line in lines:
        line = string.strip(line)
        if line:
            key,val = string.split(line,":",1)
            key = string.lower(string.strip(key))
            val = string.strip(val)
            tokens.append((key,val))
    return tokens

# in the new régime, just raising strings is frowned upon,
# so we raise real exception objects

class Error:
    def __init__(self,msg):
        self.msg = msg
    def __str__(self,msg):
        return "<Error: " + msg + ">"

# an Entry represents a file that is being contributed: typically
# providing some module in some format on some platform.

class Entry:
    def __init__(self,table):
        self.table = table
    def __str__(self):
        return '<Entry ' + str(self.table) + '>'

# a package corresponds to a PACKAGE file describing all the
# entries contributed, plus some info about mozart version, date
# of packaging, and name of packager

class Package:
    def __init__(self):
        self.mozart_version = None
        self.date           = None
        self.packager       = None
        self.entries        = []

# this function parses a PACKAGE file and returns a corresponding
# Package object.  The prefix argument is prepended to the filenames
# provided by the package: the result is not an absolute pathname
# but a relative one that can be used both for finding the files on
# the file system and for constructing frp urls to download them.

def parse(file,prefix):
    dir            = os.path.dirname(file)
    package        = Package()
    module         = None
    format         = None
    readme         = None
    pos            = None
    title          = None
    for key,val in tokenize_file(file):
        if   key == 'mozart-version':
            package.mozart_version = val
        elif key == 'date':
            package.date = val
        elif key == 'packager':
            package.packager = val
        elif key == 'module':
            module = string.lower(val)
        elif key == 'format':
            format = string.lower(val)
        elif key == 'readme':
            readme = prefix + "/" + val
        elif key == 'os':
            pos = string.lower(val)
        elif key == 'title':
            title = val
        elif key == 'file':
            table = {
                'package' : package,
                'module'  : module,
                'title'   : title,
                'format'  : format,
                'readme'  : readme,
                'os'      : pos,
                'size'    : os.path.getsize(dir + "/" + val),
                'file'    : prefix + '/' + val }
            package.entries.append(Entry(table))
            # clear some fields that are only valid for one file
            # and need to be specified repeatedly if necessary
            readme = None
        elif key == 'tag':
            table = {
                'package' : package,
                'module'  : module,
                'format'  : format,
                'readme'  : readme,
                'os'      : None,
                'tag'     : val }
            package.entries.append(Entry(table))
    return package

# get_packages(dir,prefix) is given as an argument a directory dir in
# which to find sub-directories, each one corresponding to one
# package, i.e.  containing one PACKAGE file.  It returns all
# corresponding package objects. The prefix argument is as explained
# for parse above, and, for each package, is extended with the name of
# the sub-directory in which this package resides.

def get_packages(dir,prefix):
    packages = []
    for d in os.listdir(dir):
        f = dir + "/" + d + "/PACKAGE"
        if os.path.isfile(f):
            packages.append(parse(f,prefix+"/"+d))
    return packages

######################################################################
# now, let's variously process this information for presentation to
# the user of our web download interface.
######################################################################

mozart_store = "/home/ftp/pub/mozart/store"
mozart_ftp   = "ftp://ftp.mozart-oz.org/pub/mozart/store"
mozart_http  = "http://www.mozart-oz.org/download/mozart-ftp/store"
mozart_current_version = "1.2.5"

mozart_packages = get_packages(mozart_store,mozart_http)
mozart_modules = ['mozart-base','mozart','mozart-doc','mozart-stdlib','mozart-contrib']

# figure out what versions of mozart are available
all_mozart_versions = {}
for p in mozart_packages:
    all_mozart_versions[p.mozart_version] = True
all_mozart_versions = all_mozart_versions.keys()
all_mozart_versions.sort()

def page_releases(action,version):
    page_div_begin()
    print "<p><b>Releases:",
    first = True
    for v in all_mozart_versions:
        if first:
            first = False
        else:
            print " - ",
        if v==version:
            print v,
        else:
            print "<a href='view.cgi?action="+action+"&version="+v+"'>"+v+"</a>"
    if action=='default':
        print " <span class='RIGHT'>[<a href='old-index.html'>Old Interface</a>]</span>"
    print "</p>"
    page_div_end()

def page_feedback():
    print """
  <table align='center'>
      <td align=center>
        <table cellpadding=10>
          <tr bgcolor='#e0e0f0'><td>
            We would appreciate if you could send E-mail to
            <blockquote>
              <a href="mailto:download@mozart-oz.org">
               <code>download@mozart-oz.org</code></a>
            </blockquote>
            with some brief information for what you plan to use Mozart.
         </td></tr>
        </table>
      </td>
    </tr>
  </table>
    """

def page_license():
    page_section("License")
    page_div_begin()
    print """
    Before downloading Mozart, you may wish to consult the
    <a href='http://www.mozart-oz.org/home/LICENSE.html'>Mozart License</a>.
    """
    page_div_end()

def page_rpm(mozart_version):
    # collect the rpms for this mozart version
    src = []
    bin = []
    for p in mozart_packages:
        if p.mozart_version == mozart_version:
            for e in p.entries:
                fmt = e.table['format']
                if fmt=='srcrpm':
                    src.append(e)
                elif fmt=='binrpm':
                    bin.append(e)
    # group rpms by module, sort modules, sort rpms
    src = presort_entries(src)
    bin = presort_entries(bin)
    # output boiler plate:
    page_begin("RPMs for Mozart "+mozart_version)
    page_releases('rpm',mozart_version)
    page_feedback()
    page_section("Download")
    page_div_begin()
    page_table_begin()
    page_table_header("Binary RPMs",6)
    print_entries_by_module(bin,'binrpm')
    page_table_header("Source RPMs",6)
    print_entries_by_module(src,'srcrpm')
    page_table_end()
    page_div_end()
    page_rpm_not_available()
    page_license()
    page_end()

def page_rpm_not_available():
    page_section("Your platform is not available yet?")
    page_div_begin()
    print """
    <p>We welcome contributions.  Consider downloading our source RPMs and
    building the binary RPMs yourself.  This is very easy.  As root invoke:
    </p>
    <pre>
        rpmbuild --rebuild XXX.src.rpm
    </pre>
    or, on older RPM systems:
    <pre>
        rpm --rebuild XXX.src.rpm
    </pre>
    where <code>XXX.src.rpm</code> is the source RPM.  This will typically
    create binary RPMs in /usr/src/redhat/RPMS/i386 or perhaps correspondingly
    for your architecture.  Please, contribute
    these binary RPMs back to us: we will place them on our download server
    so that others may benefit from your effort.
    """
    page_div_end()

def page_begin(title):
    print "<html><head><title>"+title+"</title>"
    print "<link rel='stylesheet' href='view.css' type='text/css'>"
    print "</head><body>"
    print """
    <form method="get" action="http://www.mozart-oz.org/forward.cgi">
  <table width="100%">
    <tr align=left>
      <td align=left>
        <a href="http://www.mozart-oz.org/"><img 
          src="/logos/mozart-259x112.gif" 
          alt="Mozart Home"
          border=0></a>
      </td>
      <td align=center>
	<a href="http://www.mozart-oz.org/mogul/">MOzart Global User Library&nbsp;&nbsp;&nbsp;&nbsp;[MOGUL Archive]</a><br>
        <small><select name=url>
        <option selected value="">Home</option>
        <option value="features.html">Feature Overview</option>
        <option value="download/view.cgi">Download</option>
	<option value="mogul">MOGUL Archive</option>
        <option value="license.html">License Information</option>
        <option value="demo">Demo</option>
        <option value="documentation">Documentation</option>
        <option value="papers">Publications</option>
        <option value="news.cgi">News</option>
        <option value="archive.cgi">Projects & Software</option>
        <option value="lists">Contact Information</option>
        <option value="people.html">Research Groups</option>
        <option value="misc">Miscellany</option>
        <option value="SUPPORT">Support</option>
        </select></small>
        <input type="submit" value="Go!">
      </td>
    </tr>
  </table>
</form>"""
    print "<h1>"+title+"</h1>"

def page_end():
    print "</body></html>"

def page_section(title):
    print "<h2>"+title+"</h2>"

def page_div_begin():
    print "<div class='OFF'>"

def page_div_end():
    print "</div>"

def page_table_begin():
    print "<table class='PKG' width='100%' cellspacing='0'>"

def page_table_end():
    print "</table>"

def page_table_header(title,colspan=6):
    print "<tr><th colspan='"+str(colspan)+"'>"+title+"</th></tr>"

def make_os_then_date_key(e):
    return (e.table.get('os','') or '') + ' ' + e.table['package'].date

def by_date_item(e):
    return e.table['package'].date,e

# we actually want the usual lexicographic order for the os key,
# but we want the newest entries first, i.e. reverse lexicographic
# order for date

def sort_by_os_then_date(entries):
    t = {}
    for e in entries:
        os = e.table.get('os','') or ''
        if t.has_key(os):
            t[os].append(e)
        else:
            t[os]=[e]
    items = t.items()
    items.sort()
    entries = []
    for k,l in items:
        l = map(by_date_item,l)
        l.sort()
        l.reverse()
        for d,e in l:
            entries.append(e)
    return entries

def sort_item_by_os_then_date(item):
    module,entries = item
    return module, sort_by_os_then_date(entries)

def make_module_key(item):
    module,entries = item
    if module in mozart_modules:
        return "A " + module
    else:
        return "B " + module

def presort_entries(entries):
    by_module = {}
    for e in entries:
        mod = e.table['module']
        if not by_module.has_key(mod):
            by_module[mod] = [e]
        else:
            by_module[mod].append(e)
    # for each module, we sort its corresponding entries using os
    # as primary key, then date as secondary key
    by_module = map(sort_item_by_os_then_date,by_module.items())
    # now we need to sort by order of decreasing importance of module
    # so that the most important modules are presented at the top of
    # the page (i.e. main mozart modules)
    l = map(make_module_key,by_module)
    l = zip(l,by_module)
    l.sort()
    return [ item for key,item in l ]

def print_entries_by_module(items,which):
    odd = True
    for module,entries in items:
        first = True
        for e in entries:
            if odd:
                tr = "<tr class='ODD'>"
                odd = False
            else:
                tr = "<tr class='EVEN'>"
                odd = True
            print tr
            if first:
                first = False
                print "<td>"+module+"</td>",
            else:
                print "<td></td>",
            label = 'UNKNOWN'
            if which=='binrpm':
                label = e.table['os']
            elif which=='srcrpm':
                label = 'source RPM'
            elif which=='windows':
                fmt = e.table['format']
                if fmt=='winexe':
                    label = 'Windows EXE'
                else:
                    label = 'Windows MSI'
            elif which=='srctar':
                label = 'source TAR'
            elif which=='bintar':
                # special case for HTML mozart doc
                if e.table['module'] == 'mozart-doc':
                    label = 'HTML TAR'
                else:
                    label = e.table['os']
            elif which=='macosx':
                label = 'MacOSX package'
            elif which=='debian':
                label = 'Debian package'
            elif which=='cvs-tag' or which=='cvs-branch':
                label = e.table['tag']
            file = e.table.get('file',None)
            if file:
                print "<td><a href='"+file+"'>"+label+"</a></td>",
            else:
                print "<td>"+label+"</td>"
            if file:
                print "<td>("+format_size(e)+")</td>"
            else:
                print "<td></td>"
            print "<td>"+e.table['package'].date+"</td>",
            readme = e.table['readme']
            if readme:
                print "<td><a href='"+readme+"'>README</a></td>",
            else:
                print "<td></td>",
            print "<td>"+e.table['package'].packager+"</td>",
            print "</tr>"

def format_size(e):
    size = int(e.table['size'])
    if size >= 1048576:
        size = fpformat.fix(size / 1048576.0,2) + "Mb"
    elif size > 1024:
        size = fpformat.fix(size / 1024.0,2) + "Kb"
    else:
        size = str(size) + "b"
    return size

######################################################################
# Windows
######################################################################

def page_windows(mozart_version):
    # collect windows entries for this version
    l = []
    for p in mozart_packages:
        if p.mozart_version == mozart_version:
            for e in p.entries:
                fmt = e.table['format']
                if fmt=='winexe' or fmt=='winmsi':
                    l.append(e)
    l = presort_entries(l)
    page_begin("Mozart "+mozart_version+" for Windows")
    page_releases('windows',mozart_version)
    page_feedback()
    page_section("Download")
    page_div_begin()
    page_table_begin()
    page_table_header("Windows EXE and MSI installers")
    print_entries_by_module(l,'windows')
    page_table_end()
    page_div_end()
    page_section("Notes")
    page_div_begin()
    print """
      <dl>
      <dt>Previous versions.</dt>
      <dd><p>Uninstall any previous release of Mozart you have on
	 your system.  If your previous version was pre-1.1.0-RC1,
	 reboot your machine after uninstalling.</p>
      <dt>Documentation format.</dt>
      <dd><p>Documentation in this release is distributed in CHM format.
	 To read it, you need Microsoft\'s HTML Help which you can download
	 <a href="ftp://ftp.mozart-oz.org/pub/mozart/extras/hhupd.exe"
	 >here</a>.  Recent versions of Windows come with HTML Help.</p>
      <dt>Windows Installer.</dt>
      <dd><p>Mozart setup now uses the Windows Installer, which also
	 comes with Office 2000 and Windows 2000.  If you do not have
	 Windows Installer on your system, download the <tt>exe</tt> above
	 and Mozart setup will install
	 it automatically.  On some systems, this requires a reboot.</p>
	 <p>We have received reports where Mozart setup failed to install
	 Windows Installer on NT4.  If you are running NT4, please
	 perform the following steps <em>before</em> running Mozart setup:</p>
	 <ol>
	 <li><p>Download the Windows Installer redistributable:</p>
	   <blockquote>
	     <a href="ftp://ftp.mozart-oz.org/pub/mozart/extras/instmsiW.exe"
	     >instmsiW.exe</a> for Windows NT4<br>
	     <a href="ftp://ftp.mozart-oz.org/pub/mozart/extras/instmsiA.exe"
	     >instmsiA.exe</a> for Windows 95/98
	   </blockquote>
	 <li><p>Run this file and reboot your machine.</p>
	 <li><p>Run Mozart setup.  Should you get the error message
	   "The Windows Installer service failed to start", then please perform
	   <a href="http://support.microsoft.com/support/kb/articles/Q251/2/74.ASP">
	   this workaround</a> described by Microsoft before running Mozart
	   setup again.</p>
	 </ol>
      <dt>Windows 98.</dt>
      <dd><p>Due to a bug in the Windows Installer service, the AUTOEXEC.BAT
	 file may be modified in the wrong way, resulting in boot problems.
	 You should either:</p>
	 <ul>
	 <li><p>make sure before installing that your AUTOEXEC.BAT uses the
	   <tt>set PATH=...</tt> syntax and <i>not</i> the <tt>PATH=...</tt>
	   syntax; or</p>
	 <li><p>revise the changes made to AUTOEXEC.BAT after installation
	   and before rebooting.</p>
	 </ul>
      </dl>"""
    page_div_end()
    page_section("Additional Packages")
    page_div_begin()
    page_table_begin()
    print "<tr><th><a href='ftp://ftp.mozart-oz.org/pub/mozart/extras/emacs-20.7.exe'>emacs</a></th><td>The Emacs editor</td></tr>"
    page_table_end()
    page_div_end()
    page_license()
    page_end()

######################################################################
# Gentoo
######################################################################

def page_gentoo(mozart_version):
    page_begin("Gentoo Ebuilds for Mozart")
    page_feedback()
    page_section("Getting and Using The Mozart Ebuilds")
    page_div_begin()
    print """<p>The mozart system and its addons are now available through
ebuilds for <a href='http://www.gentoo.org'>Gentoo</a>'s portage
system.  Our ebuilds are maintained in our CVS and should be
obtained from it:</p>
<p>First, you need to log once into our anonymous CVS server
(cvs will record this in your ~/.cvspass file and you won't
need to log in again on subsequent occasions):</p>
<pre>cvs -d :pserver:anoncvs@cvs.mozart-oz.org:/services/mozart/CVS login</pre>
<p>when prompted for a password, answer <tt>anoncvs</tt>.
Then the Mozart portage tree can be obtained as follows:
<pre>cvs -d :pserver:anoncvs@cvs.mozart-oz.org:/services/mozart/CVS get portage</pre>
<p>Let us assume that you have downloaded the Mozart portage
tree into <file>$HOME/Mozart/portage</file>, you can take advantage of it by setting
environment variable <tt>PORTDIR_OVERLAY</tt> to this directory and then
invoking <tt>emerge</tt> as usual.  For example <tt>emerge mozart</tt> gives
you a very complete mozart installation, including GTK support if you have
<tt>gtk</tt> in your <tt>USE</tt> variable.  Components of a mozart installation
can be emerged separately if you prefer (e.g. for a leaner installation).</p>"""
    page_div_end()
    page_license()
    page_end()

######################################################################
# Tarballs
######################################################################

def page_tar(mozart_version):
    # collect the tarballs for this mozart version
    src = []
    bin = []
    for p in mozart_packages:
        if p.mozart_version == mozart_version:
            for e in p.entries:
                fmt = e.table['format']
                if fmt == 'srctar':
                    src.append(e)
                elif fmt == 'bintar':
                    bin.append(e)
    # group tarballs by module, sort modules, sort tarballs
    src = presort_entries(src)
    bin = presort_entries(bin)
    page_begin("Tarballs for Mozart "+mozart_version)
    page_releases('tar',mozart_version)
    page_feedback()
    page_section("Download")
    page_div_begin()
    page_table_begin()
    page_table_header("Binary TARs",6)
    print_entries_by_module(bin,'bintar')
    page_table_header("Source TARs",6)
    print_entries_by_module(src,'srctar')
    page_table_end()
    page_div_end()
    page_tar_not_available()
    page_license()
    page_end()

def page_tar_not_available():
    page_section("Your platform is not available yet?")
    page_div_begin()
    print """
    <p>We welcome contributions.  Consider downloading our source tarballs
    and building from source yourself.  See the <a href=
    'http://www.mozart-oz.org/documentation/install/index.html'
    >System Installation Manual</a> for details.  When you are
    done, consider contributing a binary tarball back to us so that
    others may benefit from your effort.  For help with this, don't
    hesitate to ask on the Mozart users group.
    """
    page_div_end()


######################################################################
# MacOSX
######################################################################

def page_macosx(mozart_version):
    l = []
    for p in mozart_packages:
        if p.mozart_version == mozart_version:
            for e in p.entries:
                if e.table['os'] == 'macosx':
                    l.append(e)
    l = presort_entries(l)
    page_begin("MacOSX Packages for Mozart "+mozart_version)
    page_releases('macosx',mozart_version)
    page_feedback()
    page_section("Download")
    page_div_begin()
    page_table_begin()
    page_table_header("MacOSX Packages")
    print_entries_by_module(l,"macosx")
    page_table_end()
    page_div_end()
    page_license()
    page_end()

######################################################################
# Debian
######################################################################

debian_re = re.compile("debian")

def page_debian(mozart_version):
    l = []
    for p in mozart_packages:
        if p.mozart_version == mozart_version:
            for e in p.entries:
                os = e.table['os']
                if os and debian_re.match(os):
                    l.append(e)
    l = presort_entries(l)
    page_begin("Debian Packages for Mozart "+mozart_version)
    page_releases('debian',mozart_version)
    page_feedback()
    page_section("Automated Updates")
    page_div_begin()
    print """
    <p>The easiest way to to obtain the Debian packages for Mozart is
    to add the following line to your <file>/etc/apt/sources.list</file>:</p>
    <pre>deb ftp://ftp.mozart-oz.org/pub/mozart/latest/debian ./</pre>
    <p>and to run <code>apt-get update</code> (to update your local package
    database), followed by either:</p>
    <pre>apt-get install mozart mozart-contrib mozart-doc-html</pre>
    <p>(in case you have not previously installed Mozart on your
    system) or <code>apt-get upgrade</code>. If you <code>update</code>
    and <code>upgrade</code> regularly, you will always have the most recent
    version of Mozart on your system. Of course, you can also manually
    download the packages and install them using your favourite package
    manager.</p>
    """
    page_div_end()
    page_section("Download")
    page_div_begin()
    page_table_begin()
    page_table_header("Debian Packages")
    print_entries_by_module(l,"debian")
    page_table_end()
    page_div_end()
    page_end()

######################################################################
# CVS
######################################################################

def page_cvs(mozart_version):
    tag = []
    branch = []
    for p in mozart_packages:
        if p.mozart_version == mozart_version:
            for e in p.entries:
                fmt = e.table['format']
                if fmt == 'cvs-tag':
                    tag.append(e)
                elif fmt == 'cvs-branch':
                    branch.append(e)
    tag    = presort_entries(tag)
    branch = presort_entries(branch)
    page_begin("Mozart "+mozart_version+" by Anonymous CVS")
    page_releases('cvs',mozart_version)
    page_feedback()
    page_div_begin()
    print """
    <p>The Mozart system is also available directly from our anonymous CVS
    server.  Tagged releases should be safe.  Any other snapshot may have
    bugs, or may not even build: you have been warned!</p>"""
    page_div_end()
    page_section("CVS Tags")
    page_div_begin()
    print """
    <p>When an explicit tag is omitted in the table below, it just means that
    you also shouldn't supply a <code>-r TAG</code> option to cvs when retrieving the
    corresponding module from our CVS.</p>
    """
    page_table_begin()
    page_table_header("CVS Release Tags")
    print_entries_by_module(tag,'cvs-tag')
    page_table_header("CVS Branch Tags")
    print_entries_by_module(branch,'cvs-branch')
    page_table_end()
    page_div_end()
    page_section("Instructions")
    page_div_begin()
    print """
    <p>First, you need to log once into our anonymous CVS server (cvs will record this
    in your ~/.cvspass file and you won't have to log in again on subsequent occasions):</p>
    <pre>cvs -d :pserver:anoncvs@cvs.mozart-oz.org:/services/mozart/CVS login</pre>
    <p>when prompted for a password, answer <tt>anoncvs</tt>.  Then you can retrieve the
    module you are interested in as follows:</p>
    <pre>cvs -d :pserver:anoncvs@cvs.mozart-oz.org:/services/mozart/CVS get -d TAG MODULE</pre>
    <p>where <code>TAG</code> is given by the table above and <code>MODULE</code> is
    <code>mozart</code>, <code>mozart-stdlib</code> or whatever else appears in the table
    above.</p>
    """
    page_div_end()
    page_license()
    page_end()

######################################################################
# Printed Documentation
######################################################################

def page_print(mozart_version):
    # collect the documentation for this mozart version
    tutorials  = []
    references = []
    tools      = []
    others     = []
    for p in mozart_packages:
        if p.mozart_version == mozart_version:
            for e in p.entries:
                mod = e.table['module']
                if mod == 'print-tutorial':
                    tutorials.append(e)
                elif mod == 'print-reference':
                    references.append(e)
                elif mod == 'print-tools':
                    tools.append(e)
                elif mod == 'print-other':
                    others.append(e)
    page_begin("Printed Documentation for Mozart "+mozart_version)
    page_releases('print',mozart_version)
    page_div_begin()
    page_table_begin()
    page_print_section("Tutorials",tutorials)
    page_print_section("Reference",references)
    page_print_section("Tools",tools)
    page_print_section("Other",others)
    page_table_end()
    page_div_end()
    page_end()

def page_print_section(section,entries):
    # for print documentation, we should only present the most recent version
    # table mapping title -> format -> entry
    table = {}
    for e in entries:
        title = e.table['title']
        fmt   = e.table['format']
        if table.has_key(title):
            x = table[title]
        else:
            x = {}
            table[title] = x
        if x.has_key(fmt):
            y = x[fmt]
            if e.table['package'].date > y.table['package'].date:
                x[fmt] = e
        else:
            x[fmt] = e
    l = table.items()
    l.sort()
    page_table_header(section, 5)
    odd = True
    for title,fmts in l:
        if odd:
            odd = False
            tr  = "<tr class='ODD'>"
        else:
            odd = True
            tr  = "<tr class='EVEN'>"
        print tr+"<td>"+title+"</td>"
        psgz  = fmts.get('ps.gz',None)
        pdfgz = fmts.get('pdf.gz',None)
        if psgz:
            print "<td><a href='"+psgz.table['file']+"'>PS.gz</a></td><td>("+format_size(psgz)+")</td>"
        else:
            print "<td></td><td></td>"
        if pdfgz:
            print "<td><a href='"+pdfgz.table['file']+"'>PDF.gz</a></td><td>("+format_size(pdfgz)+")</td>"
        else:
            print "<td></td><td></td>"
        print "</tr>"

######################################################################
# default page
######################################################################

def page_default(mozart_version):
    page_begin("Download Mozart")
    page_releases('default',mozart_version)
    page_div_begin()
    print """<p>General information on how to install Mozart can be found in
    <a href="http://www.mozart-oz.org/documentation/install/index.html">Installing Mozart</a></p>
    <dl>
    <dt><a href="view.cgi?action=windows">Windows Installers (95/98/ME/NT/2000/XP)</a></dt>
    <dd>Mozart for Windows comes as a standard self extracting archive.
    <dt><a href="view.cgi?action=rpm">RPMs for GNU/Linux</a></dt>
    <dd>For RPM-based GNU/Linux distributions, you should install our binary RPMs.
    Source RPMs are also available.
    <dt><a href="view.cgi?action=macosx">MacOS X Packages</a></dt>
    <dd>Mozart for MacOS X is available as packages.
    <dt><a href="view.cgi?action=tar">Tarballs</a></dt>
    <dd>We provide source and binary tarballs.  If a binary tarball is available
    for your system: you just have to unpack it and start using it.  Source tarballs
    are also available if you want or need to compile Mozart for your platform.
    <dt><a href="view.cgi?action=debian">Debian Packages</a></dt>
    <dd>If your installation uses Debian packages, this is where you can get automated
    updates, packages, and sources.
    <dt><a href="view.cgi?action=gentoo">Gentoo ebuilds</a></dt>
    <dd>We now provide ebuilds for the Gentoo distribution of GNU/Linux.  Thus
    you can take advantage of <i>portage</i> to install and maintain your
    installation of Mozart.
    <dt><a href="view.cgi?action=cvs">CVS</a></dt>
    <dd>Mozart source code may also be downloaded directly from our anonymous
    CVS server.
    <dt><a href="view.cgi?action=print">Printed Documentation</a></dt>
    <dd>If you prefer documentation in a printed form, we provide Postscript and
    PDF versions of all of our manuals and tutorials.
    </dl>"""
    page_div_end()
    page_end()

######################################################################
# process the CGI request
######################################################################

print "Content-type: text/html"
print
print '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">'

ACTION  = FORM.getvalue('action','default')
VERSION = FORM.getvalue('version',mozart_current_version)

if ACTION == 'rpm':
    page_rpm(VERSION)
elif ACTION == 'windows':
    page_windows(VERSION)
elif ACTION == 'gentoo':
    page_gentoo(VERSION)
elif ACTION == 'tar':
    page_tar(VERSION)
elif ACTION == 'default':
    page_default(VERSION)
elif ACTION == 'macosx':
    page_macosx(VERSION)
elif ACTION == 'debian':
    page_debian(VERSION)
elif ACTION == 'cvs':
    page_cvs(VERSION)
elif ACTION == 'print':
    page_print(VERSION)
else:
    raise Error("unknown action: "+ACTION)


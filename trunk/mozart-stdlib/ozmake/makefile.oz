makefile(
   uri   : 'x-ozlib://duchier/tool'
   mogul : 'mogul:/duchier/ozmake'
   author: 'mogul:/duchier/denys'
   lib   : ['ozmake.ozf']
   bin   : ['ozmake.exe']
   doc   : ['index.html' 'CHANGES']
   rules :
      o(
	 'ozmake.ozf' : ozl('Main.ozf')
	 'ozmake.exe' : ozl('ozmake.ozf' [executable]))
   depends :
      o(
	 'ozmake.ozf' :
	    ['Main.ozf' 'Utils.ozf' 'Path.ozf' 'Shell.ozf'
	     'Executor.ozf' 'Attribs.ozf' 'Builder.ozf'
	     'Makefiler.ozf' 'Manager.ozf' 'Installer.ozf'
	     'Database.ozf' 'Cleaner.ozf' 'Creator.ozf'
	     'Extractor.ozf' 'Lister.ozf' 'Help.ozf'
	     'Uninstaller.ozf' 'Errors.ozf' 'Windows.ozf'
	     'MakeGUI.ozf' 'Fixes.ozf' 'DatabaseLib.ozf'
	     'Config.ozf' 'Mogul.ozf' 'Pickler.ozf'
	     'ExecutorFast.ozf' 'Depends.ozf']
	 'Help.ozf' :
	    ['Utils.ozf' 'Path.ozf' 'Windows.ozf'
	     'Shell.ozf' 'HELP.txt'])
   blurb : "a tool for project building and package management"
   categories : [tool]
   version: "0.87"
   info_html:
      '<P>
<SPAN CLASS="TOOL">ozmake</SPAN> is a tool for building Mozart-based projects
and for creating and installing Mozart packages.  It was inspired by
the Unix tools <SPAN CLASS="TOOL">make</SPAN> and <SPAN CLASS="TOOL">rpm</SPAN>, but is much,
much simpler and specialized for Mozart-based software development and
deployment.
<SPAN CLASS="TOOL">ozmake</SPAN> must currently be invoked from a shell, but it
will eventually acquire additionally an optional, user-friendly
graphical interface.
<b>Precompiled versions for both Unix and Windows can be downloaded from the documentation
page</b></P>')


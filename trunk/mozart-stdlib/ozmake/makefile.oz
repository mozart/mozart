makefile(
   uri   : 'x-oz://duchier/tool'
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
	     'Makefile.ozf' 'Manager.ozf' 'Installer.ozf'
	     'Database.ozf' 'Cleaner.ozf' 'Creator.ozf'
	     'Extractor.ozf' 'Lister.ozf' 'Help.ozf'
	     'Uninstaller.ozf' 'Errors.ozf' 'Windows.ozf'
	     'MakeGUI.ozf' 'Fixes.ozf' 'DatabaseLib.ozf'
	     'Config.ozf' 'Mogul.ozf']
	 'Help.ozf' :
	    ['Utils.ozf' 'Path.ozf' 'Windows.ozf'
	     'Shell.ozf' 'HELP.txt']))

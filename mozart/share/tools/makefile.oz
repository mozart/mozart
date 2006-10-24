makefile(
   uri     : 'x-oz://system'
   mogul   : 'mogul:/mozart/tools'
   lib     : ['Browser.ozf' 'CompilerPanel.ozf' 'Emacs.ozf'
	      'EvalDialog.ozf' 'Explorer.ozf' 'Gump.ozf'
	      'GumpScanner.ozf' 'GumpParser.ozf'
	      'ProductionTemplates.ozf' 'OPIEnv.ozf' 'OPI.ozf'
	      'OPIServer.ozf' 'Ozcar.ozf' 'Panel.ozf'
	      'Profiler.ozf']
   depends :
      o('Browser.ozf'
	: ['browser/XResources.oz' 'browser/browserObject.oz'
	   'browser/browserTerm.oz' 'browser/bufsAndStreams.oz'
	   'browser/constants.oz' 'browser/controlObject.oz'
	   'browser/core.oz' 'browser/errors.oz'
	   'browser/managerObject.oz' 'browser/reflect.oz'
	   'browser/repManager.oz' 'browser/store.oz'
	   'browser/tcl-interface.oz' 'browser/termObject.oz'
	   'browser/termsStore.oz' 'browser/windowManager.oz']
	'CompilerPanel.ozf'
	: ['compilerPanel/CompilerPanelClass.oz']
	'Explorer.ozf'
	: ['explorer/main.oz' 'explorer/misc.oz'
	   'explorer/action-nodes.oz' 'explorer/hide-nodes.oz'
	   'explorer/layout-nodes.oz' 'explorer/manager.oz'
	   'explorer/menu-manager.oz' 'explorer/move-nodes.oz'
	   'explorer/tk-nodes.oz' 'explorer/configure-static.oz'
	   'explorer/configure-dynamic.oz' 'explorer/combine-nodes.oz'
	   'explorer/nodes.oz' 'explorer/toplevel-manager.oz'
	   'explorer/default-actions.oz' 'explorer/search-nodes.oz'
	   'explorer/stat-nodes.oz' 'explorer/statistics-balloon.oz'
	   'explorer/dialog-manager.oz' 'explorer/shapes-and-images.oz'
	   'explorer/status-manager.oz' 'explorer/errors.oz']
	'Gump.ozf'
	: ['gump/Bison.oz' 'gump/Main.oz' 'gump/Output.oz'
	   'gump/ParserGenerator.oz' 'gump/ScannerGenerator.oz']
	'GumpScanner.ozf'
	: ['gump/GumpScannerClass.oz' 'gump/Errors.oz']
	'GumpParser.ozf'
	: ['gump/GumpParserClass.oz']
	'ProductionTemplates.ozf'
	: ['gump/ProductionTemplates.ozg']
	'Ozcar.ozf'
	: ['ozcar/prelude.oz' 'ozcar/string.oz' 'ozcar/tk.oz'
	   'ozcar/config.oz' 'ozcar/help.oz' 'ozcar/tree.oz'
	   'ozcar/thread.oz' 'ozcar/stack.oz' 'ozcar/source.oz'
	   'ozcar/menu.oz' 'ozcar/dialog.oz' 'ozcar/gui.oz'
	   'ozcar/ozcar.oz']
	'Panel.ozf'
	: ['panel/configure.oz' 'panel/load.oz' 'panel/make-notes.oz'
	   'panel/runtime-bar.oz' 'panel/top.oz' 'panel/main.oz'
	   'panel/dialogs.oz' 'panel/errors.oz']
	'Profiler.ozf'
	: ['profiler/prof-gui.oz' 'profiler/prof-prelude.oz'
	   'profiler/prof-tk.oz' 'profiler/prof-config.oz'
	   'profiler/prof-help.oz' 'profiler/profiler.oz'
	   'profiler/prof-dialog.oz' 'profiler/prof-menu.oz'
	   'profiler/prof-string.oz']
       )
   subdirs : ['dpanel']
   )

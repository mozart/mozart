functor
export
   'class' : Manager
import
   Makefile    at 'Makefiler.ozf'
   Executor    at 'Executor.ozf'
   Attribs     at 'Attribs.ozf'
   Builder     at 'Builder.ozf'
   Installer   at 'Installer.ozf'
   DatabaseLib at 'DatabaseLib.ozf'
   Database    at 'Database.ozf'
   Cleaner     at 'Cleaner.ozf'
   Creator     at 'Creator.ozf'
   Extractor   at 'Extractor.ozf'
   Lister      at 'Lister.ozf'
   Uninstaller at 'Uninstaller.ozf'
   MakeGUI     at 'MakeGUI.ozf'
   Config      at 'Config.ozf'
   Mogul       at 'Mogul.ozf'
   ExecutorFast at 'ExecutorFast.ozf'
   Depends     at 'Depends.ozf'
define
   class Manager
      from
	 Attribs    .'class'
	 Makefile   .'class'
	 Executor   .'class'
	 Builder    .'class'
	 Installer  .'class'
	 DatabaseLib.'class'
	 Database   .'class'
	 Cleaner    .'class'
	 Creator    .'class'
	 Extractor  .'class'
	 Lister     .'class'
	 Uninstaller.'class'
	 MakeGUI    .'class'
	 Config     .'class'
	 Mogul      .'class'
	 ExecutorFast.'class'
	 Depends    .'class'

      meth init
	 {self exec_init}
      end

      meth initAsSubdir(Subdir Superman)
	 {self set_assubdir(Subdir)}
	 {self set_superman(Superman)}
	 Manager,init
      end

      meth subdirs_to_managers(L $)
	 {Map L fun {$ D} {New Manager initAsSubdir(D self)} end}
      end

      meth recurse(CMD)
	 if {self get_local($)} then skip else
	    for
	       D in {self get_subdirs($)}
	       M in {self get_submans($)}
	    do
	       {self trace('entering '#D)}
	       {self incr}
	       try {M makefile_read} {M CMD}
	       finally {self decr} end
	       {self trace('leaving '#D)}
	    end
	 end
      end
   end
end

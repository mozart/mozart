functor
export
   'class' : Manager
import
   Makefile    at 'Makefile.ozf'
   Executor    at 'Executor.ozf'
   Attribs     at 'Attribs.ozf'
   Builder     at 'Builder.ozf'
   Installer   at 'Installer.ozf'
   Database    at 'Database.ozf'
   Cleaner     at 'Cleaner.ozf'
   Creator     at 'Creator.ozf'
   Extractor   at 'Extractor.ozf'
   Lister      at 'Lister.ozf'
   Uninstaller at 'Uninstaller.ozf'
define
   class Manager
      from
	 Attribs    .'class'
	 Makefile   .'class'
	 Executor   .'class'
	 Builder    .'class'
	 Installer  .'class'
	 Database   .'class'
	 Cleaner    .'class'
	 Creator    .'class'
	 Extractor  .'class'
	 Lister     .'class'
	 Uninstaller.'class'

      meth init
	 {self exec_init}
      end
   end
end

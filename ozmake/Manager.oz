functor
export
   'class' : Manager
import
   Makefile at 'Makefile.ozf'
   Executor at 'Executor.ozf'
   Attribs  at 'Attribs.ozf'
   Builder  at 'Builder.ozf'
define
   class Manager
      from
	 Attribs .'class'
	 Makefile.'class'
	 Executor.'class'
	 Builder .'class'

      meth init
	 {self exec_init}
      end
   end
end

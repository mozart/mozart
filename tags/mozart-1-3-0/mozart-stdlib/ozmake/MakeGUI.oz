functor
export
   'class' : MakeGUI
import
   QTK at 'x-oz://system/QTk.ozf'
define
   class MakeGUI

      meth makefileEdit()
	 {self makefile_read}
	 {{QTK.build
	   lr(
	      label(text:"Project File Editor" glue:w) continue newline
	      label(text:"Lib" glue:w)
	      {List.toTuple td
	       {Map {self get_lib_targets($)}
		fun {$ T} label(text:T glue:w) end}}
	      )} show}
	 {Wait _}
      end

   end
end

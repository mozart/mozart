functor
import
   Narrator('class')
   ErrorListener('class')
   Application(getArgs)
   Compiler(parseOzFile)
   Inspector(inspect)
   System(printInfo)
define
   PrivateNarratorO
   NarratorO = {New Narrator.'class' init(?PrivateNarratorO)}
   ListenerO = {New ErrorListener.'class' init(NarratorO)}

   case {Application.getArgs plain} of [FileName] then
      {Inspector.inspect
       {Compiler.parseOzFile FileName PrivateNarratorO
	fun {$ Switch} Switch == gump end {NewDictionary}}}
      if {ListenerO hasErrors($)} then
	 {System.printInfo {ListenerO getVS($)}}
	 {ListenerO reset()}
      end
   end
end

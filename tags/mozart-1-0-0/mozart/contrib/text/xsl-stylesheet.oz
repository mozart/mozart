functor
import
   XML('class': XML_Parser  ) at 'xml.ozf'
   XSL('class': XSL_Compiler) at 'xsl-compiler.ozf'
export
   'class' : XSL_StyleSheet
define
   Parser   = {New XML_Parser   init}
   Compiler = {New XSL_Compiler init}
   class XSL_StyleSheet
      feat data
      meth init(file:File<=unit vs:VS<=unit)
	 self.data =
	 {Compiler
	  process(if File\=unit then
		     {Parser parseFile(File $)}
		  else
		     {Parser parseVs(VS $)}
		  end $)}
      end
   end
end

functor
import
   Application
   Parser at 'Parser.ozf'
   NameSpaces at 'NameSpaces.ozf'
   Inspector(inspect:Inspect)
define
   Args = {Application.getArgs
	   record(
	      file(single type:string char:&i optional:false)
	      )}
   PrefixMap = {NameSpaces.newPrefixMap}
   local
      PACKAGE = {PrefixMap.intern unit package}.key
      HEAD    = {PrefixMap.intern unit head   }.key
      AUTHOR  = {PrefixMap.intern unit author }.key
      SECTION = {PrefixMap.intern unit section}.key
      DLIST   = {PrefixMap.intern unit dlist  }.key
      ITEM    = {PrefixMap.intern unit item   }.key
      ALIGN   = {PrefixMap.intern unit align  }.key
      ROW     = {PrefixMap.intern unit row    }.key
   in
      StripSpaces =
      o(PACKAGE : true
	HEAD    : true
	AUTHOR  : true
	SECTION : true
	DLIST   : true
	ITEM    : true
	ALIGN   : true
	ROW     : true)
   end
   Document = {Parser.new init(url         : Args.file
			       prefixMap   : PrefixMap
			       stripSpaces : StripSpaces)}
   {Inspect Document}
end
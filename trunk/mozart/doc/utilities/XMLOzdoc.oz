functor
import
   Parser at 'XMLParser.ozf'
export
   Parse
   namePI : PI
prepare
   fun {StringToLower S} {Map S Char.toLower} end
   fun {StringToUpper S} {Map S Char.toUpper} end
   fun {AtomToLower A}
      {StringToAtom {StringToLower {AtomToString A}}}
   end
   fun {TransformAttrib Tag A}
      case A of attribute(kind:_ name:K value:V) then
	 Key = {AtomToLower K}
      in
	 Key #
	 case Key
	 of 'class' then {Map {String.tokens {StringToLower V} & } StringToAtom}
	 [] 'name' then
	    if Tag=='meta' then {StringToAtom {StringToLower V}}
	    else V end
	 [] 'type' then
	    if {Member Tag ['math'  'picture']} then
	       {StringToAtom {StringToUpper V}}
	    elseif {Member Tag ['math.extern' 'picture.extern' 'var' 'grammar.alt']} then
	       {StringToAtom {StringToLower V}}
	    else V end
	 [] 'display' then {StringToAtom {StringToLower V}}
	 [] 'to' then
	    if {Member Tag ['answer' 'ref' 'ptr']} then
	       {StringToAtom {StringToLower V}}
	    else
	       V
	    end
	 [] 'id' then {StringToAtom {StringToLower V}}
	 [] 'n' then
	    if {Member Tag ['list' 'item']} then
	       {StringToInt V}
	    else
	       V
	    end
	 [] 'colspan' then
	    if {Member Tag ['td' 'th']} then
	       {StringToInt V}
	    else V end
	 else V end
      end
   end
define
   PI
   fun {Transform E}
      case E
      of element(tag:T attributes:A linkAttributes:_ children:C) then
	 TAG = {AtomToLower T}
      in
	 {List.toRecord TAG
	  {Append {List.mapInd C fun {$ I C} I#{Transform C} end}
	   {Map A fun {$ X} {TransformAttrib TAG X} end}}}
      [] pi(A)   then PI(A)
      [] data(B) then {ByteString.toString B}
      end
   end
   fun {Parse File Reporter}
      {Transform
       try
	  ParserObj = {New Parser.'class' init(File)}
       in
	  {ParserObj process}
	  {ParserObj getDoc($)}.docElem
       catch error(...)=E then {Reporter E} end}
   end
end
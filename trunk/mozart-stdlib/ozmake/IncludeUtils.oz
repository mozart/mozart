functor
define
   class IncludeGetter
      attr
	 Files
	 Stack
      meth init
	 Files <- {NewDictionary}
	 Stack <- nil
      end
      meth processFile(F)
	 
      end
   end

   fun {SkipSpaces L}
      case L
      of &  |L then {SkipSpaces L}
      [] &\t|L then {SkipSpaces L}
      else L end
   end

   fun {DropToEOL L}
      case L
      of &\n|L then L
      [] _|L then {DropToEOL L}
      [] nil then nil end
   end

   fun {SkipCComment L}
      case L
      of &*|&/|L then L
      [] _|L then {SkipCComment L}
      [] nil then nil end
   end

   fun {IgnoreCLine L}
      case L
      of &/|&/|L then {DropToEOL L}
      [] &/|&*|L then
	 {IgnoreCLine {SkipCComment L}}
      [] &\n|L then L
      [] _|L then {IgnoreCLine L}
      [] nil then nil end
   end

   class CIncludeGetter from IncludeGetter
      meth processLine(L)
	 case L
	 of &#|L then L2={SkipSpaces L} in
	    case L2
	    of &i|&n|&c|&l|&u|&d|&e|L then
	    else {
      end
   end
end

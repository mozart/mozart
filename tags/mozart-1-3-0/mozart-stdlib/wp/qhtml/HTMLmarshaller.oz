functor
require Open
prepare
   R
   local
      fun{Loop V}
	 case V of &!|&E|&N|&T|&I|&T|&Y|& |Xs then
	    N R1
	    {List.takeDropWhile Xs fun{$ C} C\=&  end N R1}
	    R2={List.dropWhile R1 fun{$ C} C==&  end}
	 in
	    case R2 of &C|&D|&A|&T|&A|_ then
	       R3={List.dropWhile R2 fun{$ C} C\=&# end}
	       V R4
	       {List.takeDropWhile R3 fun{$ C} C\=&; end V R4}
	    in
	       r(n:{String.toInt {List.drop V 1}}
		 r:{VirtualString.toString "\&"#N#";"})|{Loop R4}
	    else
	       {Loop R2}
	    end
	 [] _|Xs then {Loop Xs}
	 else nil
	 end
      end
      F={New Open.file init(url:"charref.html")}
      Vs={F read(list:$ size:all)}
      {F close}
   in
      R={Loop Vs}
   end
export
   HTML2VS
   VS2HTML
define
   HTMLDict={NewDictionary}
   VSDict={NewDictionary}
   {ForAll R
    proc{$ E}
       HTMLDict.{String.toAtom E.r} := E.n
       VSDict.(E.n):=E.r
    end}

   VSDict.&\\:=&\\|&\\|nil
   VSDict.&\r:=""
   VSDict.&\n:="&#013&#010"
       
   fun{VS2HTML S}
      fun{Insert C S}
	 case C
	 of X|Xs then X|{Insert Xs S}
	 else {Loop S}
	 end
      end
      fun{Loop S}
	 case S
	 of X|Xs then
	    if {Dictionary.member VSDict X} then
	       {Insert VSDict.X Xs}
	    else
	       X|{Loop Xs}
	    end
	 else nil
	 end
      end
   in
      {Loop {VirtualString.toString S}}
   end

   fun{HTML2VS S}
      fun{Insert C S}
	 case C
	 of X|Xs then X|{Insert Xs S}
	 else {Loop S}
	 end
      end
      fun{GetCode B S}
	 case S
	 of &;|Xs then
	    A={String.toAtom {List.append B ";"}}
	 in
	    if {Dictionary.member HTMLDict A} then
	       HTMLDict.A|{Loop Xs}
	    else
	       {Insert {Atom.toString A} Xs}
	    end
	 [] X|Xs then {GetCode {List.append B X|nil} Xs}
	 else B|nil
	 end
      end
      fun{Loop S}
	 case S
	 of &&|Xs then {GetCode "\&" Xs}
	 [] &\r|Xs then {Loop Xs}
	 [] X|Xs then X|{Loop Xs}
	 else nil
	 end
      end
   in
      {Loop {VirtualString.toString S}}
   end
end

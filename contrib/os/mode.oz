local
   Bits = bits(
	     owner: o(read	:{BitString.make 9 [8]}
		      write	:{BitString.make 9 [7]}
		      execute	:{BitString.make 9 [6]}
		      all	:{BitString.make 9 [6 7 8]})
	     group: o(read	:{BitString.make 9 [5]}
		      write	:{BitString.make 9 [4]}
		      execute	:{BitString.make 9 [3]}
		      all	:{BitString.make 9 [3 4 5]})
	     other: o(read	:{BitString.make 9 [2]}
		      write	:{BitString.make 9 [1]}
		      execute	:{BitString.make 9 [0]}
		      all	:{BitString.make 9 [0 1 2]})
	     all  : o(read      :{BitString.make 9 [2 5 8]}
		      write	:{BitString.make 9 [1 4 7]}
		      execute	:{BitString.make 9 [0 3 6]}
		      all	:{BitString.make 9 [0 1 2 3 4 5 6 7 8]})
	     none : {BitString.make 9 nil}
	     user : Bits.owner
	     u    : Bits.owner
	     g    : Bits.group
	     o    : Bits.other
	     a    : Bits.all
	     others:Bits.other
	     )
in
   functor
   export
      Make Owner Group Other
   define

      None  = Bits.none

      local
	 fun {CHK I B N Which What}
	    if {IsOdd I div N}
	    then {BitString.disj B Bits.Which.What}
	    else B end
	 end
      in
	 fun {IntToBits I}
	    B0 = None
	    B1 = {CHK I B0   1 other execute}
	    B2 = {CHK I B1   2 other write  }
	    B3 = {CHK I B2   4 other read   }
	    B4 = {CHK I B3   8 group execute}
	    B5 = {CHK I B4  16 group write  }
	    B6 = {CHK I B5  32 group read   }
	    B7 = {CHK I B6  64 owner execute}
	    B8 = {CHK I B7 128 owner write  }
	    B9 = {CHK I B8 256 owner read   }
	 in B9 end
      end

      local
	 fun {CHK B N V}
	    if {BitString.get B N} then V+{Pow 2 N} else V end
	 end
      in
	 fun {BitsToInt B}
	    I0 = 0
	    I1 = {CHK B 0 I0}
	    I2 = {CHK B 1 I1}
	    I3 = {CHK B 2 I2}
	    I4 = {CHK B 3 I3}
	    I5 = {CHK B 4 I4}
	    I6 = {CHK B 5 I5}
	    I7 = {CHK B 6 I6}
	    I8 = {CHK B 7 I7}
	    I9 = {CHK B 8 I8}
	 in I9 end
      end

      fun {Encode Spec}
	 if     {IsInt       Spec} then {IntToBits Spec}
	 elseif {IsBitString Spec} then Spec
	 elseif {IsList      Spec} then
	    {FoldL Spec
	     fun {$ B V} {BitString.disj B {Encode V}} end None}
	 elseif {IsAtom      Spec} then Bits.Spec.all
	 elseif {IsTuple     Spec} then
	    Tab = Bits.{Label Spec}
	 in
	    {Record.foldL Spec
	     fun {$ B X} {BitString.disj B Tab.X} end None}
	 elseif {IsRecord    Spec} andthen mode=={Label Spec} then
	    {Record.foldLInd Spec
	     fun {$ F B X}
		Tab = Bits.F
	     in
		if {IsAtom X} then
		   {BitString.disj B Tab.X}
		else
		   {FoldL X
		    fun {$ B Y}
		       {BitString.disj B Tab.Y}
		    end B}
		end
	     end None}
	 else raise bad end end
      end

      fun {Make Spec}
	 {BitsToInt {Encode Spec}}
      end

      fun {GET M I} {BitString.get {Encode M} I} end

      Owner = owner(read	:fun {$ M} {GET M 8} end
		    write	:fun {$ M} {GET M 7} end
		    execute	:fun {$ M} {GET M 6} end)
      Group = group(read	:fun {$ M} {GET M 5} end
		    write	:fun {$ M} {GET M 4} end
		    execute	:fun {$ M} {GET M 3} end)
      Other = other(read	:fun {$ M} {GET M 2} end
		    write	:fun {$ M} {GET M 1} end
		    execute	:fun {$ M} {GET M 0} end)
   end
end

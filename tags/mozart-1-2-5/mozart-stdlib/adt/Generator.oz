functor
export
   Concat
   Tails
   Count
   Member
   Keys
   Items
   Entries
prepare
   
   %% create a new generator that is the concatenation of a list
   %% of generators
   
   fun {Concat Gs}
      C={NewCell Gs}
      fun {ConcatGenerator}
	 case {Access C}
	 of nil then raise empty end
	 [] G|Gs then
	    try {G}
	    catch E then
	       case E
	       of error(...) then raise E end
	       [] failure(...) then raise E end
	       else {Assign C Gs} {ConcatGenerator} end
	    end
	 end
      end
   in
      ConcatGenerator
   end

   %% generator for all non-empty tails of a list
   
   fun {Tails L}
      C={NewCell L}
      fun {TailsGenerator}
	 New
      in
	 case {Exchange C $ New}
	 of nil then New=nil raise empty end
	 [] H|T then New=T H
	 end
      end
   in
      TailsGenerator
   end

   %% counting generator.  the direction of counting (up or
   %% down) is given by the 3rd argument (the step).

   fun {Count From To By}
      if To==unit then {NewCountGenerator     From    By}
      elseif By<0 then {NewCountDownGenerator From To By}
      else             {NewCountUpGenerator   From To By}
      end
   end
   fun {NewCountGenerator From By}
      C={NewCell From}
      fun {CountGenerator}
	 Old New
      in
	 {Exchange C Old New}
	 New=Old+By
	 Old
      end
   in
      CountGenerator
   end
   fun {NewCountUpGenerator From To By}
      C={NewCell From}
      fun {CountUpGenerator}
	 Old New
      in
	 {Exchange C Old New}
	 if Old>To then New=Old raise empty end
	 else New=Old+By Old end
      end
   in
      CountUpGenerator
   end
   fun {NewCountDownGenerator From To By}
      C={NewCell From}
      fun {CountDownGenerator}
	 Old New
      in
	 {Exchange C Old New}
	 if Old<To then New=Old raise empty end
	 else New=Old+By Old end
      end
   in
      CountDownGenerator
   end

   %% generator for the elements of a list

   fun {Member L}
      C={NewCell L}
      fun {MemberGenerator}
	 New
      in
	 case {Exchange C $ New}
	 of nil then New=nil raise empty end
	 [] H|T then New=T H
	 end
      end
   in
      MemberGenerator
   end

   %% polymorphic generators for keys, items and entries.  work for
   %% records, arrays and dictionaries.

   DictKeys    = Dictionary.keys
   DictItems   = Dictionary.items
   DictEntries = Dictionary.entries
   ArrayHi     = Array.high
   ArrayLo     = Array.low
   RecToList   = Record.toList
   RecToListInd= Record.toListInd
   
   fun {Keys R}
      if     {IsRecord     R} then {Member {Arity R}}
      elseif {IsArray      R} then {Count {ArrayLo R} {ArrayHi R} 1}
      elseif {IsDictionary R} then {Member {DictKeys R}}
      end
   end

   fun {Items R}
      if     {IsRecord     R} then {Member {RecToList R}}
      elseif {IsArray      R} then {Member for I in {ArrayLo R}..{ArrayHi R} collect:C do {C R.I} end}
      elseif {IsDictionary R} then {Member {DictItems R}}
      end
   end

   fun {Entries R}
      if     {IsRecord     R} then {Member {RecToListInd R}}
      elseif {IsArray      R} then {Member for I in {ArrayLo R}..{ArrayHi R} collect:C do {C I#R.I} end}
      elseif {IsDictionary R} then {Member {DictEntries R}}
      end
   end
end

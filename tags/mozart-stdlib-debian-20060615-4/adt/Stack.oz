functor
export
   New NewFromList
prepare
   %% Given a cell that implements the mutable part of
   %% a stack, return an instance of an abstract stack
   %% datatype for this cell.
   fun {StackPack C}
      fun   {StackGet} New in
	 case {Exchange C $ New}
	 of nil then New=nil raise empty end
	 [] H|T then New=T H end
      end
      proc  {StackPut X} L in {Exchange C L X|L} end
      proc  {StackGetPut X Y} New in
	 case {Exchange C $ New}
	 of nil then New=nil X=Y
	 [] H|T then New=Y|T X=H
	 end
      end
      fun   {StackTop}
	 case {Access C} of H|_ then H
	 else raise empty end end
      end
      fun  {StackIsEmpty  } {Access C}==nil end
      proc {StackReset    } {Assign C nil} end
      fun  {StackToList   } {Access C} end
      fun  {StackToListKill} {Exchange C $ unit} end
      fun  {StackClone    } {NewFromList {Access C}} end
   in
      stack(
	 get         : StackGet
	 put         : StackPut
	 getPut      : StackGetPut
	 top         : StackTop
	 isEmpty     : StackIsEmpty
	 reset       : StackReset
	 toList      : StackToList
	 toListKill  : StackToListKill
	 clone       : StackClone)
   end
   %% these are the real public API
   fun {NewFromList L} {StackPack {NewCell L}} end
   fun {New} {NewFromList nil} end
end

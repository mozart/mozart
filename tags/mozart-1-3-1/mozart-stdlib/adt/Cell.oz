functor
export
   New
prepare
   fun {CellPack C}
      fun  {CellGet} {Access C} end
      proc {CellPut V} {Assign C V} end
      proc {CellGetPut V1 V2} {Exchange C V1 V2} end
      fun  {CellClone} {CellPack {NewCell {Access C}}} end
   in
      cell(
	 get    : CellGet
	 put    : CellPut
	 getPut : CellGetPut
	 clone  : CellClone)
   end
   fun {New V} {CellPack {NewCell V}} end
end
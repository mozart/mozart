
   proc{Print S}
\ifdef DEBUG_CHECK
      {System.show S}
\else
      skip
\endif
   end

   R2L = Record.toList
   fun {VectorToType V}
      if {IsList V}       then list
      elseif {IsTuple V}  then tuple
      elseif {IsRecord V} then record
      else
	 {Exception.raiseError
	  kernel(type VectorToType [V] vector 1
		 'Vector as input argument expected.')} illegal
      end
   end
   
   fun {VectorToList V}
      if {VectorToType V}==list then V
      else {R2L V}
      end
   end
   
   proc {RecordToTuple As I R T}
      case As of nil then skip
      [] A|Ar then R.A=T.I {RecordToTuple Ar I+1 R T}
      end
   end
   
   proc {VectorToTuple V ?T}
      case {VectorToType V}
      of list   then T={List.toTuple '#' V}
      [] tuple  then T=V
      [] record then
	 T={MakeTuple '#' {Width V}} {RecordToTuple {Arity V} 1 V T}
      end
   end

%end

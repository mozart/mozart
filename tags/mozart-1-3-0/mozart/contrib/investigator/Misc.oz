functor

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
export
   CounterClass
   GetCtVarNameAsAtom
   GetCtVarConstraintAsAtom
   MergeSuspLists
   MergeSuspLists1
   VectorToList

   MemberEqProp
   MemberEqVar
   VariableToVirtualString
   
   VarReflect
   VarEq     
   PropReflect
   PropEq     
   PropName   
   PropLocation
   PropIsFailed
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
import
   Reflect(varReflect
	   varEq
	   propReflect
	   propEq
	   propName
	   propLocation
	   propIsFailed)
   at 'x-oz://contrib/Reflect.ozf'
   CTB at 'x-oz://boot/CTB'
   Error
   Tables
   FS
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
prepare
   
   fun {Flatten Ls}
      {FoldL Ls
       fun {$ L R}
	  if {IsDet R} then
	     if R == nil then L else {Append L {Flatten R}} end
	  else
	     R|L
	  end
       end nil}
   end
   
   fun {VectorToList1 Vect}
      if {IsDet Vect} then
	 VectAsList = if     {IsList Vect}   then Vect
		      elseif {IsRecord Vect} then {Record.toList Vect}
		      else nil
		      end
      in
	 {Map VectAsList fun {$ E} {VectorToList1 E} end}
      else
	 [Vect]
      end
   end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
define

   local
      T = 'visualize constraints error'
   in
      {Error.registerFormatter vc
       fun {$ Exc}
	  case Exc
	  of vc(What Where) then	  
	     error(kind:  T
		   msg:   What#" in "#Where
		   items: nil) 
	  else
	     error(kind: T items: [line(oz(Exc))])
	  end
       end}
   end

   class CounterClass
      attr i:1
      meth init
	 skip
      end
      meth next(X)
	 X = @i i <- @i+1
      end
   end 
   
   fun {MergeSuspLists PropTable IgnoreList ReflVars}
      S = {FS.var.decl}
      Ignore = {FS.value.make IgnoreList}
   in
      {ForAll ReflVars
       proc {$ ReflVar}
	  {ForAll {Arity ReflVar.susplists}
	   proc {$ Event}
	      {ForAll ReflVar.susplists.Event
	       proc {$ Susp}
		  if Susp.type == propagator then
		     Id =  {Tables.getPropId PropTable Susp.reference}
		  in
		     if {FS.isIn Id Ignore} == false 
		     then
			{FS.include Id S}
		     else skip end
		  else skip end
	       end}
	   end}
       end}
      {FS.card S {FS.reflect.cardOf.lowerBound S}}
      {Map {FS.reflect.lowerBoundList S}
       fun {$ I}
	  {Tables.getProp PropTable I}
       end}
   end

   fun {MergeSuspLists1 PropTable ReflVar}
      S = {FS.var.decl}
   in
      {ForAll {Arity ReflVar.susplists}
       proc {$ Event}
	  {ForAll ReflVar.susplists.Event
	   proc {$ Susp}
	      if Susp.type == propagator then
		 {FS.include {Tables.getPropId PropTable Susp.reference} S}
	      else skip end
	   end}
       end}
      {FS.card S {FS.reflect.cardOf.lowerBound S}}
      {Map {FS.reflect.lowerBoundList S}
       fun {$ I}
	  {Tables.getProp PropTable I}
       end}
   end
   
   GetCtVarNameAsAtom       = CTB.getNameAsAtom   
   GetCtVarConstraintAsAtom = CTB.getConstraintAsAtom

   fun {VectorToList Vs} {Flatten {VectorToList1 Vs}} end 

   fun {VariableToVirtualString V}
      case {Value.status V}
      of kinded(free)  then {Value.toVirtualString V 1 1}
      [] kinded(int)   then {Value.toVirtualString V 1 1}
      [] kinded(fset)  then {Value.toVirtualString V 1 1}
      [] kinded(other) then {Value.toVirtualString V 1 1}
	 #'<'#{GetCtVarNameAsAtom V}#':'#{GetCtVarConstraintAsAtom V}#'>'
      else
	 {Exception.raiseError vc("Unexpected case" "VariableToVirtualString")}
	 error
      end
   end
   
   fun {MemberEqVar V Vs}
      if {IsDet V} then true
      else case Vs
	   of H|T then
	      if {VarEq V H} then true else {MemberEqVar V T} end 
	   else false end
      end
   end
   
   fun {MemberEqProp P Ps}
      if P == unit then true
      else case Ps of H|T then
	      if {PropEq P H} then true else {MemberEqProp P T} end
	   else false end
      end
   end
   
   VarReflect   = Reflect.varReflect
   VarEq        = Reflect.varEq
   PropReflect  = Reflect.propReflect
   PropEq       = Reflect.propEq
   PropName     = Reflect.propName
   PropLocation = Reflect.propLocation
   PropIsFailed = Reflect.propIsFailed
end

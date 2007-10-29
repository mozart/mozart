%%%
%%% Authors:
%%%     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
%%%     Alberto Delgado <adelgado@cic.puj.edu.co>
%%%     Alejandro Arbelaez <aarbelaez@puj.edu.co>
%%%
%%% Copyright:
%%%     Gustavo Gutierrez, 2006
%%%     Alberto Delgado, 2006
%%%     Alejandro Arbelaez, 2006
%%%
%%% Last change:
%%%   $Date: 2006-10-19T01:44:35.108050Z $ by $Author: ggutierrez $
%%%   $Revision: 2 $
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor

require
   CpSupport(vectorToType:   VectorToType
	     vectorToList:   VectorToList
	     vectorToTuple:  VectorToTuple
	    )
   
import
   GFD at 'x-oz://boot/geoz-int'
   Space
   System
prepare
   %%This record must reflect the IntRelType in gecode/int.hh
   Rt = '#'('=:':0     %Equality
	    '\\=:':1    %Disequality
	    '=<:':2    %Less or equal
	    '<:':3     %Less
	    '>=:':4    %Greater or equal
	    '>:':5     %Greater
	   )
   %%This record must reflect the IntConLevel
   Cl = '#'(
	   val: 0   %Value consistency
	   bnd: 1   %Bounds consistency
	   dom: 2   %Domain consistency
	   def: 3   %The default consistency for a constraint
	   )
   IA = '#'(
	   min: 0  %Select smallest value.
	   med: 1  %Select median value.
	   max: 2  %Select maximum value.
	   )
   
   Pk = '#'(
	   def:   0  %Make a default decision
	   speed: 1  %Prefer speed over memory consumption
	   memory:2  %Prefer little memory over speed
	   )

export
   %% Telling domains
   int   : FdInt
   dom   : FdDom
   decl  : FdDecl
   list  : FdList
   tuple : FdTuple
   record: FdRecord

   %% Reflection
   reflect : FdReflect

   %% Watching Domains
   watch   : FdWatch

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% Clasify!!!!%%%%%%%%%%%%%
   
   %% Propagators
   eq:                   Eq
   rel:                  Rel       
   linear:               Linear
   linear2:              Linear2
   linearR:              LinearR
   linearCR:             LinearCR
   count:                Count
   distinct2:            Distinct2
   mult:                 Mult
   %%bool_and: 	         Bool_and
   int_Gabs:             Abs       
   int_sortedness:       Int_sortedness
   
   %%Mozart Propagators (backward compatibility))
   int_sumCN:            Int_sumCN
  % int_disjoint:         Int_disjoint
   %int_reified_int:      Int_reified_int


   %%Miscellaneous propagators
   plus:            Plus
   plusD:           PlusD
   minus:           Minus
   minusD:          MinusD
   times:           Times
   timesD:          TimesD
   divI:            DivI
   less:            Less
   lessEq:          LessEq
   greater:         Greater
   greaterEq:       GreaterEq
%   disjoint:        Disjoint 
   disjointC:       DisjointC


   %%Generic Propagators
   sum:             Sum
   sumC:            SumC
   sumCN:           SumCN
  %   sumAC:           SumAC
  %   sumACN:          SumACN
   sumD:            SumD
   sumCD:           SumCD
   sumAC:           SumAC
   sumACN:          SumACN

   %%Reified Propagators
   reified:        Reified
   
   %%Symbolic propagators
   distinct:        Distinct
   distinctOffset:  DistinctOffset
   atMost:          AtMost
   atLeast:         AtLeast
   exactly:         Exactly
   
   %% Assignment propagators
   assign: Assign

   int_ext: Int_ext
   %%Propagators
   Abs
   sortedness: Int_sortedness

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   %% Distribution
   distribute : FdDistribute
   %% Miscelaneus
   inf : FdInf
   sup : FdSup
   is  : FdIs

   %% Relation types
   rt: Rt

   %% Consistency levels
   cl: Cl

   %% Propagation kind
   pk: Pk
   
   %% Integer assignment
   ia: IA
   %% Temporal:
   'prop' : Prop
define

   %% Telling domains
   FdInt = GFD.int
   local
      
      proc {ListDom Xs Dom}
	 case Xs of nil then skip
	 [] X|Xr then {FdInt Dom X} {ListDom Xr Dom}
	 end
      end
      
      proc {TupleDom N T Dom}
	 if N>0 then {FdInt Dom T.N} {TupleDom N-1 T Dom} end
      end
      
      proc {RecordDom As R Dom}
	 case As of nil then skip
	 [] A|Ar then {FdInt Dom R.A} {RecordDom Ar R Dom}
	 end
      end
      
   in
      fun {FdDecl}
	 {FdInt {GFD.inf}#{GFD.sup}}
      end
      
      proc {FdDom Dom Vec}
	 case {VectorToType Vec}
	 of list   then {ListDom Vec Dom}
	 [] tuple  then {TupleDom {Width Vec} Vec Dom}
	 [] record then {RecordDom {Arity Vec} Vec Dom}
	 end
      end

      fun {FdList N Dom}
	 if N>0 then {FdInt Dom}|{FdList N-1 Dom}
	 else nil
	 end
      end

      proc {FdTuple L N Dom ?T}
	 T={MakeTuple L N} {TupleDom N T Dom}
      end

      proc {FdRecord L As Dom ?R}
	 R={MakeRecord L As} {RecordDom As R Dom}
      end
   end

   %% Reflection
   local
      fun {NBSusps X}
	 {GFD.'reflect.nbProp' X} + {System.nbSusps X}
      end
   in
      FdReflect = reflect(min    : GFD.'reflect.min'
			  max    : GFD.'reflect.max'
			  size   : GFD.'reflect.size'  %% cardinality
			  dom    : GFD.'reflect.dom'
			  domList: GFD.'reflect.domList'
			  nextSmaller : GFD.'reflect.nextSmaller'
			  nextLarger : GFD.'reflect.nextLarger'
			  med    : GFD.'reflect.med'   %% median of domain
			  %% distance between maximum and minimum
			  width  : GFD.'reflect.width' 
			  %% regret of domain minimum (distance to next larger value).
			  regret_min : GFD.'reflect.regretMin'
			  %% regret of domain maximum (distance to next smaller value). 
			  regret_max : GFD.'reflect.regretMax'
			  %% number of propagators associated with the variable
			  nbProp: GFD.'reflect.nbProp'
			  %% number of suspendables associated with the variables
			  nbSusps: NBSusps 
			 )
   end
   %% Watching variables
   %% Not tested yet!!
   local
      fun{WatchDomain P}
	 proc{$ D1 D2 B} BTmp in
	    BTmp = {FdInt 0#1}
	    {Wait D2}
	    {P D1 BTmp D2}
	    {Wait BTmp}
	    if BTmp == 1 then B = true else B = false end
	 end
      end
      WatchMin = {WatchDomain GFD.'watch.max'}
      WatchMax = {WatchDomain GFD.'watch.min'}
      WatchSize = {WatchDomain GFD.'watch.size'}
   in
      FdWatch = watch(size: WatchSize 
		      min : WatchMin
		      max : WatchMax)
   end

%%% Direct access to the module
   Prop = GFD
%%%
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% Clasify!!!!%%%%%%%%%%%%%
   Eq = GFD.'eq'
   Rel = GFD.'rel'       
   Linear = GFD.'linear'
   Linear2 = GFD.'linear2'
   LinearR = GFD.'linearR'
   LinearCR = GFD.'linearCR'
   
   Count = GFD.'count'
   Distinct= GFD.'distinct'
   Distinct2 = GFD.'distinct2'
   Mult=   GFD.'mult'
   %%Bool_and = GFD.'bool_and'
   Abs = GFD.'int_Gabs'
   Int_sortedness = GFD.'int_sortedness'
   
   %%Mozart Propagators (backward compatibility))
   
   Int_sumCN =   GFD.int_sumCN
   %Int_disjoint =   GFD.int_disjoint
   %Int_reified_int =   GFD.int_reified_int

   %% Assignment propagators
   Assign = GFD.'assign'

   Int_ext = GFD.'int_ext'
   %% Backward compatibility propagators


   


   
   \insert GeMozProp.oz
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   
   %% Miscelaneus
   FdInf = {GFD.inf}
   FdSup = {GFD.sup}
   FdIs  = GFD.is

   local
      \insert GeIntVarDist
   in
      FdDistribute = IntVarDistribute
   end
   
end

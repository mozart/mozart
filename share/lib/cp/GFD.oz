%%%
%%% Authors:
%%%     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
%%%     Alberto Delgado <adelgado@cic.puj.edu.co>
%%%     Alejandro Arbelaez <aarbelaez@puj.edu.co>
%%%
%%%  Contributors:
%%% 	Andres Felipe Barco (anfelbar@univalle.edu.co)
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
   GFDB at 'x-oz://boot/GFDB'
   GFDP at 'x-oz://boot/GFDP'
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
   
   %% Propagators Builtins
   
   domP: Dom
   relP: RelP
   element: Element
   channel: Channel
   circuit: Circuit
   cumulatives: Cumulatives
   distinctP: DistinctP
   minP: MinP
   maxP: MaxP
   multP: MultP
   linearP: LinearP

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   %% Distribution
   distribute : FdDistribute
   distributeBR : FdDistributeBR
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
   FdInt = GFDB.int
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
	 {FdInt {GFDB.inf}#{GFDB.sup}}
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
	 {GFDB.'reflect.nbProp' X} + {System.nbSusps X}
      end
   in
      FdReflect = reflect(min    : GFDB.'reflect.min'
			  max    : GFDB.'reflect.max'
			  size   : GFDB.'reflect.size'  %% cardinality
			  dom    : GFDB.'reflect.dom'
			  domList: GFDB.'reflect.domList'
			  nextSmaller : GFDB.'reflect.nextSmaller'
			  nextLarger : GFDB.'reflect.nextLarger'
			  med    : GFDB.'reflect.med'   %% median of domain
			  %% distance between maximum and minimum
			  width  : GFDB.'reflect.width' 
			  %% regret of domain minimum (distance to next larger value).
			  regretMin : GFDB.'reflect.regretMin'
			  %% regret of domain maximum (distance to next smaller value). 
			  regretMax : GFDB.'reflect.regretMax'
			  %% number of propagators associated with the variable
			  nbProp: GFDB.'reflect.nbProp'
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
      WatchMin = {WatchDomain GFDP.'watch.max'}
      WatchMax = {WatchDomain GFDP.'watch.min'}
      WatchSize = {WatchDomain GFDP.'watch.size'}
   in
      FdWatch = watch(size: WatchSize 
		      min : WatchMin
		      max : WatchMax)
   end

%%% Direct access to the module
   Prop = GFDP
%%%
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% Clasify!!!!%%%%%%%%%%%%%
   Eq = GFDP.'eq'
   Rel = GFDP.'rel'       
   Linear = GFDP.'linear'
   Linear2 = GFDP.'linear2'
   LinearR = GFDP.'linearR'
   LinearCR = GFDP.'linearCR'
   
   Count = GFDP.'count'
   Distinct= GFDP.'distinct'
   Distinct2 = GFDP.'distinct2'
   Mult=   GFDP.'mult'
   %%Bool_and = GFDP.'bool_and'
   Abs = GFDP.'int_Gabs'
   Int_sortedness = GFDP.'int_sortedness'
   
   %%Mozart Propagators (backward compatibility))
   
   Int_sumCN =   GFDP.int_sumCN
   %Int_disjoint =   GFDP.int_disjoint
   %Int_reified_int =   GFDP.int_reified_int

   %% Assignment propagators
   Assign = GFDP.'assign'

   Int_ext = GFDP.'int_ext'
   %% Backward compatibility propagators

   %% Propagators Builtins
   proc {Dom S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 5 then
	 {GFDP.gfd_dom_5 Sc.1 Sc.2 Sc.3 Sc.cl Sc.pk}
      [] 4 then
	 {GFDP.gfd_dom_4 Sc.1 Sc.2 Sc.cl Sc.pk}
      [] 6 then
	 {GFDP.gfd_dom_6 Sc.1 Sc.2 Sc.3 Sc.4 Sc.cl Sc.pk}
      else
	 raise malformed('Domain constraint post') end
      end
   end

   proc {RelP S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      %% Assert 2 is a GFD.rt.*
      case W
      of 5 then
	 {GFDP.gfd_rel_5 Sc.1 Sc.2 Sc.3 Sc.cl Sc.pk}
      []  6 then
	 {GFDP.gfd_rel_6 Sc.1 Sc.2 Sc.3 Sc.4 Sc.cl Sc.pk}
      [] 4 then
	 {GFDP.gfd_rel_4 Sc.1 Sc.2 Sc.cl Sc.pk}
      else
	 raise malformed('Rel post') end
      end
   end

   proc {Element S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 5 then
	 {GFDP.gfd_element_5 Sc.1 Sc.2 Sc.3 Sc.cl Sc.pk}
      else
	 raise malformed('Element constraint post') end
      end
   end

   proc {Channel S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 4 then
	 {GFDP.gfd_channel_4 Sc.1 Sc.2 Sc.cl Sc.pk}
      [] 5 then
	 % TODO: Sc.3 can be 0 by default.
	 {GFDP.gfd_channel_5 Sc.1 Sc.2 Sc.3 Sc.cl Sc.pk}
      else
	 raise malformed('Channel constraint post') end
      end
   end

   proc {Circuit S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 3 then
	 {GFDP.gfd_circuit_3 Sc.1 Sc.cl Sc.pk}
      else
	 raise malformed('Circuit constraint post') end
      end
   end

   proc {Cumulatives S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 9 then
	 {GFDP.gfd_cumulatives_9 Sc.1 Sc.2 Sc.3 Sc.4 Sc.5 Sc.6 Sc.7 Sc.cl Sc.pk}
      else
	 raise malformed('Cumulatives constraint post') end
      end
   end

/*
   Sorted2 = GFDP.'sorted_2'
   Sorted3 = GFDP.'sorted_3'
   Sorted4 = GFDP.'sorted_4'
   Sorted5 = GFDP.'sorted_5'
   
   Count2 = GFDP.'count_2'
   Count3 = GFDP.'count_3'
   Count4 = GFDP.'count_4'
   Count5 = GFDP.'count_5'
   Count6 = GFDP.'count_6'
   
   Extensional2 = GFDP.'extensional_2'
   Extensional3 = GFDP.'extensional_3'
   Extensional4 = GFDP.'extensional_4'
   Extensional5 = GFDP.'extensional_5'
   
   Mult3 = GFDP.'mult_3'
   Mult5 = GFDP.'mult_5'

   Min2 = GFDP.'min_2'
   Min3 = GFDP.'min_3'
   Min4 = GFDP.'min_4'
   Min5 = GFDP.'min_5'
   
   Abs2 = GFDP.'abs_2'
   Abs4 = GFDP.'abs_4'
   */

   proc {MinP S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 4 then
	 {GFDP.gfd_min_4 Sc.1 Sc.2 Sc.cl Sc.pk}
      [] 5 then
	 {GFDP.gfd_min_5 Sc.1 Sc.2 Sc.3 Sc.cl Sc.pk}
      else
	 raise malformed('Min constraint post') end
      end
   end


   proc {MaxP S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 4 then
	 {GFDP.gfd_max_4 Sc.1 Sc.2 Sc.cl Sc.pk}
      [] 5 then
	 {GFDP.gfd_max_5 Sc.1 Sc.2 Sc.3 Sc.cl Sc.pk}
      else
	 raise malformed('Max constraint post') end
      end
   end

   proc {MultP S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 5 then
	 {GFDP.gfd_mult_5 Sc.1 Sc.2 Sc.3 Sc.cl Sc.pk}
      else
	 raise malformed('Mult constraint post') end
      end
   end

   /*
   proc {Sqr S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 4 then
	 {GFDP.gfd_sqr_4 Sc.1 Sc.2 Sc.cl Sc.pk}
      else
	 raise malformed('Sqr constraint post') end
      end
   end

   proc {Sqrt S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 4 then
	 {GFDP.gfd_sqrt_5 Sc.1 Sc.2 Sc.cl Sc.pk}
      else
	 raise malformed('Sqrt constraint post') end
      end
   end
   */
   
   proc {LinearP S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 5 then
	 {GFDP.gfd_linear_5 Sc.1 Sc.2 Sc.3 Sc.cl Sc.pk}
      [] 6 then
	 {GFDP.gfd_linear_6 Sc.1 Sc.2 Sc.3 Sc.4 Sc.cl Sc.pk}
      [] 7 then
	 {GFDP.gfd_linear_5 Sc.1 Sc.2 Sc.3 Sc.4 Sc.5 Sc.cl Sc.pk}
      else
	 raise malformed('Linear constraint post') end
      end
   end

   proc {DistinctP S}
      Sc =  Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 3 then
	 {GFDP.gfd_distinct_3 Sc.1 Sc.cl Sc.pk}
      [] 4 then
	 {GFDP.gfd_distinct_4 Sc.1 Sc.2 Sc.cl Sc.pk}
      else
	 raise malformed('Distinct constraint post') end
      end
   end
  
   
   \insert GeMozProp.oz
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   
   %% Miscelaneus
   FdInf = {GFDB.inf}
   FdSup = {GFDB.sup}
   FdIs  = GFDB.is

   local
      \insert GeIntVarDist
   in
      FdDistribute = IntVarDistribute
   end
   local
      \insert GeIntVarDistBR
   in
      FdDistributeBR = GFDDistribute
   end
end
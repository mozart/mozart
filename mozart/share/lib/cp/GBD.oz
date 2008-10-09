%%%
%%% Authors:
%%%     Alberto Delgado <adelgado@cic.puj.edu.co>
%%%
%%%  Contributors:
%%% 	Andres Felipe Barco <anfelbar@univalle.edu.co>
%%%
%%% Copyright:
%%%     Alberto Delgado, 2006
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


%%%
%%% Boolean variables have an independent implementation from integer
%%% variables in gecode 2.0. That version of gecode has not been released
%%% so far but we made both integers and bollen veriables independent in mozart
%%% from now. To distribute these variables use GFD.distribute.

functor
require
      CpSupport(vectorToType:   VectorToType
	     vectorToList:   VectorToList
	     vectorToTuple:  VectorToTuple)

import
   
   GBDB at 'x-oz://boot/GBDB'
   GBDP at 'x-oz://boot/GBDP'
   System
   Space

prepare
   Cl = '#'(
	   val: 0   %Value consistency
	   bnd: 1   %Bounds consistency
	   dom: 2   %Domain consistency
	   def: 3   %The default consistency for a constraint
	   )

   %%This record must reflect the IntRelType in gecode/int.hh
   Rt = '#'('=:':0     %Equality
	    '\\=:':1    %Disequality
	    '=<:':2    %Less or equal
	    '<:':3     %Less
	    '>=:':4    %Greater or equal
	    '>:':5     %Greater
	   )
   
   %% Bool Operator Type
   BOT = '#'('and': 0 'or':1 'imp':2 'eqv':3 'xor':4)
   
   %% Propagator kind
   Pk = '#'(
     def:   0  %Make a default decision
     speed: 1  %Prefer speed over memory consumption
     memory:2  %Prefer little memory over speed
     )

   %% Distribition strategie selection

   GbdVarSel = map( naive:     0
		    min:       1
                    max:       2
		  )
   
   GbdValSel = map( min:        0
		    max:        1
                  )
   
   
export
   %% Telling domains
   decl:   Decl
   dom:    GBDDom
   list:   BoolVarList
   tuple:  BoolVarTuple
	 bool: BoolVar

   %% Testing
   is:    IsVar
   
   %%Access
   Reflect
   'reflect.size':       Size
   'reflect.zero':       Zero
   'reflect.one':        One

   %% Consistency levels for propagators
   cl:    Cl

   %% Integer relation types
   rt:    Rt

   %% Boolean operations
   bo:    BOT
   
   %% Miscellaneous
 

   %% propagators

   %% Variable Variable Boolean relation propagator
   rel_BV_BT_BV_BV:  VarRel

   %% boolean relations
   and: And
   'or': Or
   xor: Xor
   imp: Imp
   eqv: Eqv

  %% Propagators Builtins
  
  relP:     RelP
  linearP:  LinearP

   %% some aliases
   disj:    Or
   conj:    And
   'not':   Bool_not

   
%    conjA      :Bool_and_arr
%    disjA      :Bool_or_arr
%    rel        :Rel
%    linear     :Linear
   %Watch
   
   %%Branching
   %ValSel VarSel
   %%%distribute:     IntVarDistribute
   %Distribute
   distribute: GBDDistribute   

   %%Space
   
define
   
   
   %% operations
   BoolVar = GBDB.'bool'
   IsVar = GBDB.'isVar'
   Size = GBDB.'reflect.size'
   Zero = GBDB.'reflect.zero'
   One = GBDB.'reflect.one'
   Bool_not = GBDP.'not'
   VarRel = GBDP.'rel_BV_BT_BV_BV'

   Reflect = reflect(size: Size
		     zero: Zero
		     one:  One
		     dom:_		     
		    )		     
   
   %%This record must reflect the ConLevel

   
   fun{Decl}
      {BoolVar 0#1}
   end

   local
      proc{CreateDomainTuple N Dom Root}
	 if N > 0 then
	    Root.N = {BoolVar Dom} {CreateDomainTuple N-1 Dom Root} end
      end

      proc{CreateDomainRecord As Dom Root}
	 case As of nil then skip
	 [] A | Ar then
	    Root.A = {BoolVar Dom} {CreateDomainRecord Ar Dom Root}
	 end
      end
   
      proc{CreateDomainList Dom Root}
	 case Root of
	    X|Xs then
	    X = {BoolVar Dom}
	    {CreateDomainList Dom Xs}
	 [] nil then skip
	 end
      end
   in

      proc{GBDDom Dom Root}
	 case {VectorToType Root}
	 of list then   {CreateDomainList Dom Root}
	 [] tuple then  {CreateDomainTuple {Width Root} Dom Root}
	 [] record then {CreateDomainRecord {Arity Root} Dom Root}
	 end
      end
   end
   
   %%Declares a list of GBD vars
   fun{BoolVarList N Desc}
      Lst = {List.make N}
   in
      {List.map Lst fun{$ X} X = {BoolVar Desc} end}
   end

   proc{BoolVarTuple T N Desc Seq}
      Seq = {Tuple.make T N}
      for I in 1..N do
	 Seq.I = {BoolVar Desc}
      end
   end


   %% Bolean relations (propagators)
   proc {And X Y Z}
      {VarRel X BOT.and Y Z}
   end

   proc {Or X Y Z}
      {VarRel X BOT.'or' Y Z}
   end

   proc {Xor X Y Z}
      {VarRel X BOT.xor Y Z}
   end

   proc {Imp X Y Z}
      {VarRel X BOT.imp Y Z}
   end

   proc {Eqv X Y Z}
      {VarRel X BOT.eqv Y Z}
   end


  %%Propagators Builtins
  
   proc {RelP S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 5 then
   {GBDP.gbd_rel_5 Sc.1 Sc.2 Sc.3 Sc.cl Sc.pk}
      []  6 then
   {GBDP.gbd_rel_6 Sc.1 Sc.2 Sc.3 Sc.4 Sc.cl Sc.pk}
      []  4 then
   {GBDP.gbd_rel_4 Sc.1 Sc.2 Sc.cl Sc.pk}
      else
   raise malformed('Rel constraint post') end
      end
   end
  
   proc {LinearP S}
      Sc = {Adjoin '#'(cl:Cl.def pk:Pk.def) S}
      W = {Record.width Sc}
   in
      case W
      of 5 then
   {GBDP.gbd_linear_5 Sc.1 Sc.2 Sc.3 Sc.cl Sc.pk}
      []  6 then
   {GBDP.gbd_linear_6 Sc.1 Sc.2 Sc.3 Sc.4 Sc.cl Sc.pk}
      [] 7 then
   {GBDP.gbd_linear_7 Sc.1 Sc.2 Sc.3 Sc.4 Sc.5 Sc.cl Sc.pk}
      else
   raise malformed('Linear constraint post') end
      end
   end

   %% Bool Var distribution
   proc {GBDDistribute Spec Vs}
      case {Label Spec}
      of generic then
	 Order = Spec.order
	 Value = Spec.value
      in
	 {Wait {GBDB.'distribute' GbdVarSel.Value GbdValSel.Value Vs}}
      else
	 {Exception.raiseError
	  fd(unknownDistributionStrategy
	     'BD.distribute' [Spec Vs] 1)}
      end 
   end 
   
end

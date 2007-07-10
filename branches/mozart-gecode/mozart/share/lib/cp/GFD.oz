/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     Alejandro Arbelaez <aarbelaez@puj.edu.co>
 *
 *  Copyright:
 *     Gustavo Gutierrez, 2006
 *     Alberto Delgado, 2006
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of GeOz, a module for integrating gecode 
 *  constraint system to Mozart: 
 *     http://home.gna.org/geoz
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

functor
import
   GFD(int: IntVar
       isVar: IsVar
       'reflect.min':        GetMin
       'reflect.max':        GetMax
       'reflect.size':       GetSize
       'reflect.med':        GetMed
       'reflect.width':      GetWidth
       'reflect.regretMin':  GetRegretMin
       'reflect.regretMax':  GetRegretMax

       eq:                   Eq
       rel:                  IntRel       
       linear:               Linear
       linear2:              Linear2
       linearR:              LinearR
       linearCR:             LinearCR
       count:                Count
       distinct:             Distinct
       distinct2:            Distinct2
       mult:                 Mult
       bool_and: 	     Bool_and
       int_Gabs:             Abs       
       int_sortedness:       Int_sortedness

       %%Mozart Propagators
       int_inf:              Int_inf
       int_sup:              Int_sup
       int_watch_min:        Int_watch_min
       int_watch_max:        Int_watch_max
       int_watch_size:       Int_watch_size
       int_nextLarger:       Int_nextLarger
       int_nextSmaller:      Int_nextSmaller
       int_domlist:          Int_domlist
       int_dom:              Int_dom
       int_sumCN:            Int_sumCN
       int_disjoint:         Int_disjoint
       int_reified_int:      Int_reified_int
       bool_Gand:            Bool_Gand
       bool_Gor:             Bool_Gor
       bool_Gxor:            Bool_Gxor
       bool_Gnot:            Bool_Gnot
       bool_Gimp:            Bool_Gimp
       bool_Geqv:            Bool_Geqv
       
       status:  Status
      )
   at 'x-oz://boot/geoz-int'

   System
   Space
export
   %%Declaration
   int:   IntVar
   %%   dom:   IntVarList
   dom:   GFDDom

   dom1:    CreateDomainList
   
%    IntVarTuple
    IntVarList

   is:    IsVar
   
   %%Access
   Reflect

   %% Relation types for propagators
   Rt
   %% Consistency levels for propagators
   Cl

   %% Miscellaneous
   inf:             Inf
   sup:             Sup
   decl:            Decl

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
   disjoint:        Disjoint 
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

   
   
   %%Propagators
   eq:     Eq
   rel:    IntRel
   Linear
   Linear2
   Count
%%   Distinct
%%   Distinct2   
   Mult
   Bool_and
   Abs
   sortedness: Int_sortedness


   %% 0/1 propagators
   Conj
   Disj
   Xor
   Not
   Imp
   Equi   

   Watch
   
   %%Branching
   %ValSel VarSel
   %%%distribute:     IntVarDistribute
   Distribute
   %intBranch:Distribute   

   %%Space
   Status

   tuple: GTuple

define

   Reflect = reflect(min:  GetMin
		     max:  GetMax
		     size: GetSize
		     med:   GetMed
		     width: GetWidth
		     regret_min: GetRegretMin
		     regret_max: GetRegretMax
		     nextLarger:_
		     nextSmaller:_
		     domList:_
		     dom:_		     
		    )		     
   
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

   Reified = reified(sum:_ sumC:_ sumCN:_ int:_)

   %%misc defines: VectorToTuple, VectorToType, VectorToList ....
   \insert misc
   \insert GeMozProp

   proc{CreateDomainTuple N Dom Root}
      if N > 0 then Root.N = {IntVar Dom} {CreateDomainTuple N-1 Dom Root} end
   end

   proc{CreateDomainRecord As Dom Root}
      case As of nil then skip
      [] A | Ar then Root.A = {IntVar Dom} {CreateDomainRecord Ar Dom Root}
      end
   end
   
   %%Create GeIntVar with domain Dom per each element of the list
   proc{CreateDomainList Dom Root}
      case Root of
	 X|Xs then
	 X = {IntVar Dom}
	 {CreateDomainList Dom Xs}
      [] nil then skip
      end
   end   


   proc{GFDDom Dom Root}
      case {VectorToType Root}
      of list then   {CreateDomainList Dom Root}
      [] tuple then  {CreateDomainTuple {Width Root} Dom Root}
      [] record then {CreateDomainRecord {Arity Root} Dom Root}
      end
   end
   
   %%Declares a list of GFD vars
   fun{IntVarList N Desc}
      Lst = {List.make N}
   in
      {List.map Lst fun{$ X} X = {IntVar Desc} end}
   end

   proc{GTuple T N Desc Seq}
      Seq = {Tuple.make T N}
      for I in 1..N do
	 Seq.I = {IntVar Desc}
      end
   end
    %%Declares a list of GFD vars
%     fun{IntVarTuple S N Desc}
%        Tpl = {Tuple.make '#' N}
%     in
%        {Record.map Tpl fun{$ X} X = {IntVar S Desc} end}
%     end

   local
      \insert GeIntVarDist
   in
      Distribute = IntVarDistribute
   end

end

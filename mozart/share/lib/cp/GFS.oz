%%%
%%% Authors:
%%%     Alberto Delgado <adelgado@cic.puj.edu.co>
%%%
%%% Copyright:
%%%     Alberto Delgado, 2007
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
	     %vectorMap:      VectorMap
	     %expand:         Expand
	     %formatOrigin:   FormatOrigin
	    )
   
import
   GFS at 'x-oz://boot/geoz-set'
   GFB at 'x-oz://boot/geoz-bool'
   Space

export
   %% Telling domains
   var  : Var
   is   : IsVar
   sup  : Sup
   inf  : Inf
   compl : Compl
   isIn  :  IsIn
   complIn : ComplIn
   include : Include
   exclude : Exclude
   carVal  : CardVal
   carInt  : CardInt
   diff    : Diff 
   intersect  :  Inter
   intersectN :  InterN
   union   : Union
   unionN  : UnionN
   subset  : Subset
   disjoint   : Disj
   disjointN  : DisjointN
   distinct   : Dist
   distinctN  : DistinctN
   partition  : Partition
   

define
   %FsDecl = GFS.set
   IsVar = GFS.isVar
   Sup = {GFS.sup}
   Inf = {GFS.inf}
   Var
   Compl = GFS.comp
   ComplIn = GFS.complIn
   Include = GFS.incVal
   Exclude = GFS.excVal
   CardVal = GFS.cardVal
   CardInt = GFS.cardInt
   IsIn
   %% Propagators
   Diff = GFS.diff
   Inter = GFS.intersect
   InterN = GFS.intersectN
   Union = GFS.union
   UnionN = GFS.unionN
   Subset = GFS.subset
   Disj = GFS.disjoint
   Dist = GFS.distinct
   DisjointN
   DistinctN
   Partition
   

in
   local
      fun {Decl}
	 {GFS.bounds nil Inf#Sup}
      end
      fun {Upper Val}
	 {GFS.bounds nil Val}
      end
      
      fun {Lower Val}
	 {GFS.bounds Val Inf#Sup}
      end

      proc {TupleDom N T Fun Dom Dom2}
	 if Dom == nil then
	    if N>0 then {Fun T.N} {TupleDom N-1 T Fun Dom Dom2} end
	 else
	    if Dom2==nil then
	       if N>0 then {Fun Dom T.N} {TupleDom N-1 T Fun Dom Dom2} end
	    else
	       if N>0 then {Fun Dom Dom2 T.N} {TupleDom N-1 T Fun Dom Dom2} end
	    end
	 end
      end
      
      proc {RecordDom As R Fun Dom Dom2}
	 if Dom==nil then
	    if Dom2==nil then
	       case As of nil then skip
	       [] A|Ar then {Fun Dom R.A} {RecordDom Ar R Fun Dom Dom2}
	       end
	    else
	       case As of nil then skip
	       [] A|Ar then {Fun Dom Dom2 R.A} {RecordDom Ar R Fun Dom Dom2}
	       end
	    end
	 end
      end

      
      fun {FsList N Fun Dom}
	 if Dom == nil then
	    if N>0 then {Fun}|{FsList N-1 Fun Dom}
	    else nil
	    end
	 else
	    if N>0 then {Fun Dom}|{FsList N-1 Fun Dom}
	    else nil
	    end
	 end
      end
      
      
      proc {FsTuple L N Fun Dom Dom2 ?T}
	 T={MakeTuple L N} {TupleDom N T Fun Dom Dom2}
      end

      proc {FsRecord L As Fun Dom Dom2 ?R}
	 R={MakeRecord L As} {RecordDom As R Fun Dom Dom2}
      end
      
      
      fun {FsListDecl N}
	 {FsList N Decl nil}
      end
      
      fun {FsListUpperBound N Dom}
	 {FsList N Upper Dom}
      end

      fun {FsListLowerBound N Dom}
	 {FsList N Lower Dom}
      end

      fun {FsListBounds N Dom1 Dom2}
	 if N>0 then {GFS.bounds Dom1 Dom2}|{FsListBounds N-1 Dom1 Dom2}
	 else nil
	 end
      end

      fun {FsTupleDecl L N}
	 {FsTuple L N Decl nil nil}
      end

      fun {FsTupleUpperBound L N Dom}
	 {FsTuple L N Upper Dom nil}
      end

      fun {FsTupleLowerBound L N Dom}
	 {FsTuple L N Lower Dom nil}
      end

      fun {FsTupleBounds L N Dom1 Dom2}
	 {FsTuple L N GFS.bounds Dom1 Dom2}
      end
      

      fun {FsRecordDecl L As}
	 {FsRecord L As Decl nil nil}
      end

      fun {FsRecordUpperBound L As Dom}
	 {FsRecord L As Upper Dom nil}
      end

      fun {FsRecordLowerBound L As Dom}
	 {FsRecord L As Lower Dom nil}
      end

      fun {FsRecordBounds L As Dom1 Dom2}
	 {FsRecord L As GFS.bounds Dom1 Dom2}
      end
      
     
      
      
   in     
      Var = var(decl : Decl
		bounds : GFS.bounds
		upperBound : Upper
		lowerBound : Lower
		list(decl : FsListDecl
		     bounds : FsListBounds
		     upperBound : FsListUpperBound
		     lowerBound : FsListLowerBound
		    )
		tuple(decl: FsTupleDecl
		      bounds : FsTupleBounds
		      upperBound : FsTupleUpperBound
		      lowerBound : FsTupleLowerBound
		     )
		record(decl: FsRecordDecl
		      bounds : FsRecordBounds
		      upperBound : FsRecordUpperBound
		      lowerBound : FsRecordLowerBound
		      )		
	       )

      proc {DisjointN Mv}
	 for I in 1..{List.lenght Mv} do
	    for J in I.. {List.lenght Mv} do
	    {GFS.disjoint {List.nth Mv I} {List.nth Mv I}}
	    end
	 end
      end
      
      proc {DistinctN Mv}
	 for I in 1..{List.lenght Mv} do
	    for J in I.. {List.lenght Mv} do
	    {GFS.distinct {List.nth Mv I} {List.nth Mv I}}
	    end
	 end
      end
      
      proc {Partition Mv M}
	 {DistinctN Mv}
	 {UnionN Mv M}
      end

      fun {IsIn Var Val}
	 Bool = {GFB.init}
	 {GFB.isIn Var Val Bool}
	 Var
      end      
      
   end   
   
end

%%%
%%% Authors:
%%%     Alberto Delgado <adelgado@cic.puj.edu.co>
%%%
%%%  Contributors:
%%%     Andres Felipe Barco <anfelbar@univalle.edu.co>
%%%     Gustavo A. Gomez Farhat <gafarhat@univalle.edu.co>
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
	     expand:         ExpandList
	     %formatOrigin:   FormatOrigin
	    )
   
import
   GFSB at 'x-oz://boot/GFSB'
   GFSP at 'x-oz://boot/GFSP'
   GFB at 'x-oz://boot/GBD'
   Space
   GFD

prepare
   %%This record must reflect the SetRelType in gecode/set.hh
   Rt = '#'('=:'    :0    %Equality
	    '\\=:'  :1    %Disequality
	    '<:'    :2    %SubSet
	    '>:'    :3    %SuperSet
	    '||:'   :4    %Disjoint
	    '^:'    :5    %Complement
	   )
   
   %%This record must reflect the SetOpType in gecode/set.hh
   SOP = '#'(unionOP       :0    % Union.
	     disjointOP    :1    % Disjoint union.
	     intersectOP   :2    % Intersection
	     differenceOP  :3    % Difference.
	    )

export
%%% Classify as in the Mozart docs
   % Finite Set Interval Variables
   var : Var
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
   reified: GFSReified

   % Finite Set Intervals
   inf  : Inf
   sup  : Sup
   compl : Compl
   complIn : ComplIn
   include : Include
   exclude : Exclude
   card: Card
   cardRange: CardRange
   isIn  :  IsIn

   % Finite Set Constants
   value: GFSValue   
   
   %Gecode propagators
   sequence: Sequence
   sequentialUnion : SequentialUnion
   atMostOne: AtMostOne
   dom: Dom
   
   min: Min
   max: Max
   match: Match
   channel: Channel
   weights: Weights 
   selectUnion: SelectUnion
   selectInter: SelectInter
   selectInterIn: SelectInterIn
   selectDisjoint: SelectDisjoint
   selectSet: SelectSet
   %rel: Rel
   
   %% Set relation types
   rt:    Rt

   %% Set operations
   so:    SOP
   
define
   %FsDecl = GFS.set
   GFSisVar = GFSB.isVar
   Sup = {GFSB.sup}
   Inf = {GFSB.inf}
   Var
   Compl = GFSB.comp
   ComplIn = GFSB.complIn
   Include = GFSB.incVal
   Exclude = GFSB.excVal
   %CardVal = GFSB.cardVal
   %CardInt = GFSB.cardInt
   IsIn
   Card
   CardRange
   %% Propagators
   Diff = GFSP.diff
   Inter = GFSP.intersect
   InterN = GFSP.intersectN
   Union = GFSP.union
   UnionN = GFSP.unionN
   Subset = GFSP.subset
   Disj = GFSP.disjoint
   Dist = GFSP.distinct
   DisjointN
   DistinctN
   Partition
   GFSReified
   ReifiedInclude
   GFSValue
   GFSValueMake = GFSB.'gfs_valueMake_2'

   %% Gecode propagators
   Rel
   Sequence
   SequentialUnion
   
   AtMostOne
   Dom
   Min
   Max
   Match
   Channel
   Weights

   SelectUnion
   SelectInter
   SelectInterIn
   SelectDisjoint
   SelectSet
   
in
   local
      fun {Decl}
	 {GFSB.bounds nil Inf#Sup}
      end
      fun {Upper Val}
	 {GFSB.bounds nil Val}
      end
      
      fun {Lower Val}
	 {GFSB.bounds Val Inf#Sup}
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
	 if N>0 then {GFSB.bounds Dom1 Dom2}|{FsListBounds N-1 Dom1 Dom2}
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
	 {FsTuple L N GFSB.bounds Dom1 Dom2}
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
	 {FsRecord L As GFSB.bounds Dom1 Dom2}
      end
      
      
      
      
   in     
      Var = var(
	       is: GFSisVar 
	       decl : Decl
	       bounds : GFSB.bounds
	       upperBound : Upper
	       lowerBound : Lower
	       list: '#'(decl : FsListDecl
			 bounds : FsListBounds
			 upperBound : FsListUpperBound
			 lowerBound : FsListLowerBound
			)
	       tuple: '#'(decl: FsTupleDecl
			  bounds : FsTupleBounds
			  upperBound : FsTupleUpperBound
			  lowerBound : FsTupleLowerBound
			 )
	       record: '#'(decl: FsRecordDecl
			   bounds : FsRecordBounds
			   upperBound : FsRecordUpperBound
			   lowerBound : FsRecordLowerBound
			  )		
	       )

      GFSValue = value(
		    % empty:
% 		       {FSSetValue nil}
% 		    universal:
% 		       {FSSetValue FSUniversalRefl}
% 		    singl:
% 		       fun {$ N} {FSSetValue [N]} end
		    make:
		       GFSValueMake
		    % is:
% 		       FSisValue
% 		    toString:
% 		       FSvalueToString
		    )

      GFSReified = reified(
		     % isIn:
% 			     FSIsInReif
% 			  areIn:
% 			     proc {$ W S BList}
% 				WList = {ExpandList
% 					 {FSGetGlb {FSB.'value.make' W}}}
% 			     in
% 				BList
% 				= {FD.list {Length WList} 0#1}
% 				= {Map WList fun {$ E} {FSIsInReif E S} end}
% 			     end
		      include:
			 ReifiedInclude
			  % bounds:
% 			     FSP.'reified.bounds'
% 			  boundsN:
% 			     FSP.'reified.boundsN'
% 			  partition:
% 			     proc {$ SVs Is GSet Rs}
% 				Rs = {Map Is fun {$ I} {FD.int [0 I]} end}
% 				{FSP.'reified.partition' SVs GSet Rs}
% 			     end
% 			  equal:
% 			     FSEqualReif
		      )
      
      
      proc{Dom Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 3 then
	    {GFSP.gfs_dom_3 Sc.1 Sc.2 Sc.3}
	 []  4 then
	    {GFSP.gfs_dom_4 Sc.1 Sc.2 Sc.3 Sc.4}
	 []  5 then
	    {GFSP.gfs_dom_5 Sc.1 Sc.2 Sc.3 Sc.4 Sc.5}
	 else
	    raise malformed('Dom constraint post') end
	 end
      end
      
      proc {DisjointN Mv}
	 for I in 1..{List.lenght Mv} do
	    for J in I.. {List.lenght Mv} do
	       {GFSP.disjoint {List.nth Mv I} {List.nth Mv I}}
	    end
	 end
      end
      
      proc {DistinctN Mv}
	 for I in 1..{List.lenght Mv} do
	    for J in I.. {List.lenght Mv} do
	       {GFSP.distinct {List.nth Mv I} {List.nth Mv I}}
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

      proc{Rel Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 3 then
	    {GFSP.gfs_rel_3 Sc.1 Sc.2 Sc.3}
	 []  4 then
	    {GFSP.gfs_rel_4 Sc.1 Sc.2 Sc.3 Sc.4}
	 []  5 then
	    {GFSP.gfs_rel_5 Sc.1 Sc.2 Sc.3 Sc.4 Sc.5}
	 else
	    raise malformed('Rel constraint post') end
	 end
      end
      
      proc{Sequence Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 1 then
	    {GFSP.gfs_sequence_1 Sc.1}
	 else
	    raise malformed('Sequence constraint post') end
	 end
      end

      proc{SequentialUnion Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 2 then
	    {GFSP.gfs_sequentialUnion_2 Sc.1 Sc.2}
	 else
	    raise malformed('SequentialUnion constraint post') end
	 end
      end

      proc{AtMostOne Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 2 then
	    {GFSP.gfs_atMostOne_2 Sc.1 Sc.2}
	 else
	    raise malformed('AtMostOne constraint post') end
	 end
      end

      proc{Min Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 2 then
	    {GFSP.gfs_min_2 Sc.1 Sc.2}
	 else
	    raise malformed('Min constraint post') end
	 end
      end

      proc{Max Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 2 then
	    {GFSP.gfs_max_2 Sc.1 Sc.2}
	 else
	    raise malformed('Max constraint post') end
	 end
      end

      proc{Match Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 2 then
	    {GFSP.gfs_match_2 Sc.1 Sc.2}
	 else
	    raise malformed('Match constraint post') end
	 end
      end

      proc{Channel Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 2 then
	    {GFSP.gfs_channel_2 Sc.1 Sc.2}
	 else
	    raise malformed('Channel constraint post') end
	 end
      end
      
      proc{Card Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 2 then
	    {GFSP.gfs_cardinality_2 Sc.1 Sc.2}
	 else
	    raise malformed('Card constraint post') end
	 end
      end
      
      proc{CardRange Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 3 then
	    {GFSP.gfs_cardinality_3 Sc.1 Sc.2 Sc.3}
	 else
	    raise malformed('CardRange constraint post') end
	 end
      end  
      
      proc{Weights Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 4 then
	    {GFSP.gfs_weights_4 Sc.1 Sc.2 Sc.3 Sc.4}
	 else
	    raise malformed('Weights constraint post') end
	 end
      end
      
      proc{SelectUnion Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 3 then
	    {GFSP.gfs_selectUnion_3 Sc.1 Sc.2 Sc.3}
	 else
	    raise malformed('SelectUnion constraint post') end
	 end
      end

      proc{SelectInter Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 3 then
	    {GFSP.gfs_selectInter_3 Sc.1 Sc.2 Sc.3}
	 else
	    raise malformed('SelectInter constraint post') end
	 end
      end

      proc{SelectInterIn Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 4 then
	    {GFSP.gfs_selectInterIn_4 Sc.1 Sc.2 Sc.3 Sc.4}
	 else
	    raise malformed('SelectInterIn constraint post') end
	 end
      end

      proc{SelectDisjoint Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 2 then
	    {GFSP.gfs_selectDisjoint_2 Sc.1 Sc.2}
	 else
	    raise malformed('SelectDisjoint constraint post') end
	 end
      end

      proc{SelectSet Sc}
	 W = {Record.width Sc}
      in
	 case W
	 of 3 then
	    {GFSP.gfs_selectSet_3 Sc.1 Sc.2 Sc.3}
	 else
	    raise malformed('SelectSet constraint post') end
	 end
      end

      proc{ReifiedInclude Sc B}
	 W = {Record.width Sc}
      in
	 case W
	 of 2 then
	    {GFSP.gfs_reifiedInclude_3 Sc.1 Sc.2 B}
	 else
	    raise malformed('ReifiedInclude constraint post') end
	 end
      end

      
   end   

%    local
%       \insert GeSetVarDist
%    in
%       FsDistribute = SetVarDistribute
%    end
   
   % local
%       \insert GeSetVarDistBR
%    in
%       FsDistributeBR = GFSDistribute
%    end
   
end

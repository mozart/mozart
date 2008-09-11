%%%
%%% Authors:
%%%     Gustavo A. Gomez Farhat <gafarhat@univalle.edu.co>
%%%
%%% Copyright:
%%%     Gustavo A. Gomez Farhat, 2008
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

%%
%% Distribution
%%

%% GeIntVar Distribution. var selection and val selection                                                                                      
   GfsVarSel = map( naive:         0
                    minCard:       1
                    maxCard:       2
                    minUnknown:    3
                    maxUnknown:    4
		  )

   GfsValSel = map( min:        0
                    max:        1
                  )

local
   fun {GetFeaturePath Rec Spec Path}
      case Path of FD|T then
	 F#D = FD
	 PP  = if {HasFeature Spec F} then Rec.(Spec.F)
	       else Rec.D
	       end
      in
	 if T==nil then PP else {GetFeaturePath PP Spec T} end
      else found_nil_in_path
      end
   end
   
   fun {Find X|Xr C}
      {FoldL Xr fun {$ I E}
		   if {C I E} then I else E end
		end X}
   end

   fun {MinElement Y|_}
      case Y
      of L#_ then L
      else Y
      end
   end

   fun {MaxElement Ys}
      case {List.last Ys}
      of _#R then R
      [] Y   then Y
      end
   end

   MINELEM = {NewName}
   MAXELEM = {NewName}

   fun {LESS X Y}
      case X#Y
      of !MINELEM#!MINELEM then false
      [] !MINELEM#_        then true
      []        _#!MINELEM then false
      [] !MAXELEM#!MAXELEM then false
      [] !MAXELEM#_        then false
      []        _#!MAXELEM then true
      else X =< Y
      end
   end

   fun {GREATER X Y}
      case X#Y
      of !MINELEM#!MINELEM then false
      [] !MINELEM#_        then false
      []        _#!MINELEM then true
      [] !MAXELEM#!MAXELEM then false
      [] !MAXELEM#_        then true
      []        _#!MAXELEM then false
      else X > Y
      end
   end

   fun {WeightMin DF}
      fun {$ CL WT}
	 if CL == nil then DF
	 else {Find {ExpandList CL} fun {$ X Y} {WT X} < {WT Y} end}
	 end
      end
   end

   fun {WeightMax DF}
      fun {$ CL WT}
	 if CL == nil then DF
	 else {Find {ExpandList CL} fun {$ X Y} {WT X} > {WT Y} end}
	 end
      end
   end

   fun {WeightSum CL WT}
      %{FD.sum {Map {ExpandList CL} fun {$ X} {WT X} end} '=:'} = {FD.decl}
      {GFD.sum post({Map {ExpandList CL} fun {$ X} {WT X} end} GFD.rt.'=:')} = {GFD.decl}
   end

   fun {OrderFun Spec Select WT}
      CardTable =
      c(unknown:
	   fun {$ S} {GFSGetNumOfUnknown {Select S}} end
	lowerBound:
	   fun {$ S} {GFSGetNumOfGlb {Select S}} end
	upperBound:
	   fun {$ S} {GFSGetNumOfLub {Select S}} end)
      
      fun {MakeCompTableWeight F}
	 c(unknown:
	      fun {$ S} {F {GFSGetUnknown {Select S}} WT} end
	   lowerBound:
	      fun {$ S} {F {GFSGetGlb     {Select S}} WT} end
	   upperBound:
	      fun {$ S} {F {GFSGetLub     {Select S}} WT} end)
      end

      OrderFunTable =
      s(min: c(card:
		  CardTable
	       weightMin:
		  {MakeCompTableWeight {WeightMin MAXELEM}}
	       weightMax:
		  {MakeCompTableWeight {WeightMax MAXELEM}}
	       weightSum:
		  {MakeCompTableWeight WeightSum})
	max: c(card:
		  CardTable
	       weightMin:
		  {MakeCompTableWeight {WeightMin MINELEM}}
	       weightMax:
		  {MakeCompTableWeight {WeightMax MINELEM}}
	       weightSum:
		  {MakeCompTableWeight WeightSum})
       )
      
      OrderFunTableRel = s(min: LESS max: GREATER)
      
   in
      if {IsProcedure Spec} then Spec
      else
	 if Spec == naive then fun {$ L} L end
	 else
	    OrderFunRel = {GetFeaturePath OrderFunTableRel Spec [sel#min]}
	    
	    OrderFun = {GetFeaturePath OrderFunTable Spec
			[sel#min cost#card comp#unknown]}
	 in
	    fun {$ L}
	       {Sort L fun {$ X Y}
			  {OrderFunRel {OrderFun X} {OrderFun Y}}
		       end}
	    end
	 end
      end
   end
   
   fun {ElementFun Spec Select WT}
      ElementFunTable =
      v(min: v(unknown:
		  fun {$ S}
		     {MinElement {GFSReflect.unknown {Select S}}}
		  end
	       weight:
		  fun {$ S}
		     {{WeightMin error}
		      {GFSReflect.unknown {Select S}} WT}
		  end)
	max: v(unknown:
		  fun {$ S}
		     {MaxElement {GFSReflect.unknown {Select S}}}
		  end
	       weight:
		  fun {$ S}
		     {{WeightMax error}
		      {GFSReflect.unknown {Select S}} WT}
		  end)
       )
   in
      if {IsProcedure Spec} then Spec
      else {GetFeaturePath ElementFunTable Spec [sel#min wrt#unknown]}
      end
   end 

   fun {FilterFun Spec Select}
      case Spec
      of true then
	 fun {$ X} {GFSGetNumOfUnknown {Select X}} > 0 end
      else
	 fun {$ X} Y = {Select X} in
	    {GFSGetNumOfUnknown Y} > 0 andthen  {Spec Y}
	 end
      end 
   end

   fun {SelectFun Spec}
      case Spec
      of id then fun {$ X} X end
      else Spec
      end
   end

   fun {RRobinFun Spec}
      if Spec then fun {$ H|T} {Append T [H]} end
      else fun {$ L} L end
      end
   end

   B
   B = {GBD.decl}

   proc {FSDistNaive Xs}
      case Xs of nil then skip
      [] X|Xr then
	 {Space.waitStable}
	 Unknown={GFSReflect.unknown X}
      in
	 if Unknown==nil then
	    {FSDistNaive Xr}
	 else
	    UnknownVal = {MinElement Unknown}
	    {ReifiedInclude post(UnknownVal X B)}
	 in
	    {GFD.distribute generic(value:max) [B]}
	    /*
	    choice {FSIsIncl UnknownVal X}
	    []     {FSIsExcl UnknownVal X}
	    end
	    */
	    {FSDistNaive Xs}
	 end
      end
   end 

   proc {FSDistGeneric SL Order FCond Elem RRobin Sel Proc}
      {Space.waitStable}
      if Proc\=unit then
	 {Proc}
	 {Space.waitStable}
      end
      local
	 FilteredSL = {Filter SL FCond}
	 %% it is unnecessary to compute the sorted list
	 %% we just need to pick the right variable.
	 %% this needs to be fixed eventually.
	 SortedSL   = {Order FilteredSL}
      in
	 case SortedSL
	 of nil then skip
	 [] HSL|_ then
	    UnknownVal={Elem HSL}
	    DistVar   ={Sel  HSL}
	    {ReifiedInclude post(UnknownVal DistVar B)}
	 in
	    {GFD.distribute generic(value:max) [B]}
	    /*
	    choice {FSIsIncl UnknownVal DistVar}
	    []     {FSIsExcl UnknownVal DistVar}
	    end
	    */
	    {FSDistGeneric {RRobin FilteredSL}
	     Order FCond Elem RRobin Sel Proc}
	 end 
      end
   end 
in
   proc {GFSDistribute K Vs}
      L={VectorToList Vs}
   in
      case K
      of naive then {FSDistNaive L}
      else
	 case {Label K}
	 of opt then
	    Order = K.order
	    Value = K.value
	 in
	    {Wait {GFSP.distribute GfsVarSel.Order GfsValSel.Value L}}
	 [] generic then
	    Select  = {SelectFun {CondSelect K select id}}
	    Weights = {CondSelect K weights {GFSMakeWeights nil}}
	    Order   = {OrderFun {CondSelect K order order} Select Weights}
	    Filter  = {FilterFun {CondSelect K filter true} Select}
	    Element = {ElementFun {CondSelect K element element}
		       Select Weights}
	    RRobin  = {RRobinFun {CondSelect K rrobin false}}
	    Proc    = {CondSelect K procedure unit}
	 in
	    {FSDistGeneric L Order Filter Element RRobin Select Proc}
	 else
	    {Exception.raiseError
	     fs(unknownDistributionStrategy
		'FS.distribute' [K Vs] 1)}
	 end 
      end 
   end    
end 


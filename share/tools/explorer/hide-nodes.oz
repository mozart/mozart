%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
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

local

   Hide                       = {NewName}
   UnhideTree                 = {NewName}
   HideFailed                 = {NewName}
   UnhideButFailed            = {NewName}
   UnhideButFailedBelowHidden = {NewName}
   
   %%
   %% Hide all kids
   %%
   proc {HideKids Ks}
      case Ks of nil then skip
      [] K|Kr then {K Hide} {HideKids Kr}
      end
   end

   %%
   %% Unhide all kids
   %%
   fun {IsUnhidableKids Ks}
      case Ks of nil then false
      [] K|Kr then {K isUnhidable($)} orelse {IsUnhidableKids Kr}
      end
   end

   proc {UnhideTreeKids Ks}
      case Ks of nil then skip
      [] K|Kr then {K UnhideTree} {UnhideTreeKids Kr}
      end
   end

   %%
   %% Hide all failed kids
   %%
   fun {IsFailedHidableKids Ks}
      case Ks of nil then false
      [] K|Kr then {K isFailedHidable($)} orelse {IsFailedHidableKids Kr}
      end
   end

   fun {HideFailedKids Ks}
      case Ks of nil then false
      [] K|Kr then IsDirtyKid={K HideFailed($)} in
	 {HideFailedKids Kr} orelse IsDirtyKid
      end
   end

   %%
   %% Hide all but the failed kids
   %%
   fun {IsButFailedUnhidableKids Ks}
      case Ks of nil then false
      [] K|Kr then
	 {K isButFailedUnhidable($)} orelse {IsButFailedUnhidableKids Kr}
      end
   end
				     
   proc {UnhideButFailedKids Ks}
      case Ks of nil then skip
      [] K|Kr then {K UnhideButFailed} {UnhideButFailedKids Kr}
      end
   end

   proc {UnhideButFailedBelowHiddenKids Ks}
      case Ks of nil then skip
      [] K|Kr then
	 {K UnhideButFailedBelowHidden} {UnhideButFailedBelowHiddenKids Kr}
      end
   end

   %%
   %% Hide not yet drawn kids
   %%
   proc {HideUndrawn Ks}
      case Ks of nil then skip
      [] K|Kr then {K hideUndrawn} {HideUndrawn Kr}
      end
   end

   %%
   %% General invariant:
   %%  Below a hidden node, all nodes are marked as dirty and undrawn
   %%

   class Inner
      attr
	 isHidden: false

      %% Mark the path from a node to the root as dirty, such that the layout
      %% can be updated lateron. If an already dirty node is found, the
      %% invariant holds that the entire path to the root node is dirty
      meth dirtyUp
	 if @isDirty then skip else
	    isDirty <- true
	    {self.mom dirtyUp}
	 end
      end

      %%
      %% Toggle unhidden/unhidden nodes
      %%
      meth isHidable($)
	 true
      end
      
      meth hide
	 {self deleteTree}
	 Inner,dirtyUp
	 if @isHidden then
	    isHidden <- false
	 else
	    isHidden <- true
	    {HideKids @kids}
	 end
      end

      meth !Hide
	 isDirty  <- true
	 shape    <- nil
	 %% The invariant guarantees that we do not have to take care
	 %% of still hidden subtrees
	 if @isHidden then skip else {HideKids @kids} end
      end

      %%
      %% Recursively unhide subtrees
      %%
      meth isUnhidable($)
	 @isHidden orelse {IsUnhidableKids @kids}
      end
      
      meth unhideTree
	 Inner,UnhideTree
	 {self.mom dirtyUp}
      end

      meth !UnhideTree
	 isDirty  <- true
	 if @isHidden then
	    isHidden <- false
	    {self deleteTree}
	 end
	 {UnhideTreeKids @kids}
      end

      %%
      %% Hide failed subtree
      %%
      meth isFailedHidable($)
	 if @isHidden then false
	 elseif @choices>0 orelse @isSolBelow then
	    {IsFailedHidableKids @kids}
	 else true
	 end
      end

      meth hideFailed
	 if Inner,HideFailed($) then {self.mom dirtyUp} end
      end

      meth !HideFailed($)
	 if @isHidden then false
	 elseif @choices>0 orelse @isSolBelow then
	    if {HideFailedKids @kids} then isDirty <- true  true
	    else false
	    end
	 else
	    isHidden <- true
	    isDirty  <- true
	    {self deleteTree}
	    {HideKids @kids}
	    true
	 end
      end

      %%
      %% Unhide all but failed subtrees
      %%
      meth isButFailedUnhidable($)
	 if @isHidden then
	    @choices>0 orelse @isSolBelow
	 else {IsButFailedUnhidableKids @kids}
	 end
      end
      
      meth unhideButFailed
	 Inner,UnhideButFailed
	 {self.mom dirtyUp}
      end
      
      meth !UnhideButFailed
	 if @isHidden then
	    %% Since we know (from the testing routines above) that there is
	    %% indeed something to unhide we do not have to analyse this node
	    %% further!
	    {self deleteTree}
	    Inner,UnhideButFailedBelowHidden
	 else
	    isDirty <- true
	    {UnhideButFailedKids @kids}
	 end
      end

      meth !UnhideButFailedBelowHidden
	 isDirty <- true
	 if @choices>0 orelse @isSolBelow then
	    isHidden <- false
	    {UnhideButFailedBelowHiddenKids @kids}
	 else isHidden <- true
	 end
      end

      %%
      %% Hide all not yet drawn nodes
      %%
      meth hideUndrawn
	 if @isDirty then
	    if @isHidden then skip else
	       if @item>0 then {HideUndrawn @kids}
	       else isHidden <- true Inner,Hide
	       end
	    end
	 end
      end

      meth getOverHidden(Cursor $)
	 {self.mom getOverHidden(if @isHidden then self else Cursor end $)}
      end
	    
      meth isHidden($)
	 @isHidden
      end

   end

   class Leaf
      meth isHidable($)            false end
      meth hide                    skip  end
      meth !Hide                   skip  end

      meth isUnhidable($)          false end
      meth unhide                  skip  end
      meth !UnhideTree             skip  end

      meth isFailedHidable($)      false end
      meth hideFailed              skip  end
      meth !HideFailed($)          false end

      meth isButFailedUnhidable($)     false end
      meth !UnhideButFailed            skip  end
      meth !UnhideButFailedBelowHidden skip  end
      
      meth isHidden($)             false end %% JUNK JUNK
      meth getOverHidden(Cursor $) {self.mom getOverHidden(Cursor $)} end

      meth hideUndrawn             skip  end
   end

   class Sentinel
      meth dirtyUp skip end
      meth getOverHidden(Cursor $) Cursor end
   end
   
in

   HideNodes = c(choose:    Inner
		 failed:    Leaf
		 succeeded: Leaf
		 suspended: Leaf
		 sentinel:  Sentinel)
   
end



%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

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
      case Ks of nil then False
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
      case Ks of nil then False
      [] K|Kr then {K isFailedHidable($)} orelse {IsFailedHidableKids Kr}
      end
   end

   fun {HideFailedKids Ks}
      case Ks of nil then False
      [] K|Kr then IsDirtyKid={K HideFailed($)} in
	 {HideFailedKids Kr} orelse IsDirtyKid
      end
   end

   %%
   %% Hide all but the failed kids
   %%
   fun {IsButFailedUnhidableKids Ks}
      case Ks of nil then False
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
	 isHidden: False

      %% Mark the path from a node to the root as dirty, such that the layout
      %% can be updated lateron. If an already dirty node is found, the
      %% invariant holds that the entire path to the root node is dirty
      meth dirtyUp
	 case @isDirty then skip else
	    isDirty <- True
	    {self.mom dirtyUp}
	 end
      end

      %%
      %% Toggle unhidden/unhidden nodes
      %%
      meth isHidable($)
	 True
      end
      
      meth hide
	 TkNodes.choose,deleteTree
	 Inner,dirtyUp
	 isDrawn  <- False
	 case @isHidden then
	    isHidden <- False
	 else
	    isHidden <- True
	    {HideKids @kids}
	 end
      end

      meth !Hide
	 isDirty  <- True
	 isDrawn  <- False
	 shape    <- nil
	 %% The invariant guarantees that we do not have to take care
	 %% of still hidden subtrees
	 case @isHidden then skip else {HideKids @kids} end
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
	 isDirty  <- True
	 case @isHidden then
	    isHidden <- False
	    isDrawn  <- False
	    TkNodes.choose,deleteTree
	 else skip
	 end
	 {UnhideTreeKids @kids}
      end

      %%
      %% Hide failed subtree
      %%
      meth isFailedHidable($)
	 case @isHidden then False
	 elsecase @choices>0 orelse @isSolBelow then
	    {IsFailedHidableKids @kids}
	 else True
	 end
      end

      meth hideFailed
	 case Inner,HideFailed($) then {self.mom dirtyUp}
	 else skip
	 end
      end

      meth !HideFailed($)
	 case @isHidden then False
	 elsecase @choices>0 orelse @isSolBelow then
	    case {HideFailedKids @kids} then isDirty <- True  True
	    else False
	    end
	 else
	    isHidden <- True
	    isDrawn  <- False
	    isDirty  <- True
	    TkNodes.choose,deleteTree
	    {HideKids @kids}
	    True
	 end
      end

      %%
      %% Unhide all but failed subtrees
      %%
      meth isButFailedUnhidable($)
	 case @isHidden then
	    @choices>0 orelse @isSolBelow
	 else {IsButFailedUnhidableKids @kids}
	 end
      end
      
      meth unhideButFailed
	 Inner,UnhideButFailed
	 {self.mom dirtyUp}
      end
      
      meth !UnhideButFailed
	 case @isHidden then
	    %% Since we know (from the testing routines above) that there is
	    %% indeed something to unhide we do not have to analyse this node
	    %% further!
	    isDrawn <- False
	    TkNodes.choose,deleteTree
	    Inner,UnhideButFailedBelowHidden
	 else
	    isDirty <- True
	    {UnhideButFailedKids @kids}
	 end
      end

      meth !UnhideButFailedBelowHidden
	 isDirty <- True
	 case @choices>0 orelse @isSolBelow then
	    isHidden <- False
	    {UnhideButFailedBelowHiddenKids @kids}
	 else isHidden <- True
	 end
      end

      %%
      %% Hide all not yet drawn nodes
      %%
      meth hideUndrawn
	 case @isDirty then
	    case @isHidden then skip else
	       case @isDrawn then {HideUndrawn @kids}
	       else isHidden <- True Inner,Hide
	       end
	    end
	 else skip
	 end
      end

      meth getOverHidden(Cursor $)
	 {self.mom getOverHidden(case @isHidden then self else Cursor end $)}
      end
	    
      meth isHidden($)
	 @isHidden
      end

   end

   class Leaf
      meth isHidable($)            False end
      meth hide                    skip  end
      meth !Hide                   skip  end

      meth isUnhidable($)          False end
      meth unhide                  skip  end
      meth !UnhideTree             skip  end

      meth isFailedHidable($)      False end
      meth hideFailed              skip  end
      meth !HideFailed($)          False end

      meth isButFailedUnhidable($)     False end
      meth !UnhideButFailed            skip  end
      meth !UnhideButFailedBelowHidden skip  end
      
      meth isHidden($)             False end %% JUNK JUNK
      meth getOverHidden(Cursor $) {self.mom getOverHidden(Cursor $)} end

      meth hideUndrawn             skip  end
   end

   class Sentinel
      meth dirtyUp skip end
      meth getOverHidden(Cursor $) Cursor end
   end
   
in

   HideNodes = c(choose:    !Inner
		 failed:    !Leaf
		 succeeded: !Leaf
		 blocked:   !Leaf
		 sentinel:  !Sentinel)
   
end



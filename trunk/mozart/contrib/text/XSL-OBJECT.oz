%%% A stylesheet object is constructed from a description as returned
%%% by XSL-READ.parseFile
%%%
%%% processing a document involves (1) processing the white space
%%% (2) building id maps (id->elt & elt->id) (3) applying the
%%% templates to the root node.

functor

define
   class StyleSheet
      feat element2id
      meth init(Spec)
	 case Spec of stylesheet(id        : ID
				 space     : SPACE
				 constants : CONSTANTS
				 templates : TEMPLATES)
	    %%
	    %% {self.element2id E} applied to element E
	    %% returns unit or the attribute name that holds E's
	    %% id
	    %%
	    local
	       ElementId = {Dictionary.new}
	       GlobalIds = {NewCell nil}
	       {ForAll ID
		proc {$ A#E}
		   if E==unit then L in
		      {Exchange GlobalIds L A|L}
		   else
		      {Dictionary.put ElementId E A}
		   end
		end}
	       {Assign GlobalIds {Reverse {Access GlobalIds}}}
	       fun {ElementToId E}
		  A = {CondSelect ElementId E.type unit}
	       in
		  if A\=unit then A else
		     try
			{ForAll {Access GlobalIds}
			 proc {$ A}
			    if {HasFeature E.attribute A} then
			       raise ok(A) end
			    end
			 end}
			unit
		     catch ok(A) then A end
		  end
	       end
	    in
	       self.element2id
	    end
	 end
      end
      meth 
   end
end

%%% A stylesheet object is constructed from a description as returned
%%% by XSL-READ.parseFile
%%%
%%% processing a document involves (1) processing the white space
%%% (2) building id maps (id->elt & elt->id) (3) applying the
%%% templates to the root node.
%%%
%%% we assume that the sgml document to be processed has been put
%%% in the following format:
%%%
%%% DOC ::= root([E1...En])
%%% E   ::= element(type:TYPE attribute:o(A1...An) content:[E1...En])
%%%       | cdata(STRING)
%%%       | pi(NAME DATA)
%%% A   ::= NAME:attribute(name:NAME value:STRING)

functor

define
   class StyleSheet
      feat element2id
      attr idmap
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
	       self.element2id = ElementToId
	    end
	 end
      end
      meth transform(Root Result)
	 Prepare(Root)
	 ...
      end
      %%
      %% build id maps
      %%
      meth Prepare(Root)
	 idmap <- {Dictionary.new}
	 {MakeIdMap Root @idmap}
      end
   end
   %%
   proc {MakeIdMap Node Dict ElementToId}
      proc {Loop Node}
	 case Node
	 of root(L) then {ForAll L Loop}
	 [] element(type:T attribute:A content:C) then
	    Key = {ElementToId Node}
	    Val = if Key==unit then unit else
		     {CondSelect A Key}
		  end
	 in

	 [] cdata(_) then skip
	 [] pi(_ _) then skip
	 end
      end
   end
end

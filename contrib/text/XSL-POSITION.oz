%%% -*-oz-*-
%%%
%%% We implement here the predicates for testing that a node has a
%%% certain position among its siblings.  Each position predicate
%%% is applied to 2 arguments: the Node and the list representing the
%%% `content' of its parent; it and its siblings are members of this
%%% list.  The predicate returns a boolean.
%%%
%%% The features exported by this functor are precisely the identifiers
%%% specified in the XSL proposal.
%%%
functor
export
   'first-of-type'	: `first-of-type`
   'first-of-any'	: `first-of-any`
   'last-of-type'	: `last-of-type`
   'last-of-any'	: `last-of-any`
   'only-of-type'	: `only-of-type`
   'only-of-any'	: `only-of-any`
   %% negations
   'not-first-of-type'	: `not-first-of-type`
   'not-first-of-any'	: `not-first-of-any`
   'not-last-of-type'	: `not-last-of-type`
   'not-last-of-any'	: `not-last-of-any`
   'not-only-of-type'	: `not-only-of-type`
   'not-only-of-any'	: `not-only-of-any`
define
   fun {`first-of-type` Node Siblings}
      {Get_First_Of_Type Siblings Node.type}.index==Node.index
   end

   fun {Get_First_Of_Type Nodes Type}
      case Nodes of H|T then
	 if {CondSelect H type unit}==Type then H else
	    {Get_First_Of_Type T Type}
	 end
      end
   end

   fun {`first-of-any` Node Siblings}
      {Get_First_Element Siblings}.index==Node.index
   end

   fun {Get_First_Element Nodes}
      case Nodes of H|T then
	 if {CondSelect H type unit}\=unit then H else
	    {Get_First_Element T}
	 end
      end
   end

   fun {`last-of-type` Node Siblings}
      {Get_Last_Of_Type Siblings Node.type unit}.index==Node.index
   end

   fun {Get_Last_Of_Type Nodes Type Last}
      case Nodes of nil then Last
      [] H|T then
	 {Get_Last_Of_Type T Type
	  if {CondSelect H type unit}==Type then H else Last end}
      end
   end

   fun {`last-of-any` Node Siblings}
      {Get_Last_Element Siblings unit}.index==Node.index
   end

   fun {Get_Last_Element Nodes Last}
      case Nodes of nil then Last
      [] H|T then
	 {Get_Last_Element T
	  if {CondSelect H type unit}\=unit then H else Last end}
      end
   end

   fun {`only-of-type` Node Siblings}
      case Siblings of nil then true
      [] H|T then
	 ({CondSelect H type unit}\=Node.type orelse
	  H.index==Node.index) andthen
	 {`only-of-type` Node T}
      end
   end

   fun {`only-of-any` Node Siblings}
      case Siblings of nil then true
      [] H|T then
	 ({CondSelect H type unit}==unit orelse
	  H.index==Node.index) andthen
	 {`only-of-any` Node T}
      end
   end

   fun {`not-first-of-type` Node Siblings}
      {Not {`first-of-type` Node Siblings}}
   end

   fun {`not-first-of-any` Node Siblings}
      {Not {`first-of-any` Node Siblings}}
   end

   fun {`not-last-of-type` Node Siblings}
      {Not {`last-of-type` Node Siblings}}
   end

   fun {`not-last-of-any` Node Siblings}
      {Not {`last-of-any` Node Siblings}}
   end

   fun {`not-only-of-type` Node Siblings}
      {Not {`only-of-type` Node Siblings}}
   end

   fun {`not-only-of-any` Node Siblings}
      {Not {`only-of-any` Node Siblings}}
   end
end

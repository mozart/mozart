functor
   %% these generators generate their nodes in the expected order
   %% for the corresponding axis
export
   NewGenMember
   NewGenChild
   NewGenSelf
   NewGenParent
   NewGenAncestorOrSelf
   NewGenAncestor
   NewGenDescendantOrSelf
   NewGenDescendant
   NewGenAttribute
   NewGenFollowingSibling
   NewGenPrecedingSibling
   NewGenFollowing
   NewGenPreceding
define
   fun {GenEmpty} unit end
   fun {NewGenMember L}
      if L==nil then GenEmpty else
	 C={NewCell L}
      in
	 fun {$}
	    L1 L2
	 in
	    {Exchange C L1 L2}
	    case L1
	    of nil then L2=nil unit
	    [] H|T then L2=T H end
	 end
      end
   end
   fun {NewGenChild Node}
      {NewGenMember {CondSelect Node children nil}}
   end
   fun {NewGenSelf N}
      if N==unit then GenEmpty else
	 C={NewCell N}
      in
	 fun {$} {Exchange C $ unit} end
      end
   end
   fun {NewGenParent Node}
      {NewGenSelf {CondSelect Node parent unit}}
   end
   fun {NewGenAncestorOrSelf Node}
      if Node==unit then GenEmpty else
	 C={NewCell Node}
      in
	 fun {$} A1 A2 in
	    {Exchange C A1 A2}
	    if A1==unit then A1=unit unit
	    else
	       A2={CondSelect A1 parent unit}
	       A1
	    end
	 end
      end
   end
   fun {NewGenAncestor Node}
      {NewGenAncestorOrSelf {CondSelect Node parent unit}}
   end
   fun {NewGenDescendantOrSelfL Nodes}
      if Nodes==nil then GenEmpty else
	 Stack={NewCell [Nodes]}
      in
	 fun {$} L in
	    case {Exchange Stack $ L}
	    of nil then L=nil unit
	    [] (H|T1)|T2 then
	       LL={CondSelect H children nil}
	    in
	       if LL==nil
	       then if T1==nil
		    then L=T2
		    else L=(T1|T2) end
	       elseif T1==nil
	       then L=(LL|T2)
	       else L=(LL|T1|T2) end
	       H
	    end
	 end
      end
   end
   fun {NewGenDescendantOrSelf Node}
      if Node==unit then GenEmpty else
	 {NewGenDescendantOrSelfL [Node]}
      end
   end
   fun {NewGenDescendant Node}
      if Node==unit then GenEmpty else
	 {NewGenDescendantOrSelfL {CondSelect Node children nil}}
      end
   end
   fun {NewGenAttribute Node}
      {NewGenMember {CondSelect Node alist nil}}
   end
   fun {LocalFollowing I L}
      case L
      of H|T then
	 if H.index==I then T
	 else {LocalFollowing I T} end
      else nil end
   end
   fun {LocalPreceding I L Acc}
      case L
      of H|T then
	 if H.index==I then Acc
	 else {LocalPreceding I T H|Acc} end
      else nil end
   end
   fun {NewGenFollowingSibling Node}
      case {CondSelect Node parent unit}
      of unit then GenEmpty
      [] PAR  then
	 {NewGenMember
	  {LocalFollowing
	   Node.index {CondSelect PAR children nil}}}
      end
   end
   fun {NewGenPrecedingSibling Node}
      case {CondSelect Node parent unit}
      of unit then GenEmpty
      [] PAR  then
	 {NewGenMember
	  {LocalPreceding
	   Node.index {CondSelect PAR children nil} nil}}
      end
   end
   fun {NewGenFollowing Node}
      if Node==unit then GenEmpty else
	 Stack = {NewCell [Node]}
	 fun {Next} L in
	    case {Exchange Stack $ L}
	    of unit then L=unit unit
	    [] [H] then P={CondSelect H parent unit} in
	       if P==unit then L=unit unit
	       else
		  LL={CondSelect P children nil}
	       in
		  case {LocalFollowing H.index LL}
		  of nil then L=[P]
		  [] LLL then L=[LLL P] end
		  {Next}
	       end
	    [] LL|T then
	       case LL
	       of nil then L=T {Next}
	       [] H|TT then
		  LLL={CondSelect H children nil}
	       in
		  if LLL==nil then
		     if TT==nil then L=T
		     else L=(TT|T) end
		  elseif TT==nil then L=(LLL|T)
		  else L=(LLL|TT|T) end
		  H
	       end
	    end
	 end
      in
	 Next
      end
   end
   fun {PrecPush L Tail}
      case L
      of nil then Tail
      [] H|T then
	 down(H)|{PrecPush T Tail}
      end
   end
   fun {NewGenPreceding Node}
      if Node==unit then GenEmpty else
	 Stack = {NewCell [up(Node)]}
	 fun {Next} L in
	    case {Exchange Stack $ L}
	    of unit then L=unit unit
	    [] nil  then L=unit unit
	    [] H|T then
	       case H
	       of up(Node)   then
		  PAR={CondSelect Node parent unit}
		  PRC={LocalPreceding
		       {CondSelect Node index unit}
		       {CondSelect PAR children nil} nil}
	       in
		  L={PrecPush PRC just(PAR)|up(PAR)|T}
		  {Next}
	       [] just(Node) then
		  L=T Node
	       [] down(Node) then
		  L={PrecPush
		     {Reverse {CondSelect Node children nil}}
		     just(Node)|T}
		  {Next}
	       end
	    end
	 end
      in
	 Next
      end
   end
end

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

   GetCenter      = {NewName}
   UpperSpaceI    = VerSpaceI - CircleWidthI
   UpperSpaceF    = {IntToFloat UpperSpaceI}

   class Sentinel
      meth !GetCenter(X1 Y1 ?X2 ?Y2)
	 X2=X1+SentinelX Y2=Y1+SentinelY
      end
   end
   
   class NumberNode
      attr number:0
      %% If @number==0, the node is not tagged with a number,
      %% otherwise it is tagged with that number and the number
      %% item is refernced by the tag n#@number

      meth getNumber(Scale Font TakeNumber ?N)
	 if @number==0 then
	    Canvas  = self.canvas
	    Numbers = Canvas.numbers
	    Actions = Canvas.actions
	    X Y
	 in
	    N      = TakeNumber
	    number <- TakeNumber
	    {self getCenter(X Y)}
	    if Font\=false then
	       {Canvas
		tkCreate(text(Scale * {IntToFloat X}
			      Scale * {IntToFloat if {self isHidden($)}
						  then
						     Y + UpperSpaceI div 2
						  else Y
						  end}
			      font: Font
			      text: N
			      tags: q(Actions Numbers n#N)))}
	    end
	 else N = @number
	 end
      end

      meth redrawNumber(Scale Font)
	 Canvas  = self.canvas
	 Numbers = Canvas.numbers
	 Actions = Canvas.actions
	 N       = @number
	 X Y
      in
	 {self getCenter(?X ?Y)}
	 {Canvas
	  tkCreate(text(Scale * {IntToFloat X}
			Scale * {IntToFloat if {self isHidden($)} then
					       Y + UpperSpaceI div 2
					    else Y
					    end}
			font: Font
			text: N
			tags: q(Numbers Actions n#N)))}
      end

      meth clearNumber
	 number <- 0
      end
   end

   
   class TkNode
      attr
	 item:0
         %% If @item==0 then the node hasn't been drawn yet, otherwise
         %% @item gives the itemnumber in the canvas

      meth !GetCenter(X1 Y1 ?X2 ?Y2)
	 {self.mom GetCenter(X1+@offset Y1+VerSpaceI ?X2 ?Y2)}
      end
	    
      meth getCenter(?X ?Y)
	 TkNode,GetCenter(0 0 ?X ?Y)
      end

   end
   

   local
      proc {Purge Ks}
	 case Ks of nil then skip
	 [] K|Kr then {K purge} {Purge Kr}
	 end
      end
     
   in
      class ChooseNode
	 from TkNode NumberNode
	 	 
	 meth moveNode(MomX MyX MyByX MyY Scale)
	    Canvas   = self.canvas
	    Item     = @item
	    NodeItem = Item+1
	    Number   = @number
	 in
	    {Canvas tk(coo Item
		       Scale*{IntToFloat MomX}
		       Scale*{IntToFloat (MyY - UpperSpaceI)}
		       Scale*{IntToFloat MyX}
		       Scale*{IntToFloat (MyY - CircleWidthI)})}
	    {Canvas tk(mo NodeItem Scale*{IntToFloat MyByX} 0)}
	    if Number\=0 then
	       {Canvas tk(mo n#Number Scale*{IntToFloat MyByX} 0)}
	    end
	    if @toDo\=nil orelse @isHidden then skip else
	       {Canvas tk(itemco NodeItem
			  fi: ChooseTermColor
			  wi: ThickNodeBorderWidth)}
	    end
	 end

	 meth moveTree(MomX MyX MyByX MyY Scale)
	    Canvas    = self.canvas
	    Item      = @item
	    Number    = @number
	    ScaledByX = Scale * {IntToFloat MyByX}
	 in
	    {Canvas tk(coo Item
		       Scale*{IntToFloat MomX}
		       Scale*{IntToFloat (MyY - UpperSpaceI)}
		       Scale*{IntToFloat MyX}
		       Scale*{IntToFloat (MyY - RectangleWidthI)})}
	    {MoveTree Canvas ScaledByX
	     ChooseNode,GetMoveIds(@kids Item+1|if Number==0 then nil
						else [n#Number]
						end $)}
	 end

	 meth GetMoveIds(Ks Is $)
	    case Ks of nil then Is
	    [] K|Kr then
	       ChooseNode,GetMoveIds(Kr {K getMoveIds(Is $)} $)
	    end
	 end

	 meth getMoveIds(Is $)
	    Item   = @item
	 in
	    if Item==0 then Is else
	       Number = @number
	    in
	       ChooseNode,GetMoveIds(@kids Item|Item+1|if Number==0 then Is
						       else n#Number|Is
						       end $)
	    end
	 end
	 
	 meth DrawKids(Ks Break MomX MyY Scale Font)
	    case Ks of nil then skip
	    [] K|Kr then
	       {K drawTree(Break MomX MyY Scale Font)}
	       {self DrawKids(Kr Break MomX MyY Scale Font)}
	    end
	 end
	 
	 meth purge
	    offset <- 0
	    shape  <- nil
	    {Purge @kids}
	 end
	 
	 meth drawTree(Break MomX MyY Scale Font)
	    if {IsFree Break} then
	       NewOffset   = @offset.1
	       MyX         = MomX + NewOffset
	       ScaledWidth = Scale * CircleWidthF
	       ScaledMomX  = Scale * {IntToFloat MomX}
	       ScaledMyX   = Scale * {IntToFloat MyX}
	       ScaledMyY   = Scale * {IntToFloat MyY}
	       Canvas      = self.canvas
	       Actions     = Canvas.actions
	       Number      = @number
	    in
	       offset <- NewOffset
	       item   <- {Canvas tkGet(line(ScaledMomX
					    ScaledMyY - Scale * UpperSpaceF
					    ScaledMyX
					    ScaledMyY - ScaledWidth
					    width: LinkWidth) $)}
	       if @isHidden then
		  ScaledVerSpace = Scale * VerSpaceF
		  ScaledHorSpace = Scale * HalfHorSpaceF
	       in
		  {Canvas
		   tkCreate(polygon(ScaledMyX
				    ScaledMyY - ScaledWidth
				    ScaledMyX - ScaledHorSpace
				    ScaledMyY + ScaledVerSpace
				    ScaledMyX + ScaledHorSpace
				    ScaledMyY + ScaledVerSpace
				    tags:    Actions
				    fill:    if @isSolBelow then
						if @choices>0 then
						   StuckColor
						else EntailedColor
						end
					     else
						if @choices>0 then
						   PartialFailedColor
						else FailedColor
						end
					     end
				    width:   NodeBorderWidth
				    outline: LineColor))}
		  if Number\=0 andthen Font\=false then
		     {Canvas
		      tkCreate(te(ScaledMyX
				  ScaledMyY +
				  (ScaledVerSpace - ScaledWidth) / 2.0
				  fo: Font
				  te: Number
				  ta: q(Actions Canvas.numbers
					n#Number)))}
		  end
	       else
		  {Canvas
		   tkCreate(ov(ScaledMyX - ScaledWidth
			       ScaledMyY - ScaledWidth
			       ScaledMyX + ScaledWidth
			       ScaledMyY + ScaledWidth
			       if @toDo==nil then
				  o(fi:  ChooseTermColor
				    wi: ThickNodeBorderWidth)
			       else
				  o(fi:  ChooseColor
				    wi: NodeBorderWidth)
			       end
			       ta: Actions))}
		  if Number\=0 andthen Font\=false then
		     {Canvas
		      tkCreate(te(ScaledMyX ScaledMyY
				  fo: Font
				  te: Number
				  ta: q(Actions Canvas.numbers
					n#Number)))}
		  end
		  ChooseNode,DrawKids(@kids Break MyX MyY+VerSpaceI
				      Scale Font)
	       end
	       isDirty <- false
	    else
	       if @kids\=nil then
		  isHidden <- true
		  ChooseNode,purge
	       end
	       {Break broken(self)}
	    end
	 end
	 
	 meth deleteTree
	    Item   = @item
	    Number = @number
	 in
	    if Item\=0 then
	       item <- 0
	       {self.canvas
		tk(delete Item Item+1
		   b(ChooseNode,GetDelIds(@kids
					  if Number==nil then nil
					  else [n#Number]
					  end $)))}
	    end
	 end

	 meth GetDelIds(Ks Is $)
	    case Ks of nil then Is
	    [] K|Kr then
	       ChooseNode,GetDelIds(Kr {K getDelIds(Is $)} $)
	    end
	 end

	 meth getDelIds(Is $)
	    Item   = @item
	 in
	    if Item==0 then Is else
	       Number = @number
	    in
	       item <- 0
	       ChooseNode,GetDelIds(@kids Item|Item+1|if Number==0 then Is
						      else n#Number|Is
						      end $)
	    end
	 end
	 
      end
   end

   local

      class LeafNode
	 from TkNode

	 meth purge
	    offset <- 0
	 end
	 
	 meth deleteTree
	    Item = @item
	 in
	    if Item\=0 then
	       {self.canvas tk(de Item Item+1)}
	       item <- 0
	    end
	 end      
	 
      end

   in

      class FailedNode
	 from LeafNode
	 
	 meth drawTree(Break MomX MyY Scale Font)
	    NewOffset   = @offset.1
	    MyX         = MomX + NewOffset
	    ScaledWidth = Scale * RectangleWidthF
	    ScaledMomX  = Scale * {IntToFloat MomX}
	    ScaledMyX   = Scale * {IntToFloat MyX}
	    ScaledMyY   = Scale * {IntToFloat MyY}
	    Canvas      = self.canvas
	 in
	    offset <- NewOffset
	    item   <- {Canvas tkGet(li(ScaledMomX
				       ScaledMyY - Scale * UpperSpaceF
				       ScaledMyX
				       ScaledMyY - ScaledWidth
				       wi: LinkWidth) $)}
	    {Canvas tkCreate(rectangle(ScaledMyX - ScaledWidth
				       ScaledMyY - ScaledWidth
				       ScaledMyX + ScaledWidth
				       ScaledMyY + ScaledWidth      
				       fill: FailedColor
				       width: NodeBorderWidth
				       outline: LineColor))}
	 end
	 
	 meth moveNode(MomX MyX MyByX MyY Scale)
	    Canvas = self.canvas
	    Item   = @item
	 in
	    {Canvas tk(coo Item
		       Scale*{IntToFloat MomX}
		       Scale*{IntToFloat (MyY - UpperSpaceI)}
		       Scale*{IntToFloat MyX}
		       Scale*{IntToFloat (MyY - RectangleWidthI)})}
	    {Canvas tk(move Item+1 Scale*{IntToFloat MyByX} 0)}
	 end
      
	 meth getMoveIds(Is $)
	    Item = @item
	 in
	    if Item==0 then Is else Item|Item+1|Is end
	 end
	 
	 meth getDelIds(Is $)
	    Item = @item
	 in
	    if Item==0 then Is else item <-0 Item|Item+1|Is end
	 end
	 
      end

      class SuspendedNode
	 from LeafNode
	 
	 meth getMoveIds(Is $)
	    Item = @item
	 in
	    if Item==0 then Is else Item|Item+1|Is end
	 end
	 
	 meth getDelIds(Is $)
	    Item = @item
	 in
	    if Item==0 then Is else item <- 0  Item|Item+1|Is end
	 end
	 
	 meth drawTree(Break MomX MyY Scale Font)
	    NewOffset       = @offset.1
	    MyX             = MomX + NewOffset
	    ScaledHalfWidth = Scale * SmallRectangleWidthF
	    ScaledFullWidth = Scale * RectangleWidthF
	    ScaledMomX      = Scale * {IntToFloat MomX}
	    ScaledMyX       = Scale * {IntToFloat MyX}
	    ScaledMyY       = Scale * {IntToFloat MyY}
	    X0              = ScaledMyX - ScaledFullWidth
	    X1              = ScaledMyX - ScaledHalfWidth
	    X2              = ScaledMyX
	    X3              = ScaledMyX + ScaledHalfWidth
	    X4              = ScaledMyX + ScaledFullWidth
	    Y0              = ScaledMyY - ScaledFullWidth
	    Y1              = ScaledMyY - ScaledHalfWidth
	    Y2              = ScaledMyY
	    Y3              = ScaledMyY + ScaledHalfWidth
	    Y4              = ScaledMyY + ScaledFullWidth
	    Canvas          = self.canvas
	 in
	    offset <- NewOffset
	    item   <- {Canvas tkGet(li(ScaledMomX
				       ScaledMyY - Scale * UpperSpaceF
				       ScaledMyX
				       ScaledMyY - ScaledHalfWidth
				       wi:LinkWidth) $)}
	    {Canvas
	     tkCreate(po(X0 Y0 X2 Y1 X4 Y0 X3 Y2 X4 Y4 X2 Y3 X0 Y4 X1 Y2
			 fill: SuspendedColor
			 width: NodeBorderWidth
			 outline: LineColor))}
	 end
	 
	 meth moveNode(MomX MyX MyByX MyY Scale)
	    Canvas = self.canvas
	    Item   = @item
	 in
	    {Canvas tk(coo Item
		       Scale*{IntToFloat MomX}
		       Scale*{IntToFloat (MyY - UpperSpaceI)}
		       Scale*{IntToFloat MyX}
		       Scale*{IntToFloat (MyY - RectangleWidthI)})}
	    {Canvas tk(mo Item+1 Scale*{IntToFloat MyByX} 0)}
	 end
      
      end

      local
	 class SucceededNode
	    from LeafNode NumberNode
	    
	    meth getMoveIds(Is $)
	       Item   = @item
	    in
	       if Item==0 then Is else
		  Number = @number
	       in
		  Item|Item+1|if Number==0 then Is else n#Number|Is end
	       end
	    end
	 
	    meth getDelIds(Is $)
	       Item   = @item
	    in
	       if Item==0 then Is else
		  Number = @number
	       in
		  item <- 0
		  Item|Item+1|if Number==0 then Is else n#Number|Is end
	       end
	    end
	 
	    meth drawTree(MomX MyY Scale Font Color Width)
	       NewOffset   = @offset.1
	       MyX         = MomX + NewOffset
	       ScaledWidth = Scale * RhombeWidthF
	       ScaledMomX  = Scale * {IntToFloat MomX}
	       ScaledMyX   = Scale * {IntToFloat MyX}
	       ScaledMyY   = Scale * {IntToFloat MyY}
	       Canvas      = self.canvas
	       Actions     = Canvas.actions
	       Number      = @number
	       X0          = ScaledMyX - ScaledWidth
	       X1          = ScaledMyX
	       X2          = ScaledMyX + ScaledWidth
	       Y0          = ScaledMyY - ScaledWidth
	       Y1          = ScaledMyY
	       Y2          = ScaledMyY + ScaledWidth
	    in
	       offset <- NewOffset
	       item   <- {Canvas tkGet(li(ScaledMomX
					  ScaledMyY - Scale * UpperSpaceF
					  ScaledMyX
					  ScaledMyY - ScaledWidth
					  width: LinkWidth) $)}
	       {Canvas
		tkCreate(po(X0 Y1 X1 Y0 X2 Y1 X1 Y2 X0 Y1
			    fill:    Color
			    width:   Width
			    outline: LineColor
			    tags:    Actions))}
	       if Number\=0 andthen Font\=false then
		  {Canvas tkCreate(te(ScaledMyX ScaledMyY
				      font: Font
				      text: Number
				      tags: q(Actions Canvas.numbers
					    n#Number)))}
	       end
	    end

	    meth moveNode(MomX MyX MyByX MyY Scale)
	       Canvas = self.canvas
	       Item   = @item
	       Number = @number
	    in
	       {Canvas tk(coo Item
			  Scale*{IntToFloat MomX}
			  Scale*{IntToFloat (MyY - UpperSpaceI)}
			  Scale*{IntToFloat MyX}
			  Scale*{IntToFloat (MyY - RhombeWidthI)})}
	       {Canvas tk(mo Item+1 Scale*{IntToFloat MyByX} 0)}
	       if Number\=0 then
		  {Canvas tk(mo n#Number Scale*{IntToFloat MyByX} 0)}
	       end
	    end
	    
	 end

      in
	 
	 class EntailedNode
	    from SucceededNode
	    
	    meth drawTree(Break MomX MyY Scale Font)
	       SucceededNode,drawTree(MomX MyY Scale Font
				      EntailedColor ThickNodeBorderWidth)
	    end
	    
	 end

   
	 class StuckNode
	    from SucceededNode

	    meth drawTree(Break MomX MyY Scale Font)
	       SucceededNode,drawTree(MomX MyY Scale Font
				      StuckColor NodeBorderWidth)
	    end
	    
	 end

      end

   end
   
in

   TkNodes=c(choose:    ChooseNode
	     failed:    FailedNode
	     suspended: SuspendedNode
	     stuck:     StuckNode
	     entailed:  EntailedNode
	     sentinel:  Sentinel)

end








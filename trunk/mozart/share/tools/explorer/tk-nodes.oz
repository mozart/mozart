%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   GetCenterAbove = {NewName}
   UpperSpaceI    = VerSpaceI - CircleWidthI
   UpperSpaceF    = {IntToFloat UpperSpaceI}

   class NumberNode
      attr number: False

      meth getNumber(Scale Font TakeNumber ?N)
	 case @number==False then
	    Canvas  = self.canvas
	    Numbers = Canvas.numbers
	    Actions = Canvas.actions
	    X Y Above
	 in
	    N = TakeNumber
	    number <- TakeNumber
	    <<getCenterAbove(X Y Above)>>
	    case Font==False then true else
	       {Canvas tk(crea text
			  Scale * {IntToFloat X}
			  Scale * {IntToFloat case <<isHidden($)>> then
						 Y + UpperSpaceI div 2
					      else Y
					      end}
			  o(font: Font
			    text: N
			    tags: q(NodePrefix#self.suffix
				    Actions Numbers b(Above))))}
	    end
	 else N = @number
	 end
      end

      meth redrawNumber(Scale Font)
	 Canvas  = self.canvas
	 Numbers = Canvas.numbers
	 Actions = Canvas.actions
	 X Y Above
      in
	 <<getCenterAbove(X Y Above)>>
	 {Canvas tk(crea text
		    Scale * {IntToFloat X}
		    Scale * {IntToFloat case <<isHidden($)>> then
					   Y + UpperSpaceI div 2
					else Y
					end}
		    o(font: Font
		      text: @number
		      tags: q(NodePrefix#self.suffix Numbers Actions
			      b(Above))))}
      end

      meth clearNumber
	 number <- False
      end
   end

   
   class TkNode
      attr
	 isDrawn: False
      feat
	 suffix

      meth init
	 self.suffix = {self.canvas.genTagId get($)}
      end
	 
      meth getCenterAbove(?X ?Y ?Above)
	 <<GetCenterAbove(0 0 nil ?X ?Y ?Above)>>
      end

      meth GetCenter(X1 Y1 ?X2 ?Y2)
	 case self.mom of !False then X2=X1+RootX Y2=Y1+RootY
	 elseof Mom then {Mom GetCenter(X1+@offset Y1+VerSpaceI ?X2 ?Y2)}
	 end
      end
	    
      meth getCenter(?X ?Y)
	 <<TkNode GetCenter(0 0 ?X ?Y)>>
      end

      meth close
	 Suffix = self.suffix
      in
	 {self.canvas case self.mom of !False then
			 tk(delete NodePrefix#Suffix)
		      else
			 tk(delete NodePrefix#Suffix LinkPrefix#Suffix)
		      end}
      end

   end
   

   local
      TkMoveOpt     = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#ChooseTermColor#
			  ' -width '#TermNodeBorderWidth}})
      TkCreaTermOpt = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#ChooseTermColor#
			  ' -width '#TermNodeBorderWidth}})
      TkCreaOpt     = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#ChooseColor#
			  ' -width '#NodeBorderWidth}})

      proc {Purge Ks}
	 case Ks of nil then true
	 [] K|Kr then {K purge} {Purge Kr}
	 end
      end
     
   in
      class ChooseNode
	 from TkNode NumberNode
	 	 
	 meth moveNode(MomX MyX MyByX MyY Scale)
	    Canvas = self.canvas
	    Suffix = self.suffix
	    Node   = !NodePrefix#Suffix
	 in
	    case self.mom of !False then true else
	       {Canvas tk(coords LinkPrefix#Suffix
			  Scale*{IntToFloat MomX}
			  Scale*{IntToFloat (MyY - UpperSpaceI)}
			  Scale*{IntToFloat MyX}
			  Scale*{IntToFloat (MyY - CircleWidthI)})}
	    end
	    {Canvas tk(move Node Scale*{IntToFloat MyByX} 0)}
	    case @toDo\=nil orelse @isHidden then true else
	       {Canvas tk(itemconfigure Node TkMoveOpt)}
	    end
	 end

	 meth moveTree(MomX MyX MyByX MyY Scale)
	    Canvas = self.canvas
	    Suffix = self.suffix
	 in
	    case self.mom of !False then true else
	       {Canvas tk(coords LinkPrefix#Suffix
			  Scale*{IntToFloat MomX}
			  Scale*{IntToFloat (MyY - UpperSpaceI)}
			  Scale*{IntToFloat MyX}
			  Scale*{IntToFloat (MyY - RectangleWidthI)})}
	    end
	    {Canvas tk(move TreePrefix#Suffix Scale*{IntToFloat MyByX} 0)}
	 end

      
	 meth DrawKids(Ks Break MomTree MomX MyY Scale Font)
	    case Ks of nil then true
	    [] K|Kr then
	       {K drawTree(Break MomTree MomX MyY Scale Font)}
	       <<DrawKids(Kr Break MomTree MomX MyY Scale Font)>>
	    end
	 end
	 
	 meth purge
	    offset <- 0
	    shape  <- nil
	    {Purge @kids}
	 end
	 
	 meth drawTree(Break MomTree MomX MyY Scale Font)
	    case {System.isVar Break} then
	       NewOffset   = @offset.1
	       MyX         = MomX + NewOffset
	       ScaledWidth = Scale * CircleWidthF
	       ScaledMomX  = Scale * {IntToFloat MomX}
	       ScaledMyX   = Scale * {IntToFloat MyX}
	       ScaledMyY   = Scale * {IntToFloat MyY}
	       Canvas      = self.canvas
	       Suffix      = self.suffix
	       Node        = !NodePrefix#Suffix
	       Tree        = !TreePrefix#Suffix
	       Actions     = Canvas.actions
	    in
	       offset <- NewOffset
	       case self.mom==False then true else
		  {Canvas tk(crea line
			     ScaledMomX
			     ScaledMyY - Scale * UpperSpaceF
			     ScaledMyX
			     ScaledMyY - ScaledWidth
			     o(tag:   q(LinkPrefix#Suffix MomTree)
			       width: LinkWidth))}
	       end
	       case @isHidden then
		  ScaledVerSpace = Scale * VerSpaceF
		  BottomY        = ScaledMyY + ScaledVerSpace
		  ScaledHorSpace = Scale * HalfHorSpaceF
	       in
		  {Canvas tk(crea polygon
			     ScaledMyX
			     ScaledMyY - ScaledWidth
			     ScaledMyX - ScaledHorSpace
			     ScaledMyY + ScaledVerSpace
			     ScaledMyX + ScaledHorSpace
			     ScaledMyY + ScaledVerSpace
			     o(tags:    q(Node Tree Actions)
			       fill:    case @isSolBelow then
					   case @choices>0 then
					      SuspendedColor
					   else EntailedColor end
					else
					   case @choices>0 then
					      PartialFailedColor
					   else FailedColor end
					end
			       width:   NodeBorderWidth
			       outline: LineColor))}
		  case @number of !False then true elseof N then
		     case Font==False then true else
			{Canvas tk(crea text
				   ScaledMyX
				   ScaledMyY +
				   (ScaledVerSpace - ScaledWidth) / 2.0
				   o(font: Font
				     text: N
				     tags: q(Node Actions Tree
					     Canvas.numbers)))}
		     end
		  end
	       else
		  {Canvas tk(crea oval
			     ScaledMyX - ScaledWidth
			     ScaledMyY - ScaledWidth
			     ScaledMyX + ScaledWidth
			     ScaledMyY + ScaledWidth
			     case @toDo==nil then TkCreaTermOpt
			     else TkCreaOpt
			     end
			     o(tags: q(Node Tree Actions)))}
		  case @number of !False then true elseof N then
		     case Font==False then true else
			{Canvas tk(crea text ScaledMyX ScaledMyY
				   o(font: Font
				     text: N
				     tags: q(Node Tree Actions
					     Canvas.numbers)))}
		     end
		  end
		  <<ChooseNode DrawKids(@kids Break Tree MyX MyY+VerSpaceI
					Scale Font)>>
	       end
	       {Canvas tk(addtag MomTree withtag Tree)}
	       isDrawn <- True
	       isDirty <- False
	    else
	       case @kids\=nil then
		  isHidden <- True
		  <<ChooseNode purge>>
	       end
	       {Break broken(self)}
	    end
	 end
	 
	 meth !GetCenterAbove(X1 Y1 A1 ?X2 ?Y2 ?A2)
	    case self.mom of !False then
	       X2 = X1 + RootX
	       Y2 = Y1 + RootY
	       A2 = TreePrefix#self.suffix|A1
	    elseof Mom then
	       {Mom GetCenterAbove(X1+@offset Y1+VerSpaceI
				   TreePrefix#self.suffix|A1
				   ?X2 ?Y2 ?A2)}
	    end
	 end

	 meth deleteTree
	    Suffix = self.suffix
	 in
	    {self.canvas case self.mom of !False then
			    tk(delete TreePrefix#Suffix NodePrefix#Suffix)
			 else
			    tk(delete TreePrefix#Suffix NodePrefix#Suffix
			       LinkPrefix#Suffix)
			 end}
	 end
	 
      end
   end

   local

      class LeafNode
	 from TkNode

	 meth !GetCenterAbove(X1 Y1 A1 ?X2 ?Y2 ?A2)
	    case self.mom of !False then
	       X2 = X1 + RootX
	       Y2 = Y1 + RootY
	       A2 = A1
	    elseof Mom then
	       {Mom GetCenterAbove(X1+@offset Y1+VerSpaceI A1 ?X2 ?Y2 ?A2)}
	    end
	 end
	 
	 meth purge
	    offset <- 0
	 end
	 
	 meth deleteTree
	    Suffix = self.suffix
	 in
	    {self.canvas tk(delete NodePrefix#Suffix LinkPrefix#Suffix)}
	 end      
	 
      end

      
      TkFailedOpt   = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#FailedColor#
			  ' -width '#TermNodeBorderWidth#
			  ' -outline '#LineColor}})
      TkBlockedOpt = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#BlockedColor#
			  ' -width '#TermNodeBorderWidth#
			  ' -outline '#LineColor}})
      TkEntailedOpt = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#EntailedColor#
			  ' -width '#TermNodeBorderWidth#
			  ' -outline '#LineColor}})
      TkSuspendedOpt   = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#SuspendedColor#
			  ' -width '#TermNodeBorderWidth#
			  ' -outline '#LineColor}})

   in

      class FailedNode
	 from LeafNode
	 
	 meth drawTree(Break MomTree MomX MyY Scale Font)
	    NewOffset   = @offset.1
	    MyX         = MomX + NewOffset
	    ScaledWidth = Scale * RectangleWidthF
	    ScaledMomX  = Scale * {IntToFloat MomX}
	    ScaledMyX   = Scale * {IntToFloat MyX}
	    ScaledMyY   = Scale * {IntToFloat MyY}
	    Canvas      = self.canvas
	    Suffix      = self.suffix
	    Node        = !NodePrefix#Suffix
	    Tree        = !TreePrefix#Suffix
	 in
	    offset <- NewOffset
	    case self.mom of !False then true else
	       {Canvas tk(crea line
			  ScaledMomX
			  ScaledMyY - Scale * UpperSpaceF
			  ScaledMyX
			  ScaledMyY - ScaledWidth
			  o(tag: q(LinkPrefix#Suffix MomTree)
			    width: LinkWidth))}
	    end
	    {Canvas tk(crea rectangle
		       ScaledMyX - ScaledWidth
		       ScaledMyY - ScaledWidth
		       ScaledMyX + ScaledWidth
		       ScaledMyY + ScaledWidth
		       TkFailedOpt
		       o(tags: q(Node Tree MomTree)))}
	    isDrawn <- True
	 end
	 
	 meth moveNode(MomX MyX MyByX MyY Scale)
	    Canvas = self.canvas
	    Suffix = self.suffix
	 in
	    case self.mom of !False then true else
	       {Canvas tk(coords LinkPrefix#Suffix
			  Scale*{IntToFloat MomX}
			  Scale*{IntToFloat (MyY - UpperSpaceI)}
			  Scale*{IntToFloat MyX}
			  Scale*{IntToFloat (MyY - RectangleWidthI)})}
	    end
	    {Canvas tk(move NodePrefix#Suffix Scale*{IntToFloat MyByX} 0)}
	 end
      
      end

      class BlockedNode
	 from LeafNode
	 
	 meth drawTree(Break MomTree MomX MyY Scale Font)
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
	    Suffix          = self.suffix
	    Node            = !NodePrefix#Suffix
	    Tree            = !TreePrefix#Suffix
	 in
	    offset <- NewOffset
	    case self.mom of !False then true else
	       {Canvas tk(crea line
			  ScaledMomX
			  ScaledMyY - Scale * UpperSpaceF
			  ScaledMyX
			  ScaledMyY - ScaledHalfWidth
			  o(tag:   q(LinkPrefix#Suffix MomTree)
			    width: LinkWidth))}
	    end
	    {Canvas tk(crea polygon
		       X0 Y0 X2 Y1 X4 Y0 X3 Y2 X4 Y4 X2 Y3 X0 Y4 X1 Y2
		       TkBlockedOpt
		       o(tags: q(Node Tree)))}
	    isDrawn <- True
	 end
	 
	 meth moveNode(MomX MyX MyByX MyY Scale)
	    Canvas = self.canvas
	    Suffix = self.suffix
	 in
	    case self.mom of !False then true else
	       {Canvas tk(coords LinkPrefix#Suffix
			  Scale*{IntToFloat MomX}
			  Scale*{IntToFloat (MyY - UpperSpaceI)}
			  Scale*{IntToFloat MyX}
			  Scale*{IntToFloat (MyY - RectangleWidthI)})}
	    end
	    {Canvas tk(move NodePrefix#Suffix Scale*{IntToFloat MyByX} 0)}
	 end
      
      end

      local
	 class SucceededNode
	    from LeafNode NumberNode
	    
	    meth drawTree(MomTree MomX MyY Scale Font TkOpt)
	       NewOffset   = @offset.1
	       MyX         = MomX + NewOffset
	       ScaledWidth = Scale * RhombeWidthF
	       ScaledMomX  = Scale * {IntToFloat MomX}
	       ScaledMyX   = Scale * {IntToFloat MyX}
	       ScaledMyY   = Scale * {IntToFloat MyY}
	       Canvas      = self.canvas
	       Suffix      = self.suffix
	       Node        = !NodePrefix#Suffix
	       Tree        = !TreePrefix#Suffix
	       Actions     = Canvas.actions
	       X0          = ScaledMyX - ScaledWidth
	       X1          = ScaledMyX
	       X2          = ScaledMyX + ScaledWidth
	       Y0          = ScaledMyY - ScaledWidth
	       Y1          = ScaledMyY
	       Y2          = ScaledMyY + ScaledWidth
	    in
	       offset <- NewOffset
	       case self.mom of !False then true else
		  {Canvas tk(crea line
			     ScaledMomX
			     ScaledMyY - Scale * UpperSpaceF
			     ScaledMyX
			     ScaledMyY - ScaledWidth
			     o(tag:   q(LinkPrefix#Suffix MomTree)
			       width: LinkWidth))}
	       end
	       {Canvas tk(crea polygon X0 Y1 X1 Y0 X2 Y1 X1 Y2 X0 Y1
			  TkOpt
			  o(tags: q(Node Tree MomTree Actions)))}
	       case @number of !False then true elseof N then
		  case Font==False then true else
		     {Canvas tk(crea text ScaledMyX ScaledMyY
				o(font: Font
				  text: N
				  tags: q(Node Tree MomTree Actions
					  Canvas.numbers)))}
		  end
	       end
	       isDrawn <- True
	    end

	    meth moveNode(MomX MyX MyByX MyY Scale)
	       Canvas = self.canvas
	       Suffix = self.suffix
	    in
	       case self.mom of !False then true else
		  {Canvas tk(coords LinkPrefix#Suffix
			     Scale*{IntToFloat MomX}
			     Scale*{IntToFloat (MyY - UpperSpaceI)}
			     Scale*{IntToFloat MyX}
			     Scale*{IntToFloat (MyY - RhombeWidthI)})}
	       end
	       {Canvas tk(move NodePrefix#Suffix Scale*{IntToFloat MyByX} 0)}
	    end
	    
	 end

      in
	 
	 class EntailedNode
	    from SucceededNode
	    
	    meth drawTree(Break MomTree MomX MyY Scale Font)
	       <<SucceededNode drawTree(MomTree MomX MyY Scale Font
					TkEntailedOpt)>>
	    end
	    
	 end

   
	 class SuspendedNode
	    from SucceededNode

	    meth drawTree(Break MomTree MomX MyY Scale Font)
	       <<SucceededNode drawTree(MomTree MomX MyY Scale Font
					TkSuspendedOpt)>>
	    end
	    
	 end

      end

   end
   
in

   TkNodes=c(choose:    ChooseNode
	     failed:    FailedNode
	     blocked:   BlockedNode
	     suspended: SuspendedNode
	     entailed:  EntailedNode)

end








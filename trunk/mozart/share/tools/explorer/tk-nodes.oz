%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   GetCenterAbove = {NewName}
   UpperSpace     = VerSpace - CircleWidth
   TagEnd         = v(']')

   class NumberNode
      attr number: False

      meth getNumber(Scale Font TakeNumber ?N)
	 case @number==False then
	    Canvas    = self.canvas
	    NumberTag = Canvas.manager.numbers
	    X Y Above
	 in
	    N = TakeNumber
	    number <- TakeNumber
	    <<getCenterAbove(X Y Above)>>
	    case Font==False then true else
	       {Canvas tk(crea text
			  Scale * X
			  Scale * case <<isHidden($)>> then
				     Y + UpperSpace / 2.0
				  else Y
				  end
			  o(font: Font
			    text: N
			    tags: q(NodePrefix#self.suffix
				    Canvas.actionTag NumberTag
				    b(Above))))}
	    end
	 else
	    N = @number
	 end
      end

      meth redrawNumber(Scale Font)
	 Canvas    = self.canvas
	 NumberTag = Canvas.manager.numbers
	 X Y Above
      in
	 <<getCenterAbove(X Y Above)>>
	 {Canvas tk(crea text
		    Scale * X
		    Scale * case <<isHidden($)>> then Y + UpperSpace / 2.0
			    else Y
			    end
		    o(font: Font
		      text: @number
		      tags: q(NodePrefix#self.suffix
			      Canvas.actionTag NumberTag b(Above))))}
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
	 <<GetCenterAbove(0.0 0.0 nil ?X ?Y ?Above)>>
      end

      meth GetCenter(X1 Y1 ?X2 ?Y2)
	 case self.mom of !False then X2=X1+RootX Y2=Y1+RootY
	 elseof Mom then {Mom GetCenter(X1+@offset Y1+VerSpace ?X2 ?Y2)}
	 end
      end
	    
      meth getCenter(?X ?Y)
	 <<TkNode GetCenter(0.0 0.0 ?X ?Y)>>
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
			  '-fill '#ChoiceTermColor#
			  ' -width '#TermNodeBorderWidth}})
      TkCreaTermOpt = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#ChoiceTermColor#
			  ' -width '#TermNodeBorderWidth}})
      TkCreaOpt     = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#ChoiceColor#
			  ' -width '#NodeBorderWidth}})

      proc {Purge Ks}
	 case Ks of nil then true
	 [] K|Kr then {K purge} {Purge Kr}
	 end
      end
      
   in
      class ChoiceNode
	 from TkNode NumberNode
	 	 
	 meth moveNode(MomX MyX MyByX MyY Scale)
	    Canvas = self.canvas
	    Suffix = self.suffix
	    Node   = !NodePrefix#Suffix
	 in
	    case self.mom of !False then true else
	       {Canvas tk(coords LinkPrefix#Suffix
			  Scale*MomX  Scale*(MyY - UpperSpace)
			  Scale*MyX   Scale*(MyY - CircleWidth))}
	    end
	    {Canvas tk(move Node Scale*MyByX 0)}
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
			  Scale*MomX  Scale*(MyY - UpperSpace)
			  Scale*MyX   Scale*(MyY - RectangleWidth))}
	    end
	    {Canvas tk(move TreePrefix#Suffix Scale*MyByX 0)}
	 end

      
	 meth DrawKids(Ks Break MomX MyY Scale Font)
	    case Ks of nil then true
	    [] K|Kr then
	       {K drawTree(Break MomX MyY Scale Font)}
	       <<DrawKids(Kr Break MomX MyY Scale Font)>>
	    end
	 end
	 
	 meth purge
	    offset <- 0.0
	    shape  <- nil
	    {Purge @kids}
	 end
	 
	 meth drawTree(Break MomX MyY Scale Font)
	    case {System.isVar Break} then
	       NewOffset   = @offset.1
	       MyX         = MomX + NewOffset
	       ScaledWidth = Scale * CircleWidth
	       ScaledMomX  = Scale * MomX
	       ScaledMyX   = Scale * MyX
	       ScaledMyY   = Scale * MyY
	       Canvas      = self.canvas
	       Suffix      = self.suffix
	       Node        = !NodePrefix#Suffix
	    in
	       offset <- NewOffset
	       case self.mom of !False then true else
		  {Canvas tk(crea line
			     ScaledMomX
			     ScaledMyY - Scale * UpperSpace
			     ScaledMyX
			     ScaledMyY - ScaledWidth
			     o(tag: o(Canvas.tagsVar LinkPrefix#Suffix
				      TagEnd)
			       width: LinkWidth))}
	       end
	       {Canvas.addTag TreePrefix#Suffix}
	       case @isHidden then
		  ScaledVerSpace = Scale * VerSpace
		  BottomY        = ScaledMyY + ScaledVerSpace
		  ScaledHorSpace = Scale * HalfHorSpace
	       in
		  {Canvas tk(crea polygon
			     ScaledMyX
			     ScaledMyY - ScaledWidth
			     ScaledMyX - ScaledHorSpace
			     ScaledMyY + ScaledVerSpace
			     ScaledMyX + ScaledHorSpace
			     ScaledMyY + ScaledVerSpace
			     o(tags:  o(Canvas.tagsVar Node Canvas.actionTag
					TagEnd)
			       fill:    case @isSolBelow then
					   case @choices>0 then StableColor
					   else EntailedColor end
					else
					   case @choices>0 then PartialFailedColor
					   else FailedColor end
					end
			       width:   NodeBorderWidth
			       outline: LineColor))}
		  case @number of !False then true elseof N then
		     case Font==False then true else
			{Canvas tk(crea text
				   ScaledMyX
				   ScaledMyY + (ScaledVerSpace - ScaledWidth) / 2.0
				   o(font: Font
				     text: N
				     tags: o(Canvas.tagsVar
					     Node Canvas.actionTag
					     Canvas.manager.numbers
					     TagEnd)))}
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
			     o(tags: o(Canvas.tagsVar
				       Node Canvas.actionTag TagEnd)))}
		  case @number of !False then true elseof N then
		     case Font==False then true else
			{Canvas tk(crea text ScaledMyX ScaledMyY
				   o(font: Font
				     text: N
				     tags: o(Canvas.tagsVar
					     Node Canvas.actionTag
					     Canvas.manager.numbers
					     TagEnd)))}
		     end
		  end
		  <<ChoiceNode DrawKids(@kids Break MyX MyY+VerSpace
					Scale Font)>>
	       end
	       {Canvas.skipTag}
	       isDrawn <- True
	       isDirty <- False
	    else
	       case @kids\=nil then
		  isHidden <- True
		  <<ChoiceNode purge>>
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
	       {Mom GetCenterAbove(X1+@offset Y1+VerSpace
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
	       {Mom GetCenterAbove(X1+@offset Y1+VerSpace A1 ?X2 ?Y2 ?A2)}
	    end
	 end
	 
	 meth purge
	    offset <- 0.0
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
      TkUnstableOpt = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#UnstableColor#
			  ' -width '#TermNodeBorderWidth#
			  ' -outline '#LineColor}})
      TkEntailedOpt = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#EntailedColor#
			  ' -width '#TermNodeBorderWidth#
			  ' -outline '#LineColor}})
      TkStableOpt   = v({String.toAtom
			 {VirtualString.toString
			  '-fill '#StableColor#
			  ' -width '#TermNodeBorderWidth#
			  ' -outline '#LineColor}})

   in

      class FailedNode
	 from LeafNode
	 
	 meth drawTree(Break MomX MyY Scale Font)
	    NewOffset   = @offset.1
	    MyX         = MomX + NewOffset
	    ScaledWidth = Scale * RectangleWidth
	    ScaledMomX  = Scale * MomX
	    ScaledMyX   = Scale * MyX
	    ScaledMyY   = Scale * MyY
	    Canvas      = self.canvas
	    Suffix      = self.suffix
	    Node        = !NodePrefix#Suffix
	 in
	    offset <- NewOffset
	    case self.mom of !False then true else
	       {Canvas tk(crea line
			  ScaledMomX
			  ScaledMyY - Scale * UpperSpace
			  ScaledMyX
			  ScaledMyY - ScaledWidth
			  o(tag: o(Canvas.tagsVar LinkPrefix#Suffix TagEnd)
			    width: LinkWidth))}
	    end
	    {Canvas tk(crea rectangle
		       ScaledMyX - ScaledWidth
		       ScaledMyY - ScaledWidth
		       ScaledMyX + ScaledWidth
		       ScaledMyY + ScaledWidth
		       TkFailedOpt
		       o(tags: o(Canvas.tagsVar Node TagEnd))
		      )}
	    isDrawn <- True
	 end
	 
	 meth moveNode(MomX MyX MyByX MyY Scale)
	    Canvas = self.canvas
	    Suffix = self.suffix
	 in
	    case self.mom of !False then true else
	       {Canvas tk(coords LinkPrefix#Suffix
			  Scale*MomX  Scale*(MyY - UpperSpace)
			  Scale*MyX   Scale*(MyY - RectangleWidth))}
	    end
	    {Canvas tk(move NodePrefix#Suffix Scale*MyByX 0)}
	 end
      
      end

      class UnstableNode
	 from LeafNode
	 
	 meth drawTree(Break MomX MyY Scale Font)
	    NewOffset       = @offset.1
	    MyX             = MomX + NewOffset
	    ScaledHalfWidth = Scale * SmallRectangleWidth
	    ScaledFullWidth = Scale * RectangleWidth
	    ScaledMomX      = Scale * MomX
	    ScaledMyX   = Scale * MyX
	    ScaledMyY   = Scale * MyY
	    X0          = ScaledMyX - ScaledFullWidth
	    X1          = ScaledMyX - ScaledHalfWidth
	    X2          = ScaledMyX
	    X3          = ScaledMyX + ScaledHalfWidth
	    X4          = ScaledMyX + ScaledFullWidth
	    Y0          = ScaledMyY - ScaledFullWidth
	    Y1          = ScaledMyY - ScaledHalfWidth
	    Y2          = ScaledMyY
	    Y3          = ScaledMyY + ScaledHalfWidth
	    Y4          = ScaledMyY + ScaledFullWidth
	    Canvas      = self.canvas
	    Suffix      = self.suffix
	    Node        = !NodePrefix#Suffix
	 in
	    offset <- NewOffset
	    case self.mom of !False then true else
	       {Canvas tk(crea line
			  ScaledMomX
			  ScaledMyY - Scale * UpperSpace
			  ScaledMyX
			  ScaledMyY - ScaledHalfWidth
			  o(tag: o(Canvas.tagsVar LinkPrefix#Suffix TagEnd)
			    width: LinkWidth))}
	    end
	    {Canvas tk(crea polygon
		       X0 Y0 X2 Y1 X4 Y0 X3 Y2 X4 Y4 X2 Y3 X0 Y4 X1 Y2
		       TkUnstableOpt
		       o(tags: o(Canvas.tagsVar Node TagEnd))
		      )}
	    isDrawn <- True
	 end
	 
	 meth moveNode(MomX MyX MyByX MyY Scale)
	    Canvas = self.canvas
	    Suffix = self.suffix
	 in
	    case self.mom of !False then true else
	       {Canvas tk(coords LinkPrefix#Suffix
			  Scale*MomX  Scale*(MyY - UpperSpace)
			  Scale*MyX   Scale*(MyY - RectangleWidth))}
	    end
	    {Canvas tk(move NodePrefix#Suffix Scale*MyByX 0)}
	 end
      
      end

      local
	 class SolvedNode
	    from LeafNode NumberNode
	    
	    meth drawTree(MomX MyY Scale Font TkOpt)
	       NewOffset   = @offset.1
	       MyX         = MomX + NewOffset
	       ScaledWidth = Scale * RhombeWidth
	       ScaledMomX  = Scale * MomX
	       ScaledMyX   = Scale * MyX
	       ScaledMyY   = Scale * MyY
	       Canvas      = self.canvas
	       Suffix      = self.suffix
	       Node        = !NodePrefix#Suffix
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
			     ScaledMyY - Scale * UpperSpace
			     ScaledMyX
			     ScaledMyY - ScaledWidth
			     o(tag: o(Canvas.tagsVar LinkPrefix#Suffix TagEnd)
			       width: LinkWidth))}
	       end
	       {Canvas tk(crea polygon X0 Y1 X1 Y0 X2 Y1 X1 Y2 X0 Y1
			  TkOpt
			  o(tags: o(Canvas.tagsVar Node
				    Canvas.actionTag TagEnd)))}
	       case @number of !False then true elseof N then
		  case Font==False then true else
		     {Canvas tk(crea text ScaledMyX ScaledMyY
				o(font: Font
				  text: N
				  tags: o(Canvas.tagsVar
					  Node Canvas.actionTag
					  Canvas.manager.numbers
					  TagEnd)))}
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
			     Scale*MomX  Scale*(MyY - UpperSpace)
			     Scale*MyX   Scale*(MyY - RhombeWidth))}
	       end
	       {Canvas tk(move NodePrefix#Suffix Scale*MyByX 0)}
	    end
	    
	 end

      in
	 
	 class EntailedNode
	    from SolvedNode
	    
	    meth drawTree(Break MomX MyY Scale Font)
	       <<SolvedNode drawTree(MomX MyY Scale Font TkEntailedOpt)>>
	    end
	    
	 end

   
	 class StableNode
	    from SolvedNode

	    meth drawTree(Break MomX MyY Scale Font)
	       <<SolvedNode drawTree(MomX MyY Scale Font TkStableOpt)>>
	    end
	    
	 end

      end

   end
   
in

   TkNodes=c(choice:   ChoiceNode
	     failed:   FailedNode
	     unstable: UnstableNode
	     stable:   StableNode
	     entailed: EntailedNode)

end








%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local
   DarkColor      = '#828282'
   BrightColor    = '#ffffff'
   BackColor      = '#d9d9d9'
   MarkFrameColor = black

   Home               = ~10000
   FreeMarkX          = 0
   NotebookFrameWidth = 3
   NotebookOffset     = 0
   NotebookDelta      = NotebookFrameWidth + NotebookOffset
   
   MarkOuterOffset    = 3
   MarkInnerOffset    = 2
   MarkFrameWidth     = 2
   MarkDelta          = (MarkOuterOffset + MarkFrameWidth + MarkInnerOffset)
   MarkEdgeWidth      = 2

   MoveTag     = {NewName}
   OutlineTag  = {NewName}
   Nothing     = {NewName}

   TestText    = {String.toAtom {Filter {List.number 0 255 1} Char.isGraph}}

   local
      fun {Part Is Js Jr}
	 case Is of nil then Jr=nil [{Tk.string.toInt Js}]
	 [] I|Ir then
	    case I==&  then NewJs in
	       Jr=nil {Tk.string.toInt Js}|{Part Ir NewJs NewJs}
	    else NewJr in
	       Jr=I|NewJr {Part Ir Js NewJr}
	    end
	 end
      end
   in
      fun {Partition Is}
	 Js in {Part Is Js Js}
      end
   end
   
in

   class Notebook
      from Tk.canvas
      feat
	 TextHeight
      attr
	 Width:            0
	 Height:           0
	 NextMarkX:        FreeMarkX
	 TopNote:          False

      meth tkInit(parent: Parent
		  height: IntHeight
		  width:  IntWidth
		  font:   Font      <= Nothing)
	 <<Tk.canvas tkInit(parent:             Parent
			    highlightthickness: 0)>>
	 TestTextTag = {New Tk.canvasTag tkInit(parent:self)}
	 <<Tk.canvas tk(crea text Home Home
			anchor:  sw
			justify: left
			text:    TestText
			tags:    TestTextTag)>>
	 FontHeight  = local
			  _|Y1|_|Y2|_ =
			  {Partition <<Tk.canvas
			               tkReturn(bbox(TestTextTag) $)>>}
		       in
			  Y2 - Y1
		       end
	 ExtWidth    = IntWidth   + 2 * NotebookDelta
	 MarkHeight  = FontHeight + 2 * MarkDelta
	 ExtHeight   = IntHeight  + 2 * NotebookDelta
      in
	 self.TextHeight = FontHeight
	 <<Tk.canvas tk(configure
			yscrollincrement: 1
			xscrollincrement: 1
			height:           ExtHeight + MarkHeight
			width:            ExtWidth)>>
	 <<Tk.canvas tk(yview scroll ~MarkHeight-2 units)>> 
	 <<Tk.canvas tk(xview scroll ~2 units)>> 
	 <<Tk.canvas tk(crea polygon
			0                        0
			(NotebookFrameWidth - 1) 0
			(NotebookFrameWidth - 1) (ExtHeight - 
						  NotebookFrameWidth)
			0                        (ExtHeight - 1)
			0                        0
			fill:BrightColor)>>
	 <<Tk.canvas tk(crea polygon
			1                        (ExtHeight - 1)
			(NotebookFrameWidth - 1) (ExtHeight - 1 -
						  NotebookFrameWidth)
			(ExtWidth - NotebookFrameWidth - 1)
			(ExtHeight - NotebookFrameWidth - 1)
			(ExtWidth - NotebookFrameWidth - 1)
			(NotebookFrameWidth - 1)
			ExtWidth-1                1
			ExtWidth-1                ExtHeight-1
			1 ExtHeight-1
			fill:DarkColor)>>
	 {TestTextTag close}
	 Width      <- IntWidth
	 Height     <- IntHeight
      end

      meth add(note:   Note
	       text:   Text
	       anchor: Anchor <= Nothing)
	 ThisEventTag   = {New Tk.canvasTag tkInit(parent:self)}
	 ThisTextTag    = {New Tk.canvasTag tkInit(parent:self)}
	 ThisMoveTag    = Note.MoveTag
	 ThisOutlineTag = Note.OutlineTag
	 CurMarkX       = @NextMarkX
	 <<Notebook ReplaceTop(Note)>>
	 <<Tk.canvas tk(crea window
			NotebookDelta - 1 NotebookDelta - 1
			anchor: nw
			width:  @Width
			height: @Height
			tags:   ThisMoveTag
			window: Note)>>
	 <<Tk.canvas tk(crea text
			(CurMarkX + MarkDelta + NotebookFrameWidth)
			2 - MarkDelta
			anchor:  sw
			justify: left
			text:    Text
			tags:    q(ThisTextTag ThisEventTag))>>
	 TextWidth = local
			X1|_|X2|_ = {Partition <<Tk.canvas
				     tkReturn(bbox(ThisTextTag) $)>>}
		     in
			X2 - X1
		     end
	 TotalMarkHeight = 2 * MarkDelta + self.TextHeight
	 TotalMarkWidth  = 2 * MarkDelta + TextWidth
	 %% X coordinates needed
	 X0 = 0
	 X1 = !CurMarkX
	 X2 = X1 + NotebookFrameWidth - 1
	 X3 = X2 + MarkEdgeWidth
	 X6 = X2 + TotalMarkWidth
	 X7 = X6 + NotebookFrameWidth - 1
	 X4 = X6 - MarkEdgeWidth
	 X5 = X4 + 1
	 X8 = NotebookFrameWidth + @Width
	 X9 = X8 + NotebookFrameWidth - 1
	 %% Y coordinates needed
	 Y0 = ~TotalMarkHeight - 1
	 Y1 = Y0 + NotebookFrameWidth 
	 Y2 = Y1 + MarkEdgeWidth
	 Y3 = 0
	 Y4 = 0
	 Y5 = NotebookFrameWidth - 1
      in
	 %% Draw frame which always stays
	 <<Tk.canvas tk(crea polygon
			X1 Y3 X1 Y2 X3 Y0 X4 Y0
			X4 Y1 X3 Y1 X2 Y2 X2 Y3 X1 Y3
			fill:BrightColor)>>
	 <<Tk.canvas tk(crea polygon
			X6 Y3 X7 Y3 X7 Y2 X4 Y0
			X4 Y1 X6 Y2 X6 Y3
			fill:DarkColor)>>
	 %% Draw transient part of frame
	 case CurMarkX==0 then true else
	    <<Tk.canvas tk(crea polygon
			   X0 Y4 X2 Y4 X2 Y5 X0 Y5 X0 Y4
			   fill:BrightColor tags:ThisMoveTag)>>
	 end
	 <<Tk.canvas tk(crea polygon
			X6 Y4 X7 Y4 X6 Y5 X6 Y4
			fill:DarkColor tags:ThisMoveTag)>>
	 <<Tk.canvas tk(crea polygon
			X6 Y5 X8 Y5 X9 Y4 X7 Y4 X6 Y5
			fill:BrightColor tags:ThisMoveTag)>>
	 NextMarkX <- X7+1
	 %% Draw frame around text
	 <<Tk.canvas tk(crea rectangle
			X2 + MarkOuterOffset
			MarkFrameWidth - MarkOuterOffset 
			X6 - MarkOuterOffset
			Y1 + MarkOuterOffset 
			fill:    BackColor
			outline: black
			width:   MarkFrameWidth
			tags:    q(ThisOutlineTag ThisEventTag))>>
	 <<Tk.canvas tk(raise ThisTextTag ThisOutlineTag)>>
	 {ThisEventTag tkBind(event:  '<1>'
			      action: self # toTop(Note))}
      end

      meth ReplaceTop(Note)
	 Top = @TopNote
      in
	 case {IsObject Top} then
	    {Top.MoveTag     tk(move Home Home)}
	    {Top.OutlineTag tk(itemconf outline:BackColor)}
	 else true
	 end
	 TopNote <- Note
      end
      
      meth toTop(Note)
	 <<Notebook ReplaceTop(Note)>>
	 {Note.MoveTag    tk(move ~Home ~Home)}
	 {Note.OutlineTag tk(itemconf outline:MarkFrameColor)}
      end

      meth getTop($)
	 @TopNote
      end
   end

   
   class Note
      from Tk.frame
      feat !MoveTag !OutlineTag
      meth tkInit(parent: Parent
		  font:   Font  <= Nothing)
	 <<Tk.frame tkInit(parent:             Parent
			   highlightthickness: 0)>>
	 self.MoveTag    = {New Tk.canvasTag tkInit(parent: Parent)}
	 self.OutlineTag = {New Tk.canvasTag tkInit(parent: Parent)}
      end
      
   end
	 
end

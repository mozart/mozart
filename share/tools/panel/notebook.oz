%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local
   DarkColor   #
   BrightColor  = case Tk.isColor then '#828282' # '#ffffff'
		  else black # black
		  end

   fun {TclGetConf T Opt}
      l(lindex(l(T conf '-'#Opt) 4))
   end

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

   NoteBorderTag      = {NewName} % horizontal part of 3D-border
   MarkBorderTag      = {NewName} % mark part of 3D-border
   HighlightTag       = {NewName} % highlightframe
   TextTag            = {NewName} % text
   EventTag           = {NewName} % both text and highlightframe
   NoteTag            = {NewName} % note widget
   TextWidth          = {NewName} % text width
   
   Nothing            = {NewName}

   TestText           = {String.toAtom {Filter {List.number 0 255 1}
					Char.isGraph}}

   fun {GetNote Ns M ?Os}
      case Ns of nil then Os=nil False
      [] N|Nr then
	 case N.3==M then Os=Nr N
	 else Or in Os=N|Or {GetNote Nr M Or}
	 end
      end
   end

in

   class Notebook
      from Tk.canvas
      feat
	 TextHeight
	 StaticTag
      attr
	 Width:   0
	 Height:  0
	 NextX:   FreeMarkX
	 TopNote: Unit
	 Notes:   nil

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
			  <<Tk.canvas tkReturnListInt(bbox(TestTextTag) $)>>
		       in
			  Y2 - Y1
		       end
	 MarkHeight  = FontHeight + 2 * MarkDelta
      in
	 <<Notebook tk(configure
		       yscrollincrement: 1
		       xscrollincrement: 1)>>
	 <<Notebook tk(yview scroll ~MarkHeight-2 units)>> 
	 <<Notebook tk(xview scroll ~2 units)>> 
	 self.TextHeight = FontHeight
	 self.StaticTag  = {New Tk.canvasTag tkInit(parent:self)}
	 <<Notebook ResizeStaticFrame(IntWidth IntHeight)>>
	 {TestTextTag close}
      end

      meth ResizeStaticFrame(IntWidth IntHeight)
	 ExtWidth    = IntWidth   + 2 * NotebookDelta
	 MarkHeight  = self.TextHeight + 2 * MarkDelta
	 ExtHeight   = IntHeight  + 2 * NotebookDelta
	 Static      = self.StaticTag
      in
	 {Static tk(delete)}
	 <<Notebook tk(configure
		       height:           ExtHeight + MarkHeight
		       width:            ExtWidth)>>
	 <<Notebook tk(crea polygon
		       0                        0
		       (NotebookFrameWidth - 1) 0
		       (NotebookFrameWidth - 1) (ExtHeight - 
						 NotebookFrameWidth)
		       0                        (ExtHeight - 1)
		       0                        0
		       fill:BrightColor
		       tags:Static)>>
	 <<Notebook tk(crea polygon
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
		       fill:DarkColor
		       tags:Static)>>
	 Width      <- IntWidth
	 Height     <- IntHeight
      end

      meth DrawNote(DoMark NotActive Note X W $)
	 ThisNoteBorderTag = Note.NoteBorderTag
	 ThisMarkBorderTag = Note.MarkBorderTag
	 ThisHighlightTag  = Note.HighlightTag
	 ThisTextTag       = Note.TextTag
	 ThisEventTag      = Note.EventTag
	 ThisNoteTag       = Note.NoteTag
	 ThisTextWidth     = Note.TextWidth
	 TotalMarkHeight   = 2 * MarkDelta + self.TextHeight
	 TotalMarkWidth    = 2 * MarkDelta + ThisTextWidth
	 %% X coordinates needed
	 X0 = 0
	 X1 = !X
	 X2 = X1 + NotebookFrameWidth - 1
	 X3 = X2 + MarkEdgeWidth
	 X6 = X2 + TotalMarkWidth
	 X7 = X6 + NotebookFrameWidth - 1
	 X4 = X6 - MarkEdgeWidth
	 X5 = X4 + 1
	 X8 = NotebookFrameWidth + W
	 X9 = X8 + NotebookFrameWidth - 1
	 %% Y coordinates needed
	 Y0 = ~TotalMarkHeight - 1
	 Y1 = Y0 + NotebookFrameWidth 
	 Y2 = Y1 + MarkEdgeWidth
	 Y3 = 0
	 Y4 = 0
	 Y5 = NotebookFrameWidth - 1
	 AO = case NotActive then Home else 0 end
      in
	 %% Draw note frame
	 case X==0 then true else
	    <<Notebook tk(crea polygon
			  AO+X0 AO+Y4 AO+X2 AO+Y4 AO+X2 AO+Y5
			  AO+X0 AO+Y5 AO+X0 AO+Y4
			  fill: BrightColor
			  tags: ThisNoteBorderTag)>>
	 end
	 <<Notebook tk(crea polygon
		       AO+X6 AO+Y4 AO+X7 AO+Y4 AO+X6 AO+Y5 AO+X6 AO+Y4
		       fill: DarkColor
		       tags: ThisNoteBorderTag)>>
	 <<Notebook tk(crea polygon
		       AO+X6 AO+Y5 AO+X8 AO+Y5 AO+X9 AO+Y4
		       AO+X7 AO+Y4 AO+X6 AO+Y5
		       fill: BrightColor
		       tags: ThisNoteBorderTag)>>
	 case DoMark then
	    %% Make text visible
	    {ThisTextTag tk(coords X+MarkDelta+NotebookFrameWidth 2-MarkDelta)}
	    %% Draw mark frame
	    <<Notebook tk(crea polygon
			  X1 Y3 X1 Y2 X3 Y0 X4 Y0 X4 Y1 X3 Y1 X2 Y2 X2 Y3 X1 Y3
			  fill: BrightColor
			  tags: ThisMarkBorderTag)>>
	    <<Notebook tk(crea polygon
			  X6 Y3 X7 Y3 X7 Y2 X4 Y0 X4 Y1 X6 Y2 X6 Y3
			  fill: DarkColor
			  tags: ThisMarkBorderTag)>>
	    %% Draw frame around text
	    <<Notebook tk(crea rectangle
			  X2 + MarkOuterOffset
			  MarkFrameWidth - MarkOuterOffset 
			  X6 - MarkOuterOffset
			  Y1 + MarkOuterOffset 
			  fill:    {TclGetConf self bg}
			  outline: case NotActive then {TclGetConf self bg}
				   else black
				   end
			  width:   MarkFrameWidth
			  tags:    q(ThisHighlightTag ThisEventTag))>>
	    <<Notebook tk(raise ThisTextTag ThisHighlightTag)>>
	    {ThisEventTag tkBind(event:  '<1>'
				 action: self # toTop(Note))}
	 else true
	 end
	 %% The next X coordinate
	 X7 + 1
      end

      meth add(note:Note after:After <=Nothing)
	 NotFirstNote = {IsObject @TopNote}
	 OldX = @NextX
	 NewX = <<Notebook DrawNote(True NotFirstNote Note OldX @Width $)>>
      in
	 NextX <- NewX
	 case NotFirstNote then true else
	    {Note.NoteTag tk(move ~Home ~Home)}
	    TopNote <- Note
	 end
	 Notes <- {Append @Notes [OldX#NewX-OldX#Note]}
      end

      meth MoveNotes(Ns X ByX)
	 case Ns of nil then true
	 [] N|Nr then
	    case N.1<X then true else Note = N.3 in
	       {Note.NoteBorderTag  tk(delete)}
	       {Note.MarkBorderTag  tk(move ByX 0)}
	       {Note.HighlightTag   tk(move ByX 0)}
	       {Note.TextTag        tk(move ByX 0)}
	       <<Notebook DrawNote(False Note\=@TopNote Note X @Width _)>>
	    end
	    <<Notebook MoveNotes(Nr X ByX)>>
	 end
      end
      
      meth remove(note:Note)
	 NewNotes
	 X#W#_ = {GetNote @Notes Note NewNotes}
      in
	 case @TopNote==Note then
	    case NewNotes==nil then
	       TopNote <- Unit
	    else
	       NewTop = NewNotes.1.3
	    in
	       TopNote <- NewTop
	       <<Notebook UnhideNote(NewTop)>>
	    end
	 else true
	 end
	 {Note.HighlightTag  tk(dtag Note.EventTag)}
	 {Note.NoteBorderTag tk(delete)}
	 {Note.MarkBorderTag tk(delete)}
	 {Note.HighlightTag  tk(delete)}
	 {Note.NoteTag       tk(coords
				Home + NotebookDelta - 1
				Home + NotebookDelta - 1)}
	 {Note.TextTag       tk(coords Home Home)}
	 <<Notebook MoveNotes(NewNotes X ~W)>>
	 Notes <- NewNotes
	 NextX <- @NextX - W
      end

      meth HideNote(Note)
	 {Note.NoteTag       tk(move Home Home)}
	 {Note.NoteBorderTag tk(move Home Home)}
	 {Note.HighlightTag  tk(itemconf outline:{TclGetConf self bg})}
      end

      meth UnhideNote(Note)
	 {Note.NoteTag       tk(move ~Home ~Home)}
	 {Note.NoteBorderTag tk(move ~Home ~Home)}
	 {Note.HighlightTag  tk(itemconf outline:black)}
      end
      
      meth toTop(Note)
	 Top = @TopNote
      in
	 case Note==Top then true else
	    <<Notebook HideNote(Top)>>
	    <<Notebook UnhideNote(Note)>>
	    TopNote <- Note
	 end
	 {Note toTop}
      end

      meth confHeight(H)
	 <<Notebook ResizeStaticFrame(@Width H)>>
      end
      
      meth getTop($)
	 @TopNote
      end
   end

   
   class Note
      from Tk.frame
      feat
	 !NoteBorderTag    % horizontal part of 3D-border
	 !MarkBorderTag    % mark part of 3D-border
	 !HighlightTag     % highlightframe
	 !TextTag          % text
	 !EventTag         % both text and highlightframe
	 !NoteTag          % note widget
	 !TextWidth        % width of text

      meth tkInit(parent:Parent text:Text)
	 ThisTextTag   = {New Tk.canvasTag tkInit(parent:Parent)}
	 ThisEventTag  = {New Tk.canvasTag tkInit(parent:Parent)}
	 ThisNoteTag   = {New Tk.canvasTag tkInit(parent:Parent)}
      in
	 {Parent tk(crea text Home Home
		    text:   Text
		    anchor: sw
		    tags:   q(ThisTextTag ThisEventTag))}
	 self.TextWidth = local
			     X1|_|X2|_ =
			      {Parent tkReturnListInt(bbox(ThisTextTag) $)}
			  in X2 - X1
			  end
	 <<Tk.frame tkInit(parent:             Parent
			   highlightthickness: 0)>>
	 self.NoteBorderTag = {New Tk.canvasTag tkInit(parent:Parent)}
	 self.MarkBorderTag = {New Tk.canvasTag tkInit(parent:Parent)}
	 self.HighlightTag  = {New Tk.canvasTag tkInit(parent:Parent)}
	 self.TextTag       = ThisTextTag
	 self.EventTag      = ThisEventTag
	 self.NoteTag       = ThisNoteTag 
	 {Parent tk(crea window
		    Home + NotebookDelta - 1 Home + NotebookDelta - 1
		    anchor: nw
		    tags:   ThisNoteTag
		    window: self)}
      end

      meth toTop
	 true
      end
   end
	 
end

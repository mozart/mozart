%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1997, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%
   
local
   
   HideY      = 1000
   Rows       = 2
   Cols       = MaxDur div Rows
   CellSize   = 20
   CellBorder = 1
   CellFrame  = 1
   CellDelta  = CellBorder + CellFrame
   
in

   class DurationTool
      from TkTools.textframe
      feat
	 Canvas
	 DisableTag
	 Action
      attr
	 Selected: unit
	 Enabled:  false
	 Duration: 1
      meth tkInit(parent:P action:A)
	 TkTools.textframe, tkInit(parent: P
				   font:   Helv
				   text:   'Duration')
	 CA = {New Tk.canvas tkInit(parent:     self.inner
				    width:      CellSize * Cols + 2
				    height:     CellSize * Rows + 2
				    xscrollinc: 1
				    yscrollinc: 1)}
	 DT = {New Tk.canvasTag tkInit(parent:CA)}
      in
	 {CA tk(yview scroll ~2 units)}
	 {CA tk(xview scroll ~2 units)}
	 {DT tkBind(event:'<1>' action:self#enable)}
	 {For 1 Rows 1
	  proc {$ R}
	     Y = (Rows - R) * CellSize
	  in
	     {For 1 Cols 1
	      proc {$ C}
		 X   = (Cols - C) * CellSize
		 Dur = (Rows-R)*Cols + Cols-C+1
		 T1  = {New Tk.canvasTag tkInit(parent:CA)}
		 T2  = {New Tk.canvasTag tkInit(parent:CA)}
	      in
		 case Dur>MaxDur then skip else 
		    {CA tk(create rectangle
			   X + CellDelta
			   Y + CellDelta
			   X + CellSize - CellDelta
			   Y + CellSize - CellDelta
			   width:   1
			   fill:    wheat
			   outline: white
			   tags:    q(T1 T2))}
		    {CA tk(create text
			   X + CellSize div 2
			   Y + CellSize div 2
			   text: Dur
			   font: SmallCourierBold
			   tags: T2)}
		    {CA tk(create rectangle
			   X + CellDelta
			   Y + CellDelta
			   X + CellSize - CellDelta
			   Y + CellSize - CellDelta
			   fill:    gray50
			   outline: ''
			   stipple: gray50
			   tags:    q(T1 T2 DT))}
		    {T2 tkBind(event:  '<1>'
			       action: self # Choose(Dur T1))}
		 end
		 case Dur==1 then Selected <- T1
		 else skip
		 end
	      end}
	  end}
	 self.DisableTag = DT
	 self.Canvas     = CA
	 self.Action     = A
	 DurationTool, Choose(1 @Selected)
	 {Tk.send pack(CA)}
      end
      
      meth Choose(Dur Tag)
	 {@Selected tk(itemconfigure width:1           outline:white)}
	 {Tag       tk(itemconfigure width:CellFrame+1 outline:black)}
	 Selected <- Tag
	 Duration <- Dur
      end
      
      meth getDur($)
	 @Duration
      end
      
      meth disable
	 case @Enabled then
	    {self.DisableTag tk(move 0 HideY)}
	    Enabled <- false
	 else skip
	 end
      end
      
      meth enable
	 case @Enabled then skip else
	    {self.DisableTag tk(move 0 ~HideY)}
	    Enabled <- true
	    {self.Action}
	 end
      end
      
   end
end

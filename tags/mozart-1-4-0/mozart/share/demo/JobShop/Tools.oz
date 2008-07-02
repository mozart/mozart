%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor

import
   TkTools
   Tk
   Explorer
   
   Configure(text:             Helv
	     typeSmall:        SmallCourierBold
	     resColors:        ResColors
	     maxDur:           MaxDur
	     maxRes:           MaxRes)
   
export
   'class': Tools
   
define

   local
      HideY      = 1000
      Rows       = 2
      Cols       = MaxRes div Rows
      CellSize   = 20
      CellBorder = 1
      CellFrame  = 1
      CellDelta  = CellBorder + CellFrame
   in
      class ResourceTool
	 from TkTools.textframe
	 feat
	    Canvas
	    DisableTag
	    Action
	 attr
	    Selected: unit
	    Enabled:  false
	    Resource: 1
	 meth tkInit(parent:P action:A)
	    TkTools.textframe, tkInit(parent:P font:Helv text:'Resource')
	    CA = {New Tk.canvas tkInit(parent:  self.inner
				       width:   CellSize * Cols + 2
				       height:  CellSize * Rows + 2
				       xscrollinc: 1
				       yscrollinc: 1)}
	    DT = {New Tk.canvasTag tkInit(parent:CA)}
	 in
	    {DT tkBind(event:'<1>' action:self#enable)}
	    {CA tk(yview scroll ~2 units)}
	    {CA tk(xview scroll ~2 units)}
	    {For 1 Rows 1
	     proc {$ R}
		Y = (Rows - R) * CellSize
	     in
		{For 1 Cols 1
		 proc {$ C}
		    X   = (Cols - C) * CellSize
		    Res = (R-1)*Cols + C
		    T   = {New Tk.canvasTag tkInit(parent:CA)}
		 in
		    {CA tk(create rectangle
			   X + CellDelta
			   Y + CellDelta
			   X + CellSize - CellDelta
			   Y + CellSize - CellDelta
			   fill:    ResColors.Res
			   width:   1
			   outline: white
			   tags:    T)}
		    {CA tk(create rectangle
			   X + CellDelta
			   Y + CellDelta
			   X + CellSize - CellDelta
			   Y + CellSize - CellDelta
			   fill:    gray50
			   outline: ''
			   stipple: gray50
			   tags:    q(T DT))}
		    {T tkBind(event:  '<1>'
			      action: self # Choose(Res T))}
		    if Res==MaxRes then Selected <- T
		    end
		 end}
	     end}
	    self.DisableTag = DT
	    self.Canvas     = CA
	    self.Action     = A
	    ResourceTool, Choose(MaxRes @Selected)
	    {Tk.send pack(CA)}
	 end
	       
	 meth Choose(Res Tag)
	    {@Selected tk(itemconfigure width:1           outline:white)}
	    {Tag       tk(itemconfigure width:CellFrame+1 outline:black)}
	    Selected <- Tag
	    Resource <- Res
	 end
      
	 meth getRes($)
	    @Resource
	 end
      
	 meth disable
	    if @Enabled then
	       {self.DisableTag tk(move 0 HideY)}
	       Enabled <- false
	    end
	 end
      
	 meth enable
	    if @Enabled then skip else
	       {self.DisableTag tk(move 0 ~HideY)}
	       Enabled <- true
	       {self.Action}
	    end
	 end
      
      end
   
   end
	 

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
		    if Dur=<MaxDur then
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
		    if Dur==1 then Selected <- T1
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
	    if @Enabled then
	       {self.DisableTag tk(move 0 HideY)}
	       Enabled <- false
	    end
	 end
      
	 meth enable
	    if @Enabled then skip else
	       {self.DisableTag tk(move 0 ~HideY)}
	       Enabled <- true
	       {self.Action}
	    end
	 end
      
      end
   end

   class Tools
      from TkTools.note
      feat Board
      attr Current:create(MaxRes 1)
      meth tkInit(parent:P board:B)
	 TkTools.note,tkInit(parent:P text:'Edit')
	 
	 Var  = {New Tk.variable tkInit(create)}
	 
	 proc {ResA}
	    {DurT disable}
	    {Var tkSet(none)}
	    Current <- resource(fun {$} {ResT getRes($)} end)
	 end
	 ResT = {New ResourceTool tkInit(parent:self action:ResA)}
	 
	 proc {DurA}
	    {ResT disable}
	    {Var tkSet(none)}
	    Current <- duration(fun {$} {DurT getDur($)} end)
	 end
	 DurT = {New DurationTool tkInit(parent:self action:DurA)}
	 
	 proc {CreA}
	    {ResT disable}
	    {DurT disable}
	    Current <- create({ResT getRes($)} {DurT getDur($)})
	 end
	 CreT = {New Tk.radiobutton tkInit(parent: self
					   var:    Var
					   val:    create
					   text:   'Create Task'
					   action: CreA
					   font:   Helv
					   relief: ridge
					   bd:     2
					   anchor: w)}
	 
	 proc {DelA}
	    {ResT disable} {DurT disable}
	    Current <- delete
	 end
	 
	 DelT = {New Tk.radiobutton tkInit(parent: self
					   var:    Var
					   val:    delete
					   text:   'Delete Task'
					   action: DelA
					   font:   Helv
					   relief: ridge
					   bd:     2
					   anchor: w)}
      in
	 {Tk.batch [pack(ResT DurT padx:2 fill:x)
		    pack(CreT DelT pady:1 ipadx:2 fill:x)]}
	 {DurT disable}
	 {ResT disable}
	 self.Board = B
      end
      
      meth getTool($)
	 @Current
      end
      
      meth toTop
	 {self.Board setEdit}
	 {Explorer.object close}
      end
      
   end

end




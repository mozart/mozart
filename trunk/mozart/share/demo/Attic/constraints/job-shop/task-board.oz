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

   \insert 'task.oz'

   \insert 'job.oz'
   
   OffX = 20
   OffY = JobDistance
   
in
   
   class TaskBoard
      from Tk.canvas
      feat
	 Jobs Tools BackTag
      attr
	 EditMode: true
	 
      meth tkInit(parent:P tools:T spec:Spec)
	 self.Jobs       = {NewArray 1 MaxJobs 1}
	 {For 1 MaxJobs 1
	  proc {$ J}
	     {Put self.Jobs J {New Job init(number:J parent:self)}}
	  end}
	 self.Tools      = T
	 Tk.canvas, tkInit(parent:       P
			   bg:           ivory
			   width:  400
			   height: 220
			   bd:2 relief:sunken
			   scrollregion: q(~OffX
					   ~OffY
					   MaxSpan * DurUnit
					   MaxJobs * JobDistance)
			   xscrollincrement: 1
			   yscrollincrement: 1)
	 TaskBoard, tk(xview scroll ~OffX-6 units)
	 TaskBoard, tk(yview scroll ~OffY units)
	 self.BackTag = {New Tk.canvasTag tkInit(parent:self)}
	 {For 1 MaxJobs 1
	  proc {$ J}
	     Y = (MaxJobs - J) * JobDistance 
	  in
	     {self tk(create text ~5 Y font:Courier
		      text: case J==1 then 10 else 0#(MaxJobs - J + 1) end
		      anchor:e)}
	  end}
	 {For 1 MaxJobs 1
	  proc {$ J}
	     Y  = (MaxJobs - J) * JobDistance
	     Y0 = Y - JobDistance div 2 + 1
	     Y1 = Y + JobDistance div 2 - 1
	  in
	     {self tk(create rectangle 0 Y0 MaxSpan*DurUnit Y1
		      fill:ivory outline:'' tags:self.BackTag)}
	  end}
	 {For 1 MaxJobs+1 1
	  proc {$ J}
	     Y = (MaxJobs - J) * JobDistance + JobDistance div 2
	     in
	     {self tk(create line 0 Y MaxSpan*DurUnit Y
		      fill:gray50)}
	     end}
	 {self.BackTag tkBind(event:  '<1>'
			      args:   [int(y)]
			      action: self # action(unit))}
	 {List.forAllInd Spec
	  proc {$ JN Ts}
	     J={Get self.Jobs JN}
	  in
	     {ForAll Ts proc {$ D#R}
			   {J newTask(resource:R duration:D)}
			end}
	  end}
      end
	 
      meth action(T SY)
	 case @EditMode then
	    Y = SY - OffY
	    J = {Get self.Jobs
		 {Min {Max 1 (Y + JobDistance div 2 ) div JobDistance + 1}
		  MaxJobs}}
	 in
	    case {self.Tools getTool($)}
	    of create(R D) then
	       {J newTask(resource:R duration:D)}
	    [] delete      then
	       case T==unit then skip else {J deleteTask(T)} end
	    [] resource(GR) then
	       case T==unit then skip else {T setResource({GR})} end
	    [] duration(GD) then
	       case T==unit then skip else {J setDuration(T {GD})} end
	    end
	 else skip
	 end
      end
      
      meth getSpec($)
	 pa # 0 # nil # noResource |
	 pe # 0 # {ForThread 1 MaxJobs 1
		   fun {$ Js J}
		      {Append {{Get self.Jobs J}
			       getLastSpec($)}
		       Js}
		   end nil} # noResource |
	 {ForThread 1 MaxJobs 1
	  fun {$ Ss J}
	     {Append {{Get self.Jobs J} getSpec($)} Ss}
	  end nil}
      end
      
      meth setEdit
	 EditMode <- true
	 {For 1 MaxJobs 1
	  proc {$ J}
	     {{Get self.Jobs J} setEdit}
	  end}
      end
      
      meth setSol(Sol)
	 EditMode <- false
	    {For 1 MaxJobs 1
	     proc {$ J}
		{{Get self.Jobs J} setSol(Sol)}
	     end}
      end
      meth displaySol(Sol)
	 {For 1 MaxJobs 1
	  proc {$ J}
	     {{Get self.Jobs J} setSol(Sol)}
	  end}
	 end
      
   end
end


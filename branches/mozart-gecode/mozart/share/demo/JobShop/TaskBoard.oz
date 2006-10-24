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
   Tk

   Configure(maxJobs: MaxJobs
	     maxSpan: MaxSpan
	     
	     durUnit:     DurUnit
	     durFrame:    DurFrame
	     jobDistance: JobDistance

	     type:      Courier
	     resColors: ResColors)

export
   'class': TaskBoard

prepare

   fun {GetTaskName J T}
      {VirtualString.toAtom j#J#t#T}
   end
   
   fun {GetResourceName R}
      {VirtualString.toAtom r#R}
   end
   
   proc {TaskNameTo TN ?J ?T}
      S = {Atom.toString TN}.2
   in
      J = {String.toInt {List.takeWhile S Char.isDigit}}
      T = {String.toInt {List.dropWhile S Char.isDigit}.2}
   end
   
define

   OffX = 20
   OffY = JobDistance

   class Task
      from Tk.canvasTag
      attr
	 X0:0 Y0:0 X1:0
	 Duration: 0
	 Resource: unit
	 EditMode: true
      
      meth init(parent:P resource:R duration:D x:X y:Y)
	 Task, tkInit(parent:P)
	 X0       <- X
	 Y0       <- Y
	 Duration <- D
	 Resource <- R
	 {P tk(create rectangle
	       X                         Y - DurUnit div 2
	       X + D*DurUnit - DurFrame  Y + DurUnit div 2
	       fill:ResColors.R tags:self)}
	 Task, tkBind(event:  '<1>'
		      args:   [int(y)]
		      action: P # action(self))
      end
   
      meth setDuration(D)
	 Duration <- D
	 Task, tk(coords
		  @X0                      @Y0 - DurUnit div 2
		  @X0 + D*DurUnit-DurFrame @Y0 + DurUnit div 2)
      end
   
      meth getDuration($)
	 @Duration
      end
   
      meth setResource(R)
	 Resource <- R
	 Task, tk(itemconfigure fill:ResColors.R)
      end
   
      meth getResource($)
	 @Resource
      end
   
      meth move(ByX)
	 X0 <- @X0 + ByX
	 Task,tk(move ByX 0)
      end
   
      meth setSol(S)
	 X = S * DurUnit
      in
	 if @EditMode then Task,tk(move X-@X0 0)
	 else Task,tk(move X-@X1 0)
	 end
	 EditMode <- false
	 X1       <- X
      end
   
      meth setEdit
	 if @EditMode then skip else
	    EditMode <- true
	    Task,tk(move @X0-@X1 0)
	 end
      end
   
   end


   class Job
      feat
	 Number
	 Parent
      attr
	 Tasks:  nil
	 NextX:  0
	 
      meth init(parent:P number:N)
	 self.Parent = P
	 self.Number = N
	 Tasks <- nil
	 NextX <- 0
      end
      
      meth newTask(resource:R duration:D)
	 Tasks <- {Append @Tasks
		   [{New Task
		     init(parent:   self.Parent
			  resource: R
			  duration: D
			  x:        @NextX
			  y:        (self.Number - 1) * JobDistance)}]}
	 NextX <- @NextX + DurUnit * D
      end
      
      meth DelTask(Ts D $)
	 case Ts of nil then nil
	 [] T|Tr then
	    if T==D then
	       {ForAll Tr
		proc {$ T}
		   {T move(~{D getDuration($)} * DurUnit)}
		end} Tr
	    else T|Job,DelTask(Tr D $)
	    end
	 end
      end
      
      meth deleteTask(D)
	 {D tk(delete)}
	 NextX <- @NextX - {D getDuration($)} * DurUnit
	 Tasks <- Job,DelTask(@Tasks D $)
      end
      
      meth SetDur(Ts S D)
	 case Ts of nil then skip
	 [] T|Tr then
	    if T==S then
	       {ForAll Tr
		proc {$ T}
		   {T move((D-{S getDuration($)}) * DurUnit)}
		end}
	    else Job,SetDur(Tr S D)
	    end
	 end
      end
      
      meth setDuration(T D)
	 NextX <- @NextX + (D - {T getDuration($)}) * DurUnit
	 Job,SetDur(@Tasks T D) 
	 {T setDuration(D)}
      end
      
      meth setSol(S)
	 {Record.forAllInd S
	  proc {$ A S}
	     if A\=pa andthen A\=pe then J T in
		{TaskNameTo A ?J ?T}
		if self.Number==J then {{Nth @Tasks T} setSol(S)} end
	     end
	  end}
      end
      
      meth setEdit
	 {ForAll @Tasks proc {$ T} {T setEdit} end}
      end
      
      meth getLastSpec($)
	 case @Tasks of nil then nil else
	    [{GetTaskName self.Number {Length @Tasks}}]
	 end
      end
      
      meth getSpec($)
	 {List.mapInd @Tasks
	  fun {$ I T}
	     Task={GetTaskName self.Number I}
	     Dur ={T getDuration($)}
	     Res ={GetResourceName {T getResource($)}}
	     Pre = if I==1 then [pa]
		   else [{GetTaskName self.Number I-1}]
		   end 
	  in
	     Task(dur:Dur pre:Pre res:Res)
	  end}
      end
      
   end

   
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
		      text:  if J==1 then 10 else 0#(MaxJobs - J + 1) end
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
	 if @EditMode then
	    Y = SY - OffY
	    J = {Get self.Jobs
		 {Min {Max 1 (Y + JobDistance div 2 ) div JobDistance + 1}
		  MaxJobs}}
	 in
	    case {self.Tools getTool($)}
	    of create(R D) then
	       {J newTask(resource:R duration:D)}
	    [] delete      then
	       if T\=unit then {J deleteTask(T)} end
	    [] resource(GR) then
	       if T\=unit then {T setResource({GR})} end
	    [] duration(GD) then
	       if T\=unit then {J setDuration(T {GD})} end
	    end
	 end
      end
      
      meth getSpec($)
	 pa(dur:0) |
	 pe(dur:0 pre:{ForThread 1 MaxJobs 1
		       fun {$ Js J}
			  {Append {{Get self.Jobs J}
				   getLastSpec($)}
			   Js}
		       end nil}) |
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

%%%
%%% Authors:
%%%   Martin Müller, (mmueller@ps.uni-sb.de)
%%%
%%% Contributor:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Martin Müller, 1998
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor

import
   Tk
   TkTools
   Application

define
   %%
   %% Lift Simulation -- Randomised and Interactive Lift Requests
   %%
   %% The Scheduling Algorithm is taken from the book
   %% 'Concurrent Programming in Erlang', Chapter 11: Real-Time Control
   %%

   %%
   %% Colors
   %%
   [BGColor FGColor FloorsColor KnobColor NumColor]
   = if Tk.isColor then
	[steelblue bisque lightsteelblue blue 'IndianRed']
     else
	[grey55 grey55 grey55 black black]
     end
   
   %%
   %% Parameters
   %%
   
   NumFloors  = 5          %% number of floors
   NumLifts   = 3          %% number of lifts
   
   CallMode   = call       %% lift called from floor
   SendMode   = send       %% lift sent from inside
   DelayLift  = 2500       %% delay between simulated lift calls
   DelayOpen  = 500        %% delay for opening doors
   DelayStep  = 120        %% delay for lift speed
   
   %% 
   %% Images
   %%
   
   Images = {TkTools.images
	     {Map [upF downF upE downE lift liftUp liftDown]
	      fun {$ A}
		 'http://mozart.ps.uni-sb.de/home/doc/demo/images/lift/'#A#'.xbm'
	      end}}
   
   Bold = 'lucidasanstypewriter-bold-18'
   
   %%
   %% constants and procedures for the graphical representation
   %%
   
   FloorDelta = 10
   FloorSteps = 7
   FloorLeft  = 10
   FloorRight = 220
   FloorSize  = FloorDelta * FloorSteps
   Height     = FloorSize * NumFloors - 15
   Width      = 230
   LeftMost   = 110
   Bound      = 3
   LiftDelta  = 38
   
   fun {Pos2Floor Y} (Height - Y) div FloorSize + 1 end
   fun {Floor2Pos N} Height - (N-1) * FloorSize end
   
   GroundLevel = {Floor2Pos 1} - LiftDelta
   

   %%
   %% Random Number Generator (alternatively, see OS.rand)
   %%
      
   class Randomizer from BaseObject
      prop
	 final
	 
      feat
	 a: 24298
	 b: 9991
	 c: 199017
	 
      attr
	 seed:24676
	 
      meth next(?N)
	 S=@seed
      in
	 N={IntToFloat S}/{IntToFloat self.c}
	 seed <- (self.a * S + self.b) mod self.c
      end
      
      meth uniform(A B ?R)
	 N in Randomizer, next(N) R=N*(B-A)+A
      end
   end
   
   Random = {New Randomizer noop}
   
   fun {RandomChoice N M}
      {FloatToInt {Random uniform({IntToFloat N}
				  {IntToFloat M}
				  $)}}
   end
   
   fun {RanChoiceFloor}
      {RandomChoice 1 NumFloors}
   end
   
   fun {RanChoiceBin X Y}
      if {RandomChoice 0 1}==1 then X else Y end
   end
   
   %%
   %% Compute the cost of a request from floor Floor with direction 
   %% Dir for a lift at floor At with current task stack CurrSch,
   %% and also return the corresponding new schedule NewSch.
   %%
   
   CostWait = 5 %% cost estimate for opening and closing the doors
   
   fun {Insert Task At CurrSch}
      {Cost Task At CurrSch _} % ignore cost
   end
   
   proc {Cost Task At CurrSch Offer NewSch}
      goto(To _ Dir _) = Task
   in
      case CurrSch
      of nil then
	 Offer  = {Abs At-To}
	 NewSch = [Task]
	 
      [] CurrTask|RCurrSch then
	 goto(CurrTo _ _ _) = CurrTask
      in
	 if At=<To
	    andthen To=<CurrTo
	    andthen Dir==up
	 then
	    Offer  = {Abs At-To}
	    NewSch = Task|CurrSch
	    
	 elseif CurrTo=<To
	    andthen To=<At
	    andthen Dir==down
	 then
	    Offer  = {Abs At-CurrTo}
	    NewSch = Task|CurrSch
	    
	 else
	    ROffer RNewSch
	 in
	    {Cost Task CurrTo RCurrSch ROffer RNewSch}
	    Offer  = {Abs At-CurrTo} + CostWait + ROffer
	    NewSch = CurrTask|RNewSch
	 end
      end
   end
   
   %%
   %% Interactively send the lift somewhere
   %%
   
   class Press from BaseObject
		  
      prop final
      attr val
	 
      meth init(Choice)
	 val <- Choice
      end
      meth press(N)
	 @val = N
      end
   end
   
   proc {PopChoice View P ?Choice}
      Select = {New Press init(Choice)}
      proc {Button N}
	 At = NumFloors - N + 1
	 Y  = P + LiftDelta - At * 8
	 Knob = {New Tk.canvasTag tkInit(parent:View)}
	 Num  = {New Tk.canvasTag tkInit(parent:View)}
      in
	 {View tk(crea oval 92 Y 100 Y+5 tag:Knob fill:KnobColor)}
	 {View tk(crea text 85 Y tag:Num text:At fill:NumColor)}
	 {Knob tkBind(event:'<1>' action: Select # press(At))}
	 
	 thread
	    {Wait Choice}
	    {Knob tkClose}
	    {Num  tkClose}
	 end
      end
   in
      {Loop.for 1 NumFloors 1 Button}
   end
   
   %%
   %% Individual Lift Object
   %%
   
   class Lift from Tk.canvasTag
		 
      prop
	 locking
	 final
	 
      feat
	 View
	 PosX
	 
      attr
	 PosY     : GroundLevel
	 CurrFloor: 1
	 Agenda   : nil
	 State    : idle
	 
      meth init(L)
	 self.PosX   = {L join(self $)}
	 self.View   = L
	 
	 Tk.canvasTag, tkInit(parent:L)
	 Lift, drawInit
      end
      
      meth request(Task ?Offer Cont)
	 
	 lock
	    ReqCost NewAgenda
	 in
	    {Cost Task @CurrFloor @Agenda ReqCost NewAgenda}
	    
	    case @State
	    of openDoor then
	       %% 
	       %% guess a high delay dependent on current agenda
	       %%
	       Offer = ({Length @Agenda}+1)*1000
	    else
	       Offer = ReqCost
	    end
	    
	    %%
	    %% wait for release (false) or commit (true)
	    %%
	    if Cont then
	       Agenda <- NewAgenda
	       case @Agenda of [_] then
		  case @State of openDoor
		  then skip else Lift, proceed
		  end
	       else skip end
	    end
	 end
	 
      end
      
      meth moveBy(Y)
	 PosY      <- @PosY+Y
	 CurrFloor <- {Pos2Floor @PosY}
	 Tk.canvasTag, tk(move 0 Y)
      end
      
      meth proceed
	 
	 if @Agenda==nil then
	    State <- idle
	    Lift, draw(lift)
	 else
	    To      = @Agenda.1.1
	    PosTo   = {Floor2Pos To}
	    CurrPos = @PosY+LiftDelta
	 in
	    Lift, draw(if To>@CurrFloor then liftUp
		       elseif To<@CurrFloor then liftDown
		       else lift end)
	    
	    if {Abs CurrPos-PosTo}<5 then
	       Lift, reached
	    elseif CurrPos>PosTo then
	       Lift, moveBy(~FloorDelta)
	       
	       thread
		  {Delay DelayStep}
		  {self proceed}
	       end
	       
	    else
	       Lift, moveBy(FloorDelta)
	       
	       thread
		  {Delay DelayStep}
		  {self proceed}
	       end
	    end
	 end
	 
      end
      
      meth reached
	 
	 Mode Who RestAgenda
      in
	 
	 lock
	    goto(_ Mode _ Who)|RestAgenda = @Agenda
	    {self.View reached(@CurrFloor)}
	    Agenda <- RestAgenda
	 end
	 
	 if Mode==!CallMode then
	    To
	    CurrY = @PosY
	    Num   = @CurrFloor
	 in
	    State <- openDoor
	    
	    thread
	       To = case Who of sim then
		       {Delay DelayOpen}
		       {RanChoiceFloor}
		    elseof press then
		       {PopChoice self.View CurrY}
		    end
	    end
	    
	    {Wait To}
	    
	    Agenda <- {Insert
		       goto(To
			    SendMode
			    if To>Num then up else down end
			    Who)
		       @CurrFloor
		       @Agenda}
	 end
	 Lift, closeDoor

      end
      
      meth closeDoor
	 State <- idle
	 Lift, proceed
      end
      
      meth drawInit
	 {self.View
	  tk(crea image self.PosX+12 @PosY+12 image:Images.lift tag:self)}
      end
      
      meth draw(I)
	 Tk.canvasTag, tk(itemconf image:Images.I)
      end
   end
   
   %%
   %% request a lift 
   %%
   
   local
      fun {ListMin Xs}
	 case Xs of [X] then X
	 elseof (X#CX)|Xr then
	    MXr#CXr = {ListMin Xr}
	 in
	    if X<MXr then
	       CXr = false % release 
	       X#CX        % current minimal offer
	    else
	       CX = false  % release
	       MXr#CXr     % current minimal offer
	    end
	 end
      end
   in
      proc {Request Ls Task}
	 fun {DoReq L}
	    O C
	 in
	    thread
	       {L request(Task O C)}
	    end
	    O#C
	 end
      in
	 true = {ListMin {Map Ls DoReq}}.2 % commit 
      end
   end
   
   %%
   %% Individual Floor Object
   %%
   
   class Floor from BaseObject
		  
      prop
	 final
	 
      feat
	 Num
	 PosY
	 View
	 Up UpTag
	 Down DownTag
	 
      meth init(L N)
	 self.Num  = N
	 self.PosY = {Floor2Pos N}
	 self.View = L
	 self.Down = N>1
	 self.Up   = N<NumFloors
	 
	 Floor, drawText
	 
	 if self.Up then
	    Floor, drawButton(self.UpTag
			      35
			      self.PosY-15
			      upF
			      doUp(press))
	 end
	 
	 if self.Down then
	    Floor, drawButton(self.DownTag
			      55
			      self.PosY-15
			      downF
			      doDown(press))
	 end
	 
	 Floor, show
      end
      
      meth doUp(Who)
	 if self.Up then
	    Ms = {self.View members($)}
	 in
	    Floor, draw(true false)  % up/empty
	    {Request Ms goto(self.Num CallMode up Who)}
	 end
      end
      
      meth doDown(Who)
	 if self.Down then
	    Ms = {self.View members($)}
	 in
	    Floor, draw(false false)  % down/empty
	    {Request Ms goto(self.Num CallMode down Who)}
	 end
      end
      
      meth reached
	 if self.Up then
	    Floor, draw(true true)  % up/full
	 end
	 if self.Down then
	    Floor, draw(false true) % down/full
	 end
      end
      
      meth show
	 {self.View
	  tk(crea line FloorLeft self.PosY FloorRight self.PosY)}
      end
      
      meth draw(Up Full)
	 if Up then
	    if Full
	    then {self.UpTag tk(itemconf image:Images.'upF')}
	    else {self.UpTag tk(itemconf image:Images.'upE')}
	    end
	 else
	    if Full
	    then {self.DownTag tk(itemconf image:Images.'downF')}
	    else {self.DownTag tk(itemconf image:Images.'downE')}
	    end
	 end
      end
      
      meth drawText
	 {self.View tk(crea text 15 self.PosY-17
		       text: self.Num
		       font: Bold
		       tag : {New Tk.canvasTag tkInit(parent:self.View)}
		       fill: blue)}
      end
      
      meth drawButton(Tag X Y I Action)
	 Tag = {New Tk.canvasTag tkInit(parent:self.View)}
	 {self.View tk(crea image X Y image:Images.I tag:Tag)}
	 {Tag tkBind(event:'<1>' action:proc {$} {self Action} end)}
      end
   end
   
   %%
   %% Lift Manager
   %%
   
   class LiftManager from Tk.canvas Time.repeat
			
      prop
	 final
	 
      feat
	 Floors
	 
      attr
	 Members: nil
	 PosX   : LeftMost
	 Queue  : nil
	 
      meth init(parent:W floors:AllFloors)
	 self.Floors = AllFloors
	 
	 Tk.canvas, tkInit(parent:     W
			   width:      Width
			   height:     Height
			   relief:     sunken
			   bd:         Bound
			   background: FloorsColor)
	 
	 Time.repeat, setRepAll(action:press delay:DelayLift)
      end
      
      meth join(L ?X)
	 X = @PosX
	 PosX <- X + LiftDelta
	 Members <- L|@Members
      end
      
      meth members($)
	 @Members
      end
      
      meth press
	 R={RanChoiceFloor}
	 Action = if R==NumFloors then doDown
		  elseif R==1 then doUp
		  else {RanChoiceBin doUp doDown}
		  end
      in
	 {{Nth self.Floors R} Action(sim)}
      end
      
      meth reached(N)
	 {{Nth self.Floors N} reached}
      end
      
      meth close
	 Time.repeat, stop
	 Tk.canvas,   tkClose
      end
   end
   
   
   %%
   %% Create Window and Lifts object as manager for group of Lift objects
   %%

   Top = {New Tk.toplevel tkInit(title:  'Oz Lifts'
				 delete: Application.exit # 0)}
   
   BFrame = {New Tk.frame tkInit(parent:     Top
				 width:      20
				 background: BGColor)}
   
   L1 = {New Tk.label tkInit(parent:     BFrame
			     text:       "automatic operation"
			     background: BGColor)}
   
   L2 = {New Tk.label tkInit(parent:     Top
			     text:       "Press arrows for manual operation"
			     width:      30
			     background: BGColor)}
   
   AllFloors  = {List.make NumFloors}
   AllLifts   = {List.make NumLifts}
   
   Lifts = {New LiftManager init(parent:Top floors:AllFloors)}
   
   GoB   = {New Tk.label tkInit(parent:     BFrame
				text:       " start "
				relief:     raised
				bd:         2
				width:      9
				background: FGColor)}
   
   proc {AutoOn}
      {GoB tk(conf text:" stop ")}
      {GoB tkBind(action:AutoOff event:'<1>')}
      {Lifts go}
   end
   
   proc {AutoOff}
      {GoB tk(conf text:" start ")}
      {GoB tkBind(action:AutoOn event:'<1>')}
      {Lifts stop}
   end
   
   {GoB tkBind(action:AutoOn event:'<1>')}
   
   {Tk.batch [pack(Lifts  padx:10 pady:20)
	      pack(BFrame side:top)
	      pack(GoB L1 side:left padx:2 expand:true)
	      pack(L2     side:top pady:2)
	     ]}
   
   
   %%
   %% Create lift and floor objects
   %%
   
   if NumFloors>=2 then
      
      {List.forAllInd AllFloors
       fun {$ N}
	  {New Floor init(Lifts N)}
       end}
      
      {ForAll AllLifts
       fun{$}
	  {New Lift init(Lifts)}
       end}
      
   end
   
   
end


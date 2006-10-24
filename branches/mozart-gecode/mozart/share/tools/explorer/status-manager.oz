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

   fun {AddZero N}
      if N<10 then '0'#N else N end
   end
   
   fun {FormatTime Totaltime}
      Hours      = Totaltime div 3600000
      Minutes    = Totaltime div 60000
      Seconds    = Totaltime div 1000
   in
      if Hours==0 then
	 if Minutes==0 then
	    if Seconds==0 then Totaltime # 'ms'
	    else Seconds # '.' # {AddZero (Totaltime - 1000 * Seconds ) div 10}
	       # 's'
	    end
	 else Minutes # 'm ' # Seconds - Minutes * 60 # 's'
	 end
      else Hours # 'h ' # Minutes - Hours * 60 # 'm'
      end
   end

   class Status
      from Tk.frame
      prop final		
      attr
	 MaxDepth:        0
	 CurNodes:        0
	 CurSolutions:    0
	 CurFailures:     0
	 CurSuspended:      0
	 BreakFlag:       nil
	 BreakStatus:     none
	 BrokenNodes:     nil
	 KillFlag:        true
	 KillId:          0
	 IsPackedSuspended: false

      feat
	 Time
	 Bab
	 Choose
	 ChooseImage
	 Depth
	 Solution
	 Failure
	 Suspended
	 SuspendedImage
      
      meth init(Parent)
	 Tk.frame,tkInit(parent:             Parent
			 highlightthickness: 0
			 relief:             sunken)
	 BabField   = {New Tk.label tkInit(parent: self
					   text:   ''
					   font:   BoldStatusFont)}
	 
	 TimeFrame = {New Tk.frame tkInit(parent: self)}
	 
	 TimeLabel = {New Tk.label tkInit(parent: TimeFrame
					  text:   'Time:'
					  font:   StatusFont)}
	 TimeField = {New Tk.label tkInit(parent: TimeFrame
					  text:   {FormatTime 0}
					  font:   BoldStatusFont)}
	 
	 NodeFrame      = {New Tk.frame tkInit(parent: self)}
	 ChooseIm       = {New Images.choose init(parent: NodeFrame)}
	 ChooseNumber   = {New Tk.label tkInit(parent: NodeFrame
					       font:   BoldStatusFont
					       text:   '0')}
	 SolImage       = {New Images.succeeded init(parent: NodeFrame)}
	 SolNumber      = {New Tk.label tkInit(parent: NodeFrame
					       font:   BoldStatusFont
					       text:   '0')}
	 FailedImage    = {New Images.failed init(parent: NodeFrame)}
	 FailedNumber   = {New Tk.label tkInit(parent: NodeFrame
					       font:   BoldStatusFont
					       text:   '0')}
	 SuspendedIm      = {New Images.suspended init(parent: NodeFrame)}
	 SuspendedNumber  = {New Tk.label tkInit(parent: NodeFrame
					       font:   BoldStatusFont
					       text:   '0')}
	 DepthFrame = {New Tk.frame tkInit(parent: self)}
	 
	 DepthLabel = {New Tk.label tkInit(parent: DepthFrame
					   text:   'Depth:'
					   font:   StatusFont)}
	 DepthField = {New Tk.label tkInit(parent: DepthFrame
					   text:   0
					   font:   BoldStatusFont)}
      in
	 {Tk.batch [pack(TimeLabel  TimeField side:left)
		    pack(DepthLabel DepthField side:left)
		    pack(ChooseIm ChooseNumber
			 SolImage SolNumber
			 FailedImage FailedNumber side:left)
		    pack(BabField TimeFrame NodeFrame DepthFrame
			 side:left padx:HugePad)]}
	 self.Time          = TimeField
	 self.Bab           = BabField
	 self.Depth         = DepthField
	 self.Choose        = ChooseNumber
	 self.ChooseImage   = ChooseIm
	 self.Solution      = SolNumber
	 self.Failure       = FailedNumber
	 self.Suspended       = SuspendedNumber
	 self.SuspendedImage  = SuspendedIm
      end

      meth setBAB(IsBAB)
	 {self.Bab tk(conf text: if IsBAB then 'BAB' else '' end)}
      end

      meth setTime(T)
	 {self.Time tk(conf text:T)}
      end
      
      meth clear
	 MaxDepth      <- 0
	 CurNodes      <- 0
	 CurSolutions  <- 0
	 CurFailures   <- 0
	 CurSuspended    <- 0
	 @KillFlag     = true
	 KillFlag      <- _
	 KillId        <- @KillId + 1
	 if @IsPackedSuspended then
	    {Tk.send pack(forget self.Suspended self.SuspendedImage)}
	    IsPackedSuspended <- false 
	 end
	 Status,update
	 Status,start
	 Status,setBAB(false)
	 {self.ChooseImage clear}
      end

      meth hasSuspended($)
	 @CurSuspended>0
      end
      
      meth halt
	 @BreakFlag = self
	 BreakStatus <- case @BreakStatus
			of kill  then kill
			[] break then break
			else halt
			end
      end

      meth break
	 @BreakFlag = self
	 BreakStatus <- case @BreakStatus
			of kill  then kill
			else break
			end
      end

      meth kill
	 @BreakFlag = self
	 BreakStatus <- kill
      end

      meth broken(Node)
	 BrokenNodes <- Node|@BrokenNodes
      end

      meth getBrokenNodes(?Ns)
	 Ns=@BrokenNodes
	 BrokenNodes <- nil
      end
      
      meth start
	 BreakFlag   <- _
	 BreakStatus <- none
	 BrokenNodes <- nil
      end

      meth unbreak
	 BreakFlag   <- _
      end

      meth getBreakFlag($)
	 @BreakFlag
      end

      meth getBreakStatus($)
	 @BreakStatus
      end
      
      meth finish
	 {self.ChooseImage finish}
      end
      
      meth update
	 GetDepth     = @MaxDepth
	 GetNodes     = @CurNodes
	 GetSolutions = @CurSolutions
	 GetFailures  = @CurFailures
	 GetSuspended   = @CurSuspended
      in
	 if (GetSuspended==0) == (@IsPackedSuspended) then
	    if @IsPackedSuspended then
	       IsPackedSuspended <- false
	       {Tk.send pack(forget self.Suspended self.SuspendedImage)}
	    else
	       IsPackedSuspended <- true
	       {Tk.send pack(self.SuspendedImage self.Suspended side:left)}
	    end
	 end
	 {self.Depth    tk(conf text:GetDepth)}
	 {self.Choose   tk(conf
			   text:GetNodes-(GetSolutions+GetFailures+GetSuspended))}
	 {self.Solution tk(conf text:GetSolutions)}
	 {self.Failure  tk(conf text:GetFailures)}
	 {self.Suspended  tk(conf text:GetSuspended)}
      end
      
      meth addSolution(Depth)
	 IncNodes = @CurNodes + 1
      in
	 MaxDepth     <- {Max @MaxDepth Depth}
	 CurSolutions <- @CurSolutions + 1
	 CurNodes     <- IncNodes
	 if IncNodes mod StatusUpdateCnt==0 then Status,update end
      end

      meth addSuspended(Depth)
	 IncNodes = @CurNodes + 1
      in
	 MaxDepth    <- {Max @MaxDepth Depth}
	 CurSuspended  <- @CurSuspended + 1
	 CurNodes    <- IncNodes
	 if IncNodes mod StatusUpdateCnt==0 then Status,update end
      end

      meth removeSuspended
	 CurNodes    <- @CurNodes - 1
	 CurSuspended  <- @CurSuspended - 1
      end
      
      meth addFailed(Depth)
	 IncNodes = @CurNodes + 1
      in
	 MaxDepth    <- {Max @MaxDepth Depth}
	 CurFailures <- @CurFailures + 1
	 CurNodes    <- IncNodes
	 if IncNodes mod StatusUpdateCnt==0 then Status,update end
      end

      meth addChoose(Depth)
	 IncNodes = @CurNodes + 1
      in
	 CurNodes <- IncNodes
	 MaxDepth <- {Max @MaxDepth Depth}
	 if IncNodes mod StatusUpdateCnt==0 then Status,update end
      end

      meth getKill(?Flag ?Id)
	 Flag=@KillFlag Id=@KillId
      end
   end

in

   class StatusManager
      attr
	 CurTotalTime: 0
	 StartTime:    0
      feat
	 status

      meth init
	 self.status = {New Status init(self.toplevel)}
      end

      meth clear
	 {self.status clear}
	 CurTotalTime <- 0
	 {self.status setTime({FormatTime 0})}
      end

      meth start($)
	 StartTime  <- {Property.get 'time.user'}
	 {self.status start}
	 {self.status getBreakFlag($)}
      end

      meth startTime
	 StartTime  <- {Property.get 'time.user'}
      end
      
      meth stop
	 CurTotalTime <- (@CurTotalTime +
			  {Property.get 'time.user'} - @StartTime)
	 {self.status update}
	 {self.status setTime({FormatTime @CurTotalTime})}
      end

      meth getBreakFlag($)
	 {self.status getBreakFlag($)}
      end

      meth getBreakStatus($)
	 {self.status getBreakStatus($)}
      end
      
      meth unbreak
	 {self.status unbreak}
      end

      meth clearBreak
	 {self.status start}
      end
      
      meth getBrokenNodes($)
	 {self.status getBrokenNodes($)}
      end

      meth hasSuspended($)
	 {self.status hasSuspended($)}
      end
      
      meth finish
	 {self.status finish}
      end

   end
   
end

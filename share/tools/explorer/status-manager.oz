%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   fun {AddZero N}
      case N<10 then '0'#N else N end
   end
   
   fun {FormatTime Totaltime Copytime}
      Hours      = Totaltime div 3600000
      Minutes    = Totaltime div 60000
      Seconds    = Totaltime div 1000
   in
      case Hours==0 then
	 case Minutes==0 then
	    case Seconds==0 then Totaltime # 'ms'
	    else Seconds # '.' # {AddZero (Totaltime - 1000 * Seconds ) div 10}
	       # 's'
	    end
	 else Minutes # 'm ' # Seconds - Minutes * 60 # 's'
	 end
      else Hours # 'h ' # Minutes - Hours * 60 # 'm'
      end #
      ' (' # case Totaltime of 0 then '0'
	     else (100 * Copytime) div Totaltime
	     end # '%c)'
   end

   class Status
      from Tk.frame
      attr
	 MaxDepth:        0
	 CurNodes:        0
	 CurSolutions:    0
	 CurFailures:     0
	 CurBlocked:      0
	 BreakFlag:       nil
	 BreakStatus:     none
	 BrokenNodes:     nil
	 KillFlag:        true
	 KillId:          0
	 IsPackedBlocked: false

      feat
	 Time
	 Bab
	 Choose
	 ChooseImage
	 Depth
	 Solution
	 Failure
	 Blocked
	 BlockedImage
      
      meth init(Parent)
	 Tk.frame,tkInit(parent:             Parent
			 highlightthickness: 0
			 border:             Border
			 relief:             sunken)
	 BabField   = {New Tk.label tkInit(parent: self
					   text:   ''
					   font:   BoldStatusFont)}

	 TimeFrame = {New Tk.frame tkInit(parent: self)}
	 
	 TimeLabel = {New Tk.label tkInit(parent: TimeFrame
					  text:   'Searchtime:'
					  font:   StatusFont)}
	 TimeField = {New Tk.label tkInit(parent: TimeFrame
					  text:   {FormatTime 0 0}
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
	 BlockedIm      = {New Images.blocked init(parent: NodeFrame)}
	 BlockedNumber  = {New Tk.label tkInit(parent: NodeFrame
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
	 self.Blocked       = BlockedNumber
	 self.BlockedImage  = BlockedIm
      end

      meth setBAB(IsBAB)
	 {self.Bab tk(conf text: case IsBAB then 'BAB' else '' end)}
      end

      meth setTime(T)
	 {self.Time tk(conf text:T)}
      end
      
      meth clear
	 MaxDepth      <- 0
	 CurNodes      <- 0
	 CurSolutions  <- 0
	 CurFailures   <- 0
	 CurBlocked    <- 0
	 @KillFlag     = true
	 KillFlag      <- _
	 KillId        <- @KillId + 1
	 case @IsPackedBlocked then
	    {Tk.send pack(forget self.Blocked self.BlockedImage)}
	    IsPackedBlocked <- false 
	 else skip end
	 Status,update
	 Status,start
	 Status,setBAB(false)
	 {self.ChooseImage clear}
      end

      meth hasBlocked($)
	 @CurBlocked>0
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
	 GetBlocked   = @CurBlocked
      in
	 case (GetBlocked==0) == (@IsPackedBlocked) then
	    case @IsPackedBlocked then
	       IsPackedBlocked <- false
	       {Tk.send pack(forget self.Blocked self.BlockedImage)}
	    else
	       IsPackedBlocked <- true
	       {Tk.send pack(self.BlockedImage self.Blocked side:left)}
	    end
	 else skip end
	 {self.Depth    tk(conf text:GetDepth)}
	 {self.Choose   tk(conf
			   text:GetNodes-(GetSolutions+GetFailures+GetBlocked))}
	 {self.Solution tk(conf text:GetSolutions)}
	 {self.Failure  tk(conf text:GetFailures)}
	 {self.Blocked  tk(conf text:GetBlocked)}
      end
      
      meth addSolution(Depth)
	 IncNodes = @CurNodes + 1
      in
	 MaxDepth     <- {Max @MaxDepth Depth}
	 CurSolutions <- @CurSolutions + 1
	 CurNodes     <- IncNodes
	 case IncNodes mod StatusUpdateCnt==0 then Status,update
	 else skip
	 end
      end

      meth addBlocked(Depth)
	 IncNodes = @CurNodes + 1
      in
	 MaxDepth    <- {Max @MaxDepth Depth}
	 CurBlocked  <- @CurBlocked + 1
	 CurNodes    <- IncNodes
	 case IncNodes mod StatusUpdateCnt==0 then Status,update
	 else skip end
      end

      meth removeBlocked
	 CurNodes    <- @CurNodes - 1
	 CurBlocked  <- @CurBlocked - 1
      end
      
      meth addFailed(Depth)
	 IncNodes = @CurNodes + 1
      in
	 MaxDepth    <- {Max @MaxDepth Depth}
	 CurFailures <- @CurFailures + 1
	 CurNodes    <- IncNodes
	 case IncNodes mod StatusUpdateCnt==0 then Status,update
	 else skip end
      end

      meth addChoose(Depth)
	 IncNodes = @CurNodes + 1
      in
	 CurNodes <- IncNodes
	 MaxDepth <- {Max @MaxDepth Depth}
	 case IncNodes mod StatusUpdateCnt==0 then Status,update
	 else skip end
      end

      meth getKill(?Flag ?Id)
	 Flag=@KillFlag Id=@KillId
      end
   end

in

   class StatusManager
      attr
	 CurTotalTime: 0
	 CurCopyTime:  0
	 StartTime:    0
      feat
	 status

      meth init
	 self.status = {New Status init(self.toplevel)}
      end

      meth clear
	 {self.status clear}
	 CurTotalTime <- 0
	 CurCopyTime  <- 0
	 {self.status setTime({FormatTime 0 0})}
      end

      meth start($)
	 StartTime  <- {System.get time}
	 {self.status start}
	 {self.status getBreakFlag($)}
      end

      meth startTime
	 StartTime  <- {System.get time}
      end
      
      meth stop
	 S = {System.get time}
	 RunDelta  = S.run  - @StartTime.run
	 CopyDelta = S.copy - @StartTime.copy
      in
	 CurTotalTime <- @CurTotalTime + RunDelta + CopyDelta
	 CurCopyTime  <- @CurCopyTime  + CopyDelta
	 {self.status update}
	 {self.status setTime({FormatTime @CurTotalTime @CurCopyTime})}
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

      meth hasBlocked($)
	 {self.status hasBlocked($)}
      end
      
      meth finish
	 {self.status finish}
      end

   end
   
end

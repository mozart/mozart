%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   fun {FormatTime Totaltime Copytime}
      Hours      = Totaltime div 3600000
      Minutes    = Totaltime div 60000
      Seconds    = Totaltime div 1000
   in
      case Hours==0 then
	 case Minutes==0 then
	    case Seconds==0 then Totaltime # 'ms'
	    else Seconds # '.' # (Totaltime - 1000 * Seconds ) div 10 # 's'
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
	 MaxDepth:         0
	 CurNodes:         0
	 CurSolutions:     0
	 CurFailures:      0
	 CurUnstable:      0
	 BreakFlag:        nil
	 BreakStatus:      none
	 BrokenNodes:      nil
	 IsPackedUnstable: False

      feat
	 Time
	 Bab
	 Choice
	 ChoiceImage
	 Depth
	 Solution
	 Failure
	 Unstable
	 UnstableImage
      
      meth init(Parent)
	 <<Tk.frame tkInit(parent:             Parent
			   highlightthickness: 0
			   border:             Border
			   relief:             sunken)>>
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
	 ChoiceIm       = {New Images.choice init(parent: NodeFrame)}
	 ChoiceNumber   = {New Tk.label tkInit(parent: NodeFrame
					       font:   BoldStatusFont
					       text:   '0')}
	 SolImage       = {New Images.solved init(parent: NodeFrame)}
	 SolNumber      = {New Tk.label tkInit(parent: NodeFrame
					       font:   BoldStatusFont
					       text:   '0')}
	 FailedImage    = {New Images.failed init(parent: NodeFrame)}
	 FailedNumber   = {New Tk.label tkInit(parent: NodeFrame
					       font:   BoldStatusFont
					       text:   '0')}
	 UnstableIm     = {New Images.unstable init(parent: NodeFrame)}
	 UnstableNumber = {New Tk.label tkInit(parent: NodeFrame
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
	 {Tk.batch [pack(TimeLabel  TimeField o(side:left))
		    pack(DepthLabel DepthField o(side:left))
		    pack(ChoiceIm ChoiceNumber
			 SolImage SolNumber
			 FailedImage FailedNumber o(side:left))
		    pack(BabField TimeFrame NodeFrame DepthFrame
			 o(side:left padx:HugePad fill:x))]}
	 self.Time          = TimeField
	 self.Bab           = BabField
	 self.Depth         = DepthField
	 self.Choice        = ChoiceNumber
	 self.ChoiceImage   = ChoiceIm
	 self.Solution      = SolNumber
	 self.Failure       = FailedNumber
	 self.Unstable      = UnstableNumber
	 self.UnstableImage = UnstableIm
      end

      meth setBAB(?IsBAB)
	 {self.Bab tk(configure o(text: case IsBAB then 'BAB' else '' end))}
      end

      meth setTime(T)
	 {self.Time tk(configure o(text:T))}
      end
      
      meth clear
	 MaxDepth      <- 0
	 CurNodes      <- 0
	 CurSolutions  <- 0
	 CurFailures   <- 0
	 CurUnstable   <- 0
	 case @IsPackedUnstable then
	    {Tk.send pack(forget self.Unstable self.UnstableImage)}
	    IsPackedUnstable <- False 
	 else true end
	 <<Status update>>
	 <<Status unbreak>>
	 <<Status setBAB(False)>>
	 {self.ChoiceImage clear}
      end

      meth hasUnstable($)
	 @CurUnstable>0
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
      
      meth unbreak
	 BreakFlag   <- _
	 BreakStatus <- none
	 BrokenNodes <- nil
      end

      meth getBreakFlag($)
	 @BreakFlag
      end

      meth getBreakStatus($)
	 @BreakStatus
      end
      
      meth finish
	 {self.ChoiceImage finish}
      end
      
      meth update
	 GetDepth     = @MaxDepth
	 GetNodes     = @CurNodes
	 GetSolutions = @CurSolutions
	 GetFailures  = @CurFailures
	 GetUnstable  = @CurUnstable
      in
	 case (GetUnstable==0) == (@IsPackedUnstable) then
	    case @IsPackedUnstable then
	       IsPackedUnstable <- False
	       {Tk.send pack(forget self.Unstable self.UnstableImage)}
	    else
	       IsPackedUnstable <- True
	       {Tk.send pack(self.UnstableImage self.Unstable o(side:left))}
	    end
	 else true end
	 {self.Depth    tk(conf(text:GetDepth))}
	 {self.Choice   tk(conf(text:GetNodes -
				(GetSolutions+GetFailures+GetUnstable)))}
	 {self.Solution tk(conf(text:GetSolutions))}
	 {self.Failure  tk(conf(text:GetFailures))}
	 {self.Unstable tk(conf(text:GetUnstable))}
      end
      
      meth addSolution(Depth)
	 IncNodes = @CurNodes + 1
      in
	 MaxDepth     <- {Max @MaxDepth Depth}
	 CurSolutions <- @CurSolutions + 1
	 CurNodes     <- IncNodes
	 case IncNodes mod StatusUpdateCnt==0 then <<Status update>>
	 else true
	 end
      end

      meth addUnstable(Depth)
	 IncNodes = @CurNodes + 1
      in
	 MaxDepth    <- {Max @MaxDepth Depth}
	 CurUnstable <- @CurUnstable + 1
	 CurNodes    <- IncNodes
	 case IncNodes mod StatusUpdateCnt==0 then <<Status update>>
	 else true end
      end

      meth removeUnstable
	 CurNodes    <- @CurNodes - 1
	 CurUnstable <- @CurUnstable - 1
      end
      
      meth addFailed(Depth)
	 IncNodes = @CurNodes + 1
      in
	 MaxDepth    <- {Max @MaxDepth Depth}
	 CurFailures <- @CurFailures + 1
	 CurNodes    <- IncNodes
	 case IncNodes mod StatusUpdateCnt==0 then <<Status update>>
	 else true end
      end

      meth addChoice(Depth)
	 IncNodes = @CurNodes + 1
      in
	 CurNodes <- IncNodes
	 MaxDepth <- {Max @MaxDepth Depth}
	 case IncNodes mod StatusUpdateCnt==0 then <<Status update>>
	 else true end
      end

   end

in

   class StatusManager
      attr
	 CurTotalTime: 0
	 CurCopyTime:  0
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
	 {System.statistics _}
	 {self.status unbreak}
	 {self.status getBreakFlag($)}
      end

      meth stop
	 Statistics = {System.statistics}
      in
	 CurTotalTime <- @CurTotalTime + Statistics.r + Statistics.c
	 CurCopyTime  <- @CurCopyTime  + Statistics.c
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

      meth getBrokenNodes($)
	 {self.status getBrokenNodes($)}
      end

      meth hasUnstable($)
	 {self.status hasUnstable($)}
      end
      
      meth finish
	 {self.status finish}
      end

   end
   
end

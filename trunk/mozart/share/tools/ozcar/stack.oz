%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   local
      FDelimiter    = ' / '
      LRDelimiter   = ': '

      fun {FormatFeature R F D}
	 case {CondSelect R F unit} of unit then nil
	 elseof X then {Error.formatLine X} # D
	 end
      end

      fun {FormatLR L}
	 Left  = {FormatFeature L l nil}
	 Right = {FormatFeature L m nil}
	 Both  = Left \= nil andthen Right \= nil
      in
	 Left # case Both then LRDelimiter else nil end # Right
      end

      fun {CheckNil A}
	 case A == nil then nil else A # FDelimiter end
      end

      fun {FormatBody B}
	 case B == nil then nil else
	    {FoldL B
	     fun {$ A L}
		case L
		of hint(...)  then {CheckNil A} # {FormatLR L}
		[] pos(X Y _) then {CheckNil A} # 'Pos: ' # X # ', line ' # Y
		[] line(X)    then {CheckNil A} # {Error.formatLine X}
		[] unit       then A
		else {OzcarError 'wrong error message format'} A
		end
	     end nil}
	 end
      end
   in
      fun {FormatExceptionLine E}
	 {FormatFeature E kind FDelimiter} #
	 {FormatFeature E msg  FDelimiter} #
	 {FormatBody {CondSelect E body nil}}
      end
   end

   fun {S2F Nr Frame}
      frame(nr:      Nr   % frame counter
	    dir:     {Label Frame}   % 'entry' or 'exit'
	    file:    {CondSelect Frame file nofile}
	    line:    {CondSelect Frame line unit}
	    column:  {CondSelect Frame column unit}
	    time:    Frame.time
	    name:    case {CondSelect Frame name unit} of unit then
			case Frame.kind of call then
			   Data = {CondSelect Frame data unit}
			in
			   case {IsDet Data} then
			      case Data == unit then 'unknown'
			      else {System.printName Data}
			      end
			   else '_'   %--** should be clickable!
			   end
			[] 'lock' then 'lock'
			[] handler then 'exception handler'
			[] cond then 'conditional'
			[] exception then 'exception'
			end
		     elseof Name then Name
		     end
	    args:    case Frame.kind == 'lock' then [Frame.data]
		     else {CondSelect Frame args unit}
		     end
	    frameID: {CondSelect Frame frameID unit}
	    vars:    {CondSelect Frame vars unit})
   end

   proc {StackToDict Frames D}
      {List.forAllInd Frames
       proc {$ Key Frame}
	  {Dput D Key {S2F Key Frame}}
       end}
   end

in

   class StackManager

      prop
	 locking

      feat
	 T                 % the thread...
	 I                 % ...with it's ID
	 D                 % dictionary for stackframes

      attr
	 Size              % current size of stack
	 Rebuild           % should we re-calculate the stack
			   % when the next 'step' message arrives?

	 Exception : nil   % saved exception

	 New : true        % the thread has not made any step yet...

      meth init(thr:Thr id:ID)
	 self.T = Thr
	 self.I = ID
	 self.D = {Dictionary.new}
	 Size    <- 0
	 Rebuild <- false
      end

      meth checkNew(R)
	 R = @New
	 case R then New <- false else skip end
      end

      meth getFrame(Nr $)
	 S = @Size
	 N = case     Nr == ~1  then S
	     elsecase Nr < 1    then 1
	     elsecase Nr > S    then S
	     else                    Nr end
      in
	 try
	    {Dget self.D N}
	 catch
	    system(kernel(dict ...) ...) then nil
	 end
      end

      meth rebuild(Flag)
	 Rebuild <- Flag
      end

      meth getException($)
	 @Exception
      end

      meth printException(X)
	 Status
      in
	 case {HasFeature X debug} andthen {HasFeature X.debug stack} then
	    Stack = X.debug.stack
	    F#L#C#Time = case Stack of Frame|_ then
			    {CondSelect Frame file nofile}#
			    case X of error(kernel(noElse Line ...) ...) then
			       %% correct the line number
			       Line
			    else
			       {CondSelect Frame line unit}
			    end#
			    {CondSelect Frame column unit}#
			    Frame.time
			 else nofile#unit#unit#999999999
			 end
	    S = entry(kind: exception thr: self.T
		      file: F line: L column: C time: Time
		      args: [X]) | Stack
	 in
	    Status = {FormatExceptionLine {Error.formatExc X}}
	    {Ozcar status(Status clear BlockedThreadColor)}
	    {Ozcar bar(file:F line:L column:C state:blocked)}
	    StackManager,ReCalculate({Reverse S})

	 else              % no stack available
	    E = {V2VS X}
	 in
	    Status = UserExcText # E # NoStackText
	    {Ozcar status(Status clear BlockedThreadColor)}
	    {Ozcar removeBar}
	    StackManager,ReCalculate(nil)
	 end

	 Exception <- Status
      end

      meth print
	 case @Rebuild then
	    StackManager,ReCalculate
	 else
	    Frames = {Ditems self.D}
	    Depth  = @Size
	    Last   = case Depth > 0 then {Dget self.D Depth} else nil end
	 in
	    {Ozcar printStack(id:self.I frames:Frames depth:Depth last:Last)}
	    case {CheckState self.T} == running then
	       {Ozcar markStack(inactive)}
	    else
	       {Ozcar markStack(active)}
	    end
	 end
      end

      meth getTop($)
	 S = @Size
      in
	 case S == 0 then nil else {Dget self.D S} end
      end

      meth printTop
	 case @Rebuild then
	    StackManager,ReCalculate
	 else
	    S = @Size
	 in
	    case S == 0 then skip else
	       TopFrame = {Dget self.D S}
	    in
	       {Ozcar printStackFrame(frame:TopFrame delete:true)}
	    end
	 end
      end

      meth entry(Frame)
	 S = @Size
	 Key = case S == 0 orelse {Dget self.D S}.dir == entry then S + 1
	       else S
	       end
      in
	 Size <- Key
	 {Dput self.D Key {S2F Key Frame}}
      end

      meth exit(Frame)
	 S = @Size
      in
	 case S == 0 then
	    {OzcarError 'internal stack inconsistency; recalculating stack'}
	    StackManager, ReCalculate
	 else
	    Key = case {Dget self.D S}.dir == entry then S
		  else
		     {Dremove self.D S}
		     S - 1
		  end
	 in
	    Size <- Key
	    case Key > 0 then
	       {Dput self.D Key {S2F Key Frame}}
	    else
	       {Thread.resume self.T}
	    end
	 end
      end

      %% local helpers %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth GetStack($)
	 {Reverse {Thread.taskStack self.T MaxStackSize false}}
      end

      meth RemoveAllFrames
	 {DremoveAll self.D}
      end

      meth ReCalculate(S<=noStack)
	 CurrentStack = case S == noStack then StackManager,GetStack($)
			else S end
      in
	 {OzcarMessage 're-calculating stack of thread #' # self.I}
	 {Ozcar removeSkippedProcs(self.I)}
	 StackManager,rebuild(false)
	 StackManager,RemoveAllFrames
	 {StackToDict CurrentStack self.D}
	 Size <- {Length {Dkeys self.D}}
	 StackManager,print
      end

      %% access methods %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth getThread($)
	 self.T
      end

      meth getId($)
	 self.I
      end

      meth getSize($)
	 @Size
      end

      meth getPos(file:?F line:?L column:?C)
	 S = @Size
      in
	 case S == 0 then
	    F = nofile
	    L = unit
	    C = unit
	 else
	    TopFrame = {Dget self.D S}
	 in
	    F = TopFrame.file
	    L = TopFrame.line
	    C = TopFrame.column
	 end
      end

   end
end

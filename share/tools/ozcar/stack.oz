%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
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

      fun {FormatItems B}
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
	 {FormatItems {CondSelect E items nil}}
      end
   end

   fun {S2F Nr Frame}
      Data = {CondSelect Frame data unit}
      Kind#Granul = case Frame.kind of 'call/c' then 'call'#coarse
		    [] 'call/f' then 'call'#fine
		    [] 'lock/c' then 'lock'#coarse
		    [] 'lock/f' then 'lock'#fine
		    [] 'exception handler/c' then 'exception handler'#coarse
		    [] 'exception handler/f' then 'exception handler'#fine
		    [] 'conditional/c' then 'conditional'#coarse
		    [] 'conditional/f' then 'conditional'#fine
		    [] 'definition/c' then 'definition'#coarse
		    [] 'definition/f' then 'definition'#fine
		    [] 'skip/c' then 'skip'#coarse
		    [] 'skip/f' then 'skip'#fine
		    [] 'fail/c' then 'fail'#coarse
		    [] 'fail/f' then 'fail'#fine
		    [] 'thread/c' then 'thread'#coarse
		    [] 'thread/f' then 'thread'#fine
		    elseof K then K#unknown
		    end
   in
      frame(nr:      Nr   % frame counter
	    dir:     {Label Frame}   % 'entry' or 'exit'
	    file:    {CondSelect Frame file nofile}
	    line:    {CondSelect Frame line unit}
	    column:  {CondSelect Frame column unit}
	    time:    Frame.time
	    name:    case {CondSelect Frame name unit} of unit then
			case Kind == 'call' then
			   case {IsDet Data} then
			      case Data == unit then 'unknown'
			      elsecase {IsProcedure Data} then
				 {System.printName Data}
			      else
				 {System.valueToVirtualString Data 0 0}
			      end
			   elsecase {IsLazy Data} then
			      LazyVarType
			   else
			      UnboundType
			   end
			else Kind
			end
		     elseof Name then Name
		     end
	    kind:    Kind
	    granul:  Granul
	    data:    Data
	    args:    case Kind of 'lock' then [Frame.data]
		     [] 'conditional' then
			case {IsDet Data} andthen Data == unit then unit
			else [Data]
			end
		     else
			{CondSelect Frame args unit}
		     end
	    frameID: {CondSelect Frame frameID unit}
	    vars:    {CondSelect Frame vars unit})
   end

   proc {StackToDict Frames D}
      {List.forAllInd Frames
       proc {$ Key Frame}
	  {Dictionary.put D Key {S2F Key Frame}}
       end}
   end

in

   class StackManager

      feat
	 T                 % the thread...
	 I                 % ...with it's ID
	 D                 % dictionary for stackframes

      attr
	 Size              % current size of stack
	 Rebuild           % should we re-calculate the stack
			   % when the next 'step' message arrives?

	 Exception : nil   % saved exception

	 Step : 1
	 Next : 1

      meth init(thr:Thr id:ID)
	 self.T = Thr
	 self.I = ID
	 self.D = {Dictionary.new}
	 Size    <- 0
	 Rebuild <- false
      end

      meth incStep($)
	 Step <- @Step+1
      end

      meth incNext($)
	 Next <- @Next+1
      end

      meth getFrame(Nr $)
	 S = @Size
	 N = case     Nr == ~1  then S
	     elsecase Nr < 1    then 1
	     elsecase Nr > S    then S
	     else                    Nr end
      in
	 {Dictionary.condGet self.D N unit}
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
	    {Ozcar PrivateSend(status(Status clear BlockedThreadColor))}
	    {SendEmacs bar(file:F line:L column:C state:blocked)}
	    StackManager,ReCalculate({Reverse S})

	 else              % no stack available
	    E = {V2VS X}
	 in
	    Status = 'Exception: ' # E # ' / no stack available'
	    {Ozcar PrivateSend(status(Status clear BlockedThreadColor))}
	    {SendEmacs removeBar}
	    StackManager,ReCalculate(nil)
	 end

	 Exception <- Status
      end

      meth print
	 State = {CheckState self.T}
      in
	 case @Rebuild andthen State \= running then
	    {OzcarMessage 'stack,print: rebuild flag detected'}
	    StackManager,ReCalculate
	 else
	    Frames = {Dictionary.items self.D}
	    Depth  = @Size
	    Last   = case Depth > 0 then {Dictionary.get self.D Depth}
		     else nil
		     end
	 in
	    {Ozcar PrivateSend(printStack(id:self.I frames:Frames
					  depth:Depth last:Last))}
	    case State == running then
	       {Ozcar PrivateSend(markStack(inactive))}
	    else
	       {Ozcar PrivateSend(markStack(active))}
	    end
	 end
      end

      meth getTop($)
	 S = @Size
      in
	 case S == 0 then unit else {Dictionary.get self.D S} end
      end

      meth emacsBarToTop
	 F = StackManager,getTop($)
      in
	 case F == unit then skip else
	    S = {CheckState self.T}
	 in
	    {SendEmacs bar(file:F.file line:F.line column:F.column state:S)}
	 end
      end

      meth printTop
	 case @Rebuild then
	    {OzcarMessage 'stack,printTop: rebuild flag detected'}
	    StackManager,ReCalculate
	 else
	    S = @Size
	 in
	    case S == 0 then skip else
	       TopFrame = {Dictionary.get self.D S}
	    in
	       {Ozcar PrivateSend(printStackFrame(frame:TopFrame delete:true))}
	    end
	 end
      end

      meth entry(Frame)
	 S = @Size
	 Key = case S == 0 orelse {Dictionary.get self.D S}.dir == entry then
		  S + 1
	       else
		  S
	       end
      in
	 Size <- Key
	 {Dictionary.put self.D Key {S2F Key Frame}}
      end

      meth CountFramesWithoutDebug(I $)
	 case I == 0 then
	    0
	 else
	    Frame = {Dictionary.get self.D I}
	 in
	    case Frame.kind == call andthen Frame.args == unit then
	       StackManager, CountFramesWithoutDebug(I - 1 $) + 1
	    else
	       0
	    end
	 end
      end

      meth exit(Frame)
	 S = @Size
      in
	 case S == 0 then
	    {OzcarError 'internal stack inconsistency; recalculating stack'}
	    StackManager, ReCalculate
	 else
	    Key = case {Dictionary.get self.D S}.dir == entry then S
		  else N in
		     StackManager,CountFramesWithoutDebug(S - 1 ?N)
		     {For 0 N 1
		      proc {$ I}
			 {Dictionary.remove self.D S - I}
		      end}
		     S - (N + 1)
		  end
	 in
	    Size <- Key
	    case Key > 0 then
	       {Dictionary.put self.D Key {S2F Key Frame}}
	    else
	       {OzcarError 'internal stack inconsistency; resuming thread'}
	       {Thread.resume self.T}
	    end
	 end
      end

      %% local helpers %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth GetStack($)
	 {Reverse {Thread.taskStack self.T ~1 false}}
      end

      meth RemoveAllFrames
	 {Dictionary.removeAll self.D}
      end

      meth ReCalculate(S<=noStack)
	 CurrentStack = case S == noStack then StackManager,GetStack($)
			else S end
      in
	 {OzcarMessage 'recalculating stack of thread #' # self.I}
	 {Ozcar PrivateSend(removeSkippedProcs(self.I))}
	 StackManager,rebuild(false)
	 StackManager,RemoveAllFrames
	 {StackToDict CurrentStack self.D}
	 Size <- {Length {Dictionary.keys self.D}}
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
	    TopFrame = {Dictionary.get self.D S}
	 in
	    F = TopFrame.file
	    L = TopFrame.line
	    C = TopFrame.column
	 end
      end

   end
end

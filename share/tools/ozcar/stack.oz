%%%
%%% Authors:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%   Leif Kornstaedt, 2001
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

local

   local
      FDelimiter    = ' / '
      LRDelimiter   = ': '

      fun {FormatFeature R F D}
	 case {CondSelect R F unit} of unit then nil
	 elseof X then {Error.extendedVSToVS X} # D
	 end
      end

      fun {FormatLR L}
	 Left  = {FormatFeature L l nil}
	 Right = {FormatFeature L m nil}
	 Both  = Left \= nil andthen Right \= nil
      in
	 Left # if Both then LRDelimiter else nil end # Right
      end

      fun {CheckNil A}
	 if A == nil then nil else A # FDelimiter end
      end

      fun {FormatItems B}
	 if B == nil then nil else
	    {FoldL B
	     fun {$ A L}
		case L
		of hint(...)  then {CheckNil A} # {FormatLR L}
		[] pos(X Y _) then {CheckNil A} # 'Pos: ' # X # ', line ' # Y
		[] line(X)    then {CheckNil A} # {Error.extendedVSToVS X}
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

   AtomToKindAndGranul = {NewDictionary}

   fun {S2F Nr Frame}
      Data = {CondSelect Frame data unit}
      Kind Granul
   in
      case {Dictionary.condGet AtomToKindAndGranul Frame.kind unit}
      of unit then K Rest in
	 {List.takeDropWhile {Atom.toString Frame.kind} fun {$ C} C \= &/ end
	  ?K ?Rest}
	 Kind = {String.toAtom K}
	 Granul = case Rest of "/c" then coarse
		  elseof "/f" then fine
		  else unknown
		  end
	 {Dictionary.put AtomToKindAndGranul Frame.kind Kind#Granul}
      elseof K#G then
	 Kind = K
	 Granul = G
      end
      frame(nr:      Nr   % frame counter
	    dir:     {Label Frame}   % 'entry' or 'exit'
	    file:    {CondSelect Frame file ''}
	    line:    {CondSelect Frame line unit}
	    column:  {CondSelect Frame column unit}
	    name:    if Kind == 'call' then
			case {Value.status Data}
			of det(procedure) then {System.printName Data}
			[] det(atom) then Data
			[] free then {System.printName Data}
			[] future then {System.printName Data}
			else 'unknown'
			end
		     else Kind
		     end
	    kind:    Kind
	    granul:  Granul
	    data:    Data
	    args:    {CondSelect Frame args unit}
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
	 T                    % the thread...
	 I                    % ...with it's ID
	 D                    % dictionary for stackframes

      attr
	 Size                 % current size of stack
	 Rebuild              % should we re-calculate the stack
			      % when the next 'step' message arrives?

	 AtBreakpoint: false  % currently stopped at a breakpoint?
	 Exception: unit      % saved exception
	 SkippedProcs: nil    % list of frame IDs suppressed for display

      meth init(Thr)
	 self.T = Thr
	 self.D = {Dictionary.new}
	 Size    <- 0
	 Rebuild <- false
      end

      meth addToSkippedProcs(FrameID)
	 SkippedProcs <- FrameID|@SkippedProcs
      end
      meth isSkippedProc(FrameID $)
	 {Member FrameID @SkippedProcs}
      end
      meth removeSkippedProc(FrameID)
	 SkippedProcs <- {Filter @SkippedProcs fun {$ F} F \= FrameID end}
      end
      meth ClearSkippedProcs()
	 SkippedProcs <- nil
      end

      meth atBreakpoint($)
	 @AtBreakpoint
      end

      meth setAtBreakpoint(YesNo)
	 AtBreakpoint <- YesNo
      end

      meth getFrame(Nr $)
	 S = @Size
	 N = if     Nr == ~1  then S
	     elseif Nr < 1    then 1
	     elseif Nr > S    then S
	     else                  Nr end
      in
	 {Dictionary.condGet self.D N unit}
      end

      meth rebuild(Flag)
	 Rebuild <- Flag
      end

      meth setException(X)
	 if {IsDet X} andthen {IsRecord X} andthen {HasFeature X debug}
	    andthen {IsDet X.debug} andthen {IsRecord X.debug}
	    andthen {HasFeature X.debug stack}
	 then
	    Stack = X.debug.stack
	    F#L#C = case X of error(kernel(noElse F L ...) ...) then F#L#~1
		    elsecase Stack of Frame|_ then
		       {CondSelect Frame file ''}#
		       {CondSelect Frame line unit}#
		       {CondSelect Frame column ~1}
		    [] nil then ''#unit#~1
		    end
	    NewStack = entry(kind:exception thr:self.T
			     file:F line:L column:C args:[X])|Stack
	 in
	    Exception <- {FormatExceptionLine {Error.exceptionToMessage X}}
	    {SendEmacsBar F L C stoppedBlocked}
	    StackManager,Recalculate(NewStack)
	 else
	    Exception <- 'Exception: ' # {V2VS X} # ' / no stack available'
	    {SendEmacs removeBar}
	    StackManager,Recalculate(nil)
	 end
      end

      meth getException($)
	 @Exception
      end

      meth print
	 Running = case {Primitives.threadState self.T}
		   of runnable then true
		   [] blocked  then true
		   else false
		   end
      in
	 if @Rebuild andthen {Not Running} then
	    {OzcarMessage 'stack,print: rebuild flag detected'}
	    StackManager,Recalculate(unit)
	 else
	    Frames = {Dictionary.items self.D}
	    Depth  = @Size
	    Last   = if Depth > 0 then {Dictionary.get self.D Depth}
		     else unit
		     end
	 in
	    {Ozcar PrivateSend(printStack(thr:self.T frames:Frames
					  depth:Depth last:Last))}
	    if Running then
	       {Ozcar PrivateSend(markStack(inactive))}
	    else
	       {Ozcar PrivateSend(markStack(active))}
	    end
	 end
      end

      meth getTop($)
	 case @Size of 0 then unit
	 elseof S then {Dictionary.get self.D S}
	 end
      end

      meth emacsBarToTop
	 case StackManager,getTop($) of unit then skip
	 elseof F then
	    {SendEmacsBar F.file F.line F.column
	     {Primitives.threadState self.T}}
	 end
      end

      meth printTop
	 if @Rebuild then
	    {OzcarMessage 'stack,printTop: rebuild flag detected'}
	    StackManager,Recalculate(unit)
	 elsecase StackManager,getTop($) of unit then skip
	 elseof F then
	    {Ozcar PrivateSend(printStackFrame(frame:F delete:true))}
	 end
      end

      meth entry(Frame)
	 S = @Size
	 Key = if S == 0 then 1
	       elseif {Dictionary.get self.D S}.dir == entry then S + 1
	       else S
	       end
      in
	 Size <- Key
	 {Dictionary.put self.D Key {S2F Key Frame}}
      end

      meth exit(Frame)
	 case @Size of 0 then
	    {OzcarError 'internal stack inconsistency; recalculating stack'}
	    StackManager,Recalculate(unit)
	 elseof S then
	    if {Dictionary.get self.D S}.dir == entry then
	       {Dictionary.put self.D S {S2F S Frame}}
	    else
	       {Dictionary.remove self.D S}
	       Size <- StackManager,RemoveFramesWithoutDebug(S - 1 $)
	       case @Size of 0 then
		  {OzcarError 'internal stack inconsistency; resuming thread'}
		  {Primitives.unleash self.T 0 false}
	       elseof Key then
		  {Dictionary.put self.D Key {S2F Key Frame}}
	       end
	    end
	 end
      end

      meth RemoveFramesWithoutDebug(I $)
	 case I of 0 then 0
	 else Frame in
	    Frame = {Dictionary.get self.D I}
	    if Frame.kind == call andthen Frame.args == unit then
	       {Dictionary.remove self.D I}
	       StackManager,RemoveFramesWithoutDebug(I - 1 $)
	    else I
	    end
	 end
      end

      %% local helpers %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth Recalculate(S)
	 CurrentStack = {Reverse case S of unit then
				    {Primitives.getStack self.T ~1 false}
				 else S
				 end}
      in
	 {OzcarMessage
	  'recalculating stack of thread ' # {Primitives.getThreadName self.T}}
	 StackManager,ClearSkippedProcs()
	 StackManager,rebuild(false)
	 {Dictionary.removeAll self.D}
	 {StackToDict CurrentStack self.D}
	 Size <- {Length {Dictionary.keys self.D}}
	 StackManager,print
      end

      %% access methods %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth getThread($)
	 self.T
      end

      meth getSize($)
	 @Size
      end

      meth getPos(file:?F line:?L column:?C)
	 case @Size of 0 then
	    F = ''
	    L = unit
	    C = ~1
	 elseof S then TopFrame in
	    TopFrame = {Dictionary.get self.D S}
	    F = TopFrame.file
	    L = TopFrame.line
	    C = TopFrame.column
	 end
      end

   end
end

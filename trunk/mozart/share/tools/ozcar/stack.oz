%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   
   fun {B2F Nr B}
      frame(nr      : Nr
	    id      : 0
	    file    : ''
	    line    : 0
	    time    : 0
	    name    : B.name
	    args    : B.args
	    builtin : true)
   end

   fun {P2F Nr P D}
      frame(nr      : Nr
	    id      : 0
	    file    : P.file
	    line    : P.line
	    time    : 0
	    name    : P.name
	    args    : D.1.2
	    builtin : false)
   end

   fun {S2F Nr Id File Line Time Name Args Builtin}
      frame(nr      : Nr
	    id      : Id
	    file    : File
	    line    : Line
	    time    : Time
	    name    : Name
	    args    : Args
	    builtin : Builtin)
   end
   
   local
      fun {Correct F}
	 case F == nil then nil else
	    case {Label F.1} == debug then
	       {Correct F.2}
	    else
	       F
	    end
	 end
      end
      proc {DoStackForAllInd Xs I P}
	 case Xs of nil then skip
	 [] X|Y|Z then
	    case X == toplevel then skip else
	       {P I X Y}
	       {DoStackForAllInd {Correct Z} I+1 P}
	    end
	 end
      end
   in
      proc {StackForAllInd Xs P}
	 case {Label Xs.1} == builtin then
	    {P 1 Xs.1 nil}
	    {DoStackForAllInd Xs.2 2 P}
	 else
	    {DoStackForAllInd Xs 1 P}
	 end
      end
   end
   
in
   
   class StackManager

      prop
	 locking
      
      feat
	 T              % the thread...
	 I              % ...with it's ID
	 D              % dictionary for stackframes

      attr
	 Size           % current size of stack
      
      meth init(thr:Thr id:ID)
	 self.T = Thr
	 self.I = ID
	 self.D = {Dictionary.new}
	 Size <- 0
      end

      meth step(name:N args:A builtin:B file:F line:L time:T frame:FrameId)
	 OldSize = @Size
	 Frame   = {S2F OldSize+1 FrameId F L T N A B}
      in
	 {Dput self.D OldSize Frame}
	 {Ozcar printStackFrame(frame:Frame direction:enter)}
	 Size <- OldSize + 1
      end

      meth exit(FrameId)
	 Key   = @Size - 1
	 Frame = {Dget self.D Key}
      in
	 {Ozcar printStackFrame(frame:Frame direction:leave)}
	 Size <- Key
      end
      
      meth clear
	 %% clear stack widget
	 skip
	 %% clear env widgets
	 skip
      end

      %% local helpers %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      meth GetStack($)
	 {Dbg.taskstack self.T MaxStackSize}
      end
      
      %% access methods %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
      
      meth getThread($)
	 self.T
      end

      meth getPos(file:?F line:?L)
	 F = undef
	 L = 0
      end
	 
   end
end

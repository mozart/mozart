%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   
   fun {B2F I B}
      frame(nr      : I
	    file    : ''
	    line    : 0
	    name    : B.name
	    args    : B.args
	    env     : undef
	    builtin : true)
   end

   fun {P2F I P D}
      frame(nr      : I
	    file    : P.file
	    line    : P.line
	    name    : P.name
	    args    : D.1.2
	    env     : P.vars
	    builtin : false)
   end
       
   local
      proc {DoStackForAllInd Xs I P}
	 case Xs of nil then skip
	 [] X|nil then {P I nil X}
	 [] X|Y|Z then {P I X Y} {DoStackForAllInd Z I+1 P}
	 end
      end
   in
      proc {StackForAllInd Xs P}
	 case {Label Xs.2.1} == toplevel then
	    {DoStackForAllInd Xs.2.2 1 P}
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
	 W              % text widget for output
	 D              % dictionary for stackframes

      attr
	 Size           % current size of stack
      
      meth init(thr:Thr id:ID output:TextWidget)
	 self.T = Thr
	 self.I = ID
	 self.W = TextWidget
	 self.D = {Dictionary.new}
	 Size <- 0
      end

      meth GetStack($)
	 {Dbg.taskstack self.T 40}
      end
      
      meth Reset
	 CurrentStack = StackManager,GetStack($)
	 OldKeys      = {Dkeys self.D}
      in
	 lock
	    {ForAll OldKeys proc {$ K} {Dremove self.D K} end}
	    case CurrentStack \= nil then
	       {StackForAllInd CurrentStack
		proc {$ Ind Debug Proc}
		   case Debug == nil then  % builtin
		      {Dput self.D 0         {B2F Ind Proc}}
		   else                    % procedure
		      {Dput self.D Debug.1.1 {P2F Ind Proc Debug}}
		   end
		end}
	       Size <- {Length {Dkeys self.D}}
	    else
	       Size <- 0
	    end
	 end
      end
      
      %% completely re-print the stack
      meth print
	 StackManager,Reset
	 local
	    S = @Size
	 in
	    case S > 0 then
	       {Ozcar printStack(id:self.I size:S stack:self.D)}
	    else
	       StackManager,clear
	    end
	 end
      end
      
      meth Clear
	 {ForAll [tk(conf state:normal)
		  tk(delete '0.0' 'end')] self.W}
      end

      meth Enable
	 {self.W tk(conf state:normal)}
      end
      
      meth Disable
	 {self.W tk(conf state:disabled)}
      end
      
      meth clear
	 %% clear stack widget
	 StackManager,Clear
	 StackManager,Disable
	 %% clear env widgets
	 {Ozcar printEnv(frame:0)}
      end
      
   end
end

%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   
   fun {IsDefList X}
      S = {Space.new fun {$} {IsList X} end}
      V = thread {Space.askVerbose S} end
   in
      case V
      of succeeded(entailed)
      then
	 {Space.merge S}
      [] blocked(_)
      then
	 {Space.inject S proc {$ _} fail end}
	 false
      end
   end
   
   fun {FormatArgs A}
      {Map A
       fun {$ X}
	  {ArgType X} # X
       end}
   end
   
   fun {ArgType X}
      case {IsDet X} then
	 case     {IsArray X}      then ArrayType
	 elsecase {IsThread X}     then ThreadType
	 elsecase {IsAtom X}       then case X
					of 'nil'         then NilAtom
					[] '|'           then ConsAtom
					[] '#'           then HashAtom
					[] 'unallocated' then UnAllocatedType
					else                  '\'' # X # '\''
					end
	 elsecase {IsBool X}       then case X then TrueName else FalseName end
	 elsecase {IsCell X}       then CellType
	 elsecase {IsClass X}      then ClassType
	 elsecase {IsDictionary X} then DictionaryType
	 elsecase {IsFloat X}      then FloatType
	 elsecase {IsInt X}        then MagicAtom
	 elsecase {IsUnit  X}      then UnitType
	 elsecase {IsName X}       then NameType
	 elsecase {IsLock X}       then LockType
	 elsecase {IsObject X}     then ObjectType
	 elsecase {IsPort X}       then PortType
	 elsecase {IsProcedure X}  then ProcedureType
	 elsecase {IsDefList X}    then ListType
	 elsecase {IsTuple X}      then TupleType
	 elsecase {IsRecord X}     then RecordType
	 elsecase {IsChunk X}      then ChunkType
	 else                           UnknownType
	 end
      else                              UnboundType
      end
   end
   
   TagCounter =
   {New class 
	   attr n
	   meth clear n<-1000 end  % low integers are reserved for
	                           % stack frame clicks
	   meth get($) N=@n in n<-N+1 N end
	end clear}
   
   fun {MakeLines N}
      case N < 1 then nil
      else 10 | {MakeLines N-1} end
   end
   
   Lck =
   {New class
	   attr
	      L : false
	   meth init skip end
	   meth set   L <- true  end
	   meth unset L <- false end
	   meth is($) @L end
	end init}
   
in

   class Gui from Menu Dialog

      prop
	 locking
      
      feat
	 toplevel
	 menuBar
      
	 ButtonFrame
      
	 ThreadTree
	 StackText
	 GlobalEnvText
	 LocalEnvText
      
	 StatusFrame
	 StatusText

      attr
	 LastSelectedFrame : undef

      meth init
	 %% create the main window, but delay showing it
	 self.toplevel = {New Tk.toplevel tkInit(title:TitleName
						 withdraw:true)}
	 {Tk.batch [wm(iconname   self.toplevel IconName)
		    wm(iconbitmap self.toplevel BitMap)
		    wm(geometry   self.toplevel ToplevelGeometry)]}
	 
	 Menu,init
	 Dialog,init

	 {ForAll [self.ButtonFrame self.StatusFrame]
	  proc{$ F}
	     F = {New Tk.frame tkInit(parent:self.toplevel
				      bd:BorderSize
				      relief:ridge)}
	  end}
      
	 {Tk.batch [grid(self.menuBar       row:0 column:0
			 sticky:we columnspan:3)
		    grid(self.ButtonFrame   row:1 column:0
			 sticky:we columnspan:3)
		    grid(self.StatusFrame   row:6 column:0
			 sticky:we columnspan:3)
		   ]}
      
	 %% the buttons
	 local
	    %% Tk has some problems printing centered text :-(
	    Bs = {Map [' step' ' next' /* ' finish' */ ' cont' ' forget'
		       ' stack' ]
		  fun {$ B}
		     {New Tk.button tkInit(parent: self.ButtonFrame
					   text:   B
					   padx:   PadXButton
					   pady:   PadYButton
					   font:   ButtonFont
					   borderwidth: SmallBorderSize
					   action: self # action(B))}
		  end}
	    TkSusp = {New Tk.variable tkInit(0)} % emulator default
	    Susp   = {New Tk.checkbutton
		      tkInit(parent:    self.ButtonFrame
			     variable:  TkSusp
			     text:      'Suspend'
			     relief:    raised
			     font:      ButtonFont
			     padx:      PadXButton
			     pady:      PadYButton
			     borderwidth: SmallBorderSize
			     action:    self # suspend(TkSusp))}
	 in
	    {Tk.batch [pack(b(Bs) side:left  padx:1)
		       pack(Susp  side:right padx:2)]}
	 end
      
	 %% status line
	 self.StatusText =
	 {New Tk.text tkInit(parent: self.StatusFrame
			     state:  disabled
			     height: 1
			     width:  0
			     bd:     0
			     cursor: TextCursor
			     font:   BoldFont)}
	 {Tk.send pack(self.StatusText side:left padx:2 fill:x expand:yes)}

	 %% create the thread tree object...
	 self.ThreadTree =
	 {New Tree tkInit(parent: self.toplevel
			  title:  TreeTitle
			  width:  ThreadTreeWidth
			  bg:     DefaultBackground)}
	 %% ...and the text widgets for stack and environment
	 {ForAll [self.StackText       # StackTitle      # StackTextWidth
		  self.LocalEnvText    # LocalEnvTitle   # EnvTextWidth
		  self.GlobalEnvText   # GlobalEnvTitle  # EnvTextWidth ]
	  proc {$ T}
	     T.1 = {New ScrolledTitleText tkInit(parent: self.toplevel
						 title:  T.2
						 wrap:   none
						 state:  disabled
						 width:  T.3
						 bd:     SmallBorderSize
						 cursor: TextCursor
						 font:   DefaultFont
						 bg:     DefaultBackground)}
	  end}
	 {Tk.batch [grid(self.ThreadTree    row:3 column:0
			 sticky:nswe rowspan:2)
		    grid(self.StackText     row:3 column:1 sticky:nswe
			 columnspan:2)
		    grid(self.LocalEnvText  row:4 column:1 sticky:nswe)
		    grid(self.GlobalEnvText row:4 column:2 sticky:nswe)
		    grid(rowconfigure       self.toplevel 3 weight:1)
		    grid(rowconfigure       self.toplevel 4 weight:1)
		    grid(columnconfigure    self.toplevel 0 weight:1)
		    grid(columnconfigure    self.toplevel 1 weight:1)
		    grid(columnconfigure    self.toplevel 2 weight:1)
		   ]}
      end

      meth DoPrintEnv(Widget Vars CV CP)
	 {ForAll Vars
	  proc{$ V}
	     AT = {ArgType V.2}
	  in
	     case CV orelse {Atom.toString V.1}.1 \= 96 then
		case CP orelse AT \= ProcedureType then
		   case AT == MagicAtom then
		      {Widget tk(insert 'end'
				 {PrintF ' '#V.1 EnvVarWidth} # V.2 # NL)}
		   else
		      T = {TagCounter get($)}
		      Ac = {New Tk.action
			    tkInit(parent: Widget
				   action: proc {$} {Browse V.2} end)}
		   in
		      {ForAll [tk(insert 'end' {PrintF ' ' # V.1 EnvVarWidth})
			       tk(insert 'end' AT # NL T)
			       tk(tag bind T '<1>' Ac)
			       tk(tag conf T font:BoldFont)] Widget}
		   end
		else skip end
	     else skip end
	  end}
      end

      meth printEnv(frame:I vars:V<=nil)
	 CV = {Not {Cget envSystemVariables}}
	 CP = {Not {Cget envProcedures}}
      in
	 Gui,Clear(self.LocalEnvText)
	 Gui,Clear(self.GlobalEnvText)
      
	 case V == nil then
	    skip
	 else
	    Gui,DoPrintEnv(self.LocalEnvText  V.'Y' CV CP)
	    Gui,DoPrintEnv(self.GlobalEnvText V.'G' CV CP)
	 end
      
	 Gui,Disable(self.LocalEnvText)
	 Gui,Disable(self.GlobalEnvText)
      end
   
      meth frameClick(frame:F highlight:Highlight<=true)
	 L
      in
	 {Delay 70} % > TIME_SLICE
	 L = {Lck is($)}
	 case L then skip else
	    Ack
	    FrameId       = F.id
	    FrameNr       = F.nr
	    CurrentThread = ThreadManager,getCurrentThread($)
	    Vars          = {Dbg.frameVars CurrentThread FrameId}
	 in
	    {OzcarMessage 'Selecting frame #' # FrameId}
	    case Highlight then
	       thread
		  SourceManager,scrollbar(file:F.file line:{Abs F.line} ack:Ack
					  color:ScrollbarStackColor what:stack)
	       end
	       thread Gui,loadStatus(F.file Ack) end
	       Gui,SelectStackFrame(FrameNr)
	    else
	       Gui,SelectStackFrame(0)
	    end
	    Gui,printEnv(frame:FrameNr vars:Vars)
	    /*
	    case {Cget verbose} then
	       {Debug.displayCode F.'PC' 5}
	    else skip end
	    */
	 end
      end
   
      meth SelectStackFrame(T)
	 W   = self.StackText
	 LSF = @LastSelectedFrame
      in
	 case LSF \= T then
	    case LSF \= undef then
	       {W tk(tag conf LSF
		     relief:flat borderwidth:0
		     background: DefaultBackground
		     foreground: DefaultForeground)}
	    else skip end
	    case T > 0 then
	       {W tk(tag conf T
		     relief:raised borderwidth:0
		     background: SelectedBackground
		     foreground: SelectedForeground)}
	       LastSelectedFrame <- T
	    else
	       LastSelectedFrame <- undef
	    end
	 else skip end
      end

      meth printStackFrame(frame:Frame delete:Delete<=true)
	 W          = self.StackText
	 FrameNr    = Frame.nr                 % frame number
	 FrameName  = Frame.name               % procedure/builtin name
	 FrameArgs  = {FormatArgs Frame.args}  % argument list
	 FrameFile  = {StripPath  Frame.file}
	 FrameLine  = {Abs Frame.line}
	 LineTag    = FrameNr
	 LineAction =
	 {New Tk.action
	  tkInit(parent: W
		 action: Ozcar # frameClick(frame:Frame))}
	 LineEnd    = FrameNr # DotEnd
	 UpToDate   = SourceManager,isUpToDate(Frame.time $)
      in
	 
	 {OzcarMessage '  printing frame #' # FrameNr}

	 lock
	    case Delete then
	       Gui,Enable(W)
	       Gui,DeleteToEnd(W FrameNr+1)
	       Gui,DeleteLine(W FrameNr)
	    else skip end
	    
	    {W tk(insert LineEnd
		  case Frame.dir == enter then
		     ' -> '
		  else
		     ' <- '
		  end # FrameNr #
		  ' ' # BraceLeft #
		  case FrameName == '' then '$' else FrameName end
		  LineTag)}
	    
	    {ForAll FrameArgs
	     proc {$ Arg}
		case Arg.1 == MagicAtom then
		   {W tk(insert LineEnd ' ' # Arg.2 LineTag)}
		else
		   ArgTag    = {TagCounter get($)}
		   ArgAction =
		   {New Tk.action
		    tkInit(parent: W
			   action: proc {$}
				      {Lck set}
				      {Browse Arg.2}
				      {Delay 150}
				      {Lck unset}
				   end)}
		in
		   {ForAll [tk(insert LineEnd ' ' LineTag)
			    tk(insert LineEnd Arg.1 q(LineTag ArgTag))
			    tk(tag bind ArgTag '<1>' ArgAction)
			    tk(tag conf ArgTag font:BoldFont)] W}
		end
	     end}
	 
	    {ForAll [tk(insert LineEnd BraceRight #
			case UpToDate then nil else
			   ' (source has changed)' end #
			case Delete then NL else nil end
			LineTag)
		     tk(tag add  LineTag LineEnd) % extend tag to whole line
		     tk(tag bind LineTag '<1>' LineAction)] W}
	    
	    case Delete then
	       FrameDir = Frame.dir
	    in
	       Gui,Disable(W)
	       case FrameDir == enter then % should also work with 'leave' :-(
		  Gui,frameClick(frame:Frame highlight:false)
	       else skip end
	    else skip end
	 end
      end
	 
      meth printStack(id:I frames:Frames depth:Depth last:LastFrame<=nil)
	 W = self.StackText
      in
	 {OzcarMessage 'printing complete stack of size ' # Depth}
	 case I == 0 then
	    {W title(StackTitle)}
	    Gui,Clear(W)
	    Gui,Disable(W)
	    Gui,printEnv(frame:0)
	 else
	    {W title(AltStackTitle # I)}
	    Gui,Clear(W)
	    case Depth == 0 then
	       Gui,Disable(W)
	       Gui,printEnv(frame:0)
	    else
	       lock
		  Gui,Append(W {MakeLines Depth})  % Tk is _really_ stupid...
		  {ForAll Frames
		   proc{$ Frame}
		      Gui,printStackFrame(frame:Frame delete:false)
		   end}
		  Gui,Disable(W)
		  case LastFrame == nil then
		     {OzcarError 'printStack: LastFrame == nil ??'}
		  else
		     Gui,frameClick(frame:LastFrame highlight:false)
		  end
	       end
	    end
	 end
      end
      
      meth selectNode(I)
	 {self.ThreadTree select(I)}
      end
   
      meth markNode(I How)
	 {self.ThreadTree mark(I How)}
      end

      meth addNode(I Q)
	 {self.ThreadTree add(I Q)}
      end
   
      meth removeNode(I)
	 {self.ThreadTree remove(I)}
      end

      meth killNode(I)
	 {self.ThreadTree kill(I)}
      end

      meth displayTree
	 {self.ThreadTree display}
      end

      meth getStackText($)
	 self.StackText
      end
      
      meth status(I S<=nil)
	 W = self.StatusText
      in
	 Gui,Clear(W)
	 Gui,Append(W case I of
			 ~1 then StatusEndText
		      elseof
			 0  then StatusInitText
		      else
			 'Current Thread: #' # I # ' (' # S # ')'
		      end)
	 Gui,Disable(W)
      end
   
      meth rawStatus(S M<=clear)
	 W = self.StatusText
      in
	 case M == clear then
	    Gui,Clear(W)
	 else
	    Gui,Enable(W)
	 end
	 Gui,Append(W S)
	 Gui,Disable(W)
      end

      meth loadStatus(File Ack)
	 case {UnknownFile File} then skip else
	    {Delay TimeoutToMessage}
	    case {IsDet Ack} then skip else
	       RealFile = {LookupFile File}
	    in
	       Gui,rawStatus('Loading file ' # RealFile # '...')
	       {Wait Ack}
	       Gui,rawStatus(' done' append)
	    end
	 end
      end
      
      meth stackStatus(Size Ack)
	 {Delay TimeoutToMessage}
	 case {IsDet Ack} then skip else
	    Gui,rawStatus('Printing stack of size ' # Size # '...')
	    {Wait Ack}
	    Gui,rawStatus(' done' append)
	 end
      end
	    
      meth action(A)
	 T = @currentThread
	 I
      in
	 case T \= undef then
	    I = {Thread.id T}

	    case {Label A}
	    
	    of ' step' then
	       {Thread.resume T}
	    
	    elseof ' next' then
	       ThreadDic = ThreadManager,getThreadDic($)
	       TopFrame  = {{Dget ThreadDic I} getTop($)}
	       Dir = case TopFrame == nil then enter else TopFrame.dir end
	    in
	       case Dir == leave then
		  {OzcarMessage NextOnLeave}
	       else
		  {Dbg.stepmode T false}
	       end
	       {Thread.resume T}
	       
	    elseof ' finish' then
	       {Browse 'not yet implemented'}
	       skip
	    
	    elseof ' cont' then
	       {Dbg.stepmode T false}
	       {Dbg.contflag T true}
	       {Thread.resume T}
	    
	    elseof ' forget' then
	       ThreadManager,forget(T I)
	    
	    elseof ' stack' then  %% will go away, someday...
	       {Browse {Dbg.taskstack T MaxStackBrowseSize}}
	    end
	 
	 else skip end
      end

      meth Clear(Widget)
	 {ForAll [tk(conf state:normal)
		  tk(delete '0.0' 'end')] Widget}
      end
      
      meth Enable(Widget)
	 {Widget tk(conf state:normal)}
      end
      
      meth Append(Widget Text)
	 {Widget tk(insert 'end' Text)}
      end
      
      meth Disable(Widget)
	 {Widget tk(conf state:disabled)}
      end
      
      meth DeleteLine(Widget Nr)
	 {Widget tk(delete Nr#'.0' Nr#DotEnd)}
      end

      meth DeleteToEnd(Widget Nr)
	 {Widget tk(delete Nr#'.0' 'end')}
      end
   end
end

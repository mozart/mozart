%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

class Gui from Menu Dialog

   feat
      toplevel
      menuBar

      ButtonFrame

      ThreadTree
      StackText
      GlobalEnvText
      LocalEnvText
   
      ApplFrame
      ApplFileFrame
      ApplPrefix
      ApplText
      ApplFilePrefix
      ApplFileText
   
      StatusFrame
      StatusLabel

   attr
      LastSelectedFrame : undef

   meth init
      %% create the main window, but delay showing it
      self.toplevel = {New Tk.toplevel tkInit(title:TitleName withdraw:true)}
      {Tk.batch [wm(iconname   self.toplevel IconName)
		 wm(iconbitmap self.toplevel BitMap)
		 wm(geometry   self.toplevel ToplevelGeometry)]}

      Menu,init
      Dialog,init

      {ForAll [self.ButtonFrame   self.ApplFrame
	       self.ApplFileFrame self.StatusFrame]
       proc{$ F}
	  F = {New Tk.frame tkInit(parent:self.toplevel
				   bd:BorderSize
				   relief:ridge)}
       end}
      
      {Tk.batch [grid(self.menuBar       row:0 column:0 sticky:we columnspan:3)
		 grid(self.ButtonFrame   row:1 column:0 sticky:we columnspan:3)
		 grid(self.ApplFrame     row:2 column:0 sticky:we columnspan:2)
		 grid(self.ApplFileFrame row:2 column:2 sticky:we columnspan:1)
		 grid(self.StatusFrame   row:6 column:0 sticky:we columnspan:3)
		]}
      
      %% the buttons
      local
	 %% Tk has some problems printing centered text :-(
	 Bs = {Map [' step' ' next' /* ' finish' */ ' cont' ' forget'
		    /* ' stack' */ ]
	       fun {$ B}
		  {New Tk.button tkInit(parent:      self.ButtonFrame
					text:        B
					padx:        5
					pady:        3
					font:        ButtonFont
					borderwidth: 1
					action:      self # action(B))}
	       end}
      in
	 {Tk.send pack(b(Bs) side:left padx:1)}
      end
      
      %% application frame
      self.ApplPrefix = {New Tk.label tkInit(parent: self.ApplFrame
					     text:   ApplPrefixText)}
      self.ApplText   = {New Tk.text tkInit(parent: self.ApplFrame
					    height: 1
					    width:  0
					    bd:     SmallBorderSize
					    cursor: TextCursor
					    font:   DefaultFont
					    bg:     DefaultBackground)}
      self.ApplFilePrefix = {New Tk.label tkInit(parent: self.ApplFileFrame
						 text:   ApplFilePrefixText)}
      self.ApplFileText   = {New Tk.text tkInit(parent: self.ApplFileFrame
						height: 1
						width:  0
						bd:     SmallBorderSize
						cursor: TextCursor
						font:   DefaultFont
						bg:     DefaultBackground)}
      
      {Tk.batch [pack(self.ApplPrefix side:left)
		 pack(self.ApplText   side:left fill:x expand:yes)]}
      {Tk.batch [pack(self.ApplFilePrefix side:left)
		 pack(self.ApplFileText   side:left fill:x expand:yes)]}
      
      %% border line
      local
	 F = {New Tk.frame tkInit(parent:self.toplevel height:3
				  relief:ridge bd:1)}
      in
	 {Tk.send grid(F row:5 column:0 sticky:we columnspan:3)}
      end
      
      %% status line
      self.StatusLabel  = {New Tk.label tkInit(parent:self.StatusFrame
					       text:StatusInitText)}
      {Tk.send pack(self.StatusLabel side:left fill:x)}
      
      %% create the thread tree object...
      self.ThreadTree = {New Tree tkInit(parent: self.toplevel
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
					      state:  disabled
					      width:  T.3
					      bd:     SmallBorderSize
					      cursor: TextCursor
					      font:   DefaultFont
					      bg:     DefaultBackground)}
       end}
      {Tk.batch [grid(self.ThreadTree row:3 column:0
		      sticky:nswe rowspan:2)
		 grid(self.StackText     row:3 column:1 sticky:nswe
		                               columnspan:2)
		 grid(self.LocalEnvText  row:4 column:1 sticky:nswe)
		 grid(self.GlobalEnvText row:4 column:2 sticky:nswe)
		 grid(rowconfigure       self.toplevel 3 weight:1)
		 grid(rowconfigure       self.toplevel 4 weight:1)
		 grid(columnconfigure    self.toplevel 0 weight:4)
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
		T = {TagCounter get($)}
		Ac = {New Tk.action
		      tkInit(parent: Widget
			     action: proc{$}{Browse V.2}end)}
	     in
		{ForAll [tk(insert 'end' {PrintF ' ' # V.1 18})
			 tk(insert 'end' AT # NL T)
			 tk(tag bind T '<1>' Ac)
			 tk(tag conf T font:BoldFont)] Widget}
	     else skip end
	  else skip end
       end}
   end

   meth Clear(Widget)
      {ForAll [tk(conf state:normal)
	       tk(delete '0.0' 'end')] Widget}
   end

   meth Disable(Widget)
      {Widget tk(conf state:disabled)}
   end
   
   meth printEnv(frame:I vars:V<=undef)
      CV = {Not {Cget envSystemVariables}}
      CP = {Not {Cget envProcedures}}
   in
      case I == 0 then
	 {self.LocalEnvText title(LocalEnvTitle)}
	 {self.GlobalEnvText title(GlobalEnvTitle)}
      else
	 {self.LocalEnvText title(AltLocalEnvTitle # I)}
	 {self.GlobalEnvText title(AltGlobalEnvTitle # I)}
      end
      
      Gui,Clear(self.LocalEnvText)
      Gui,Clear(self.GlobalEnvText)
      
      case V == undef then
	 skip
      else
	 Gui,DoPrintEnv(self.LocalEnvText  V.'Y' CV CP)
	 Gui,DoPrintEnv(self.GlobalEnvText V.'G' CV CP)
      end
      
      Gui,Disable(self.LocalEnvText)
      Gui,Disable(self.GlobalEnvText)
   end
   
   meth frameClick(nr:I frame:F tag:T)
      Gui,SelectStackFrame(T)
      Gui,printEnv(frame:I vars:F.vars)
      SourceManager,scrollbar(file:F.file line:F.line
			      color:ScrollbarStackColor what:stack)
   end
   
   meth SelectStackFrame(T)
      W = self.StackText
   in
      case @LastSelectedFrame \= undef then
	 {W tk(tag conf @LastSelectedFrame
	       relief:flat borderwidth:0
	       background: DefaultBackground
	       foreground: DefaultForeground)}
      else skip end
      {W tk(tag conf T
	    relief:raised borderwidth:0  %% borderwidth:1 looks somehow ugly...
	    background: SelectedBackground
	    foreground: SelectedForeground)}
      LastSelectedFrame <- T
   end
   
   meth printStack(id:ThrID stack:Stack top:Top<=true)
      W = self.StackText
      S = {List.filter Stack fun{$ X}
				{Label X} == 'proc'
				% andthen X.name \= 'Toplevel abstraction'
			     end}
      SL = {List.length S}
   in
      case S == nil then
	 {ForAll [tk(conf state:normal)
		  tk(delete '0.0' 'end')
		  %tk(insert 'end' "  nil")
		  tk(conf state:disabled)] W}
	 {self.StackText title(StackTitle)}
	 Gui,printEnv(frame:0)
      else
	 {self.StackText title(AltStackTitle # ThrID)}
	 case Top orelse SL == 1 then
	    Gui,printEnv(frame:1 vars:S.1.vars)
	 else
	    Gui,printEnv(frame:2 vars:S.2.1.vars)
	 end
	 
	 {ForAll [tk(conf state:normal)
		  tk(delete '0.0' 'end')] W}
	 {List.forAllInd S
	  proc{$ I F}
	     T = {TagCounter get($)}
	     Ac = {New Tk.action
		   tkInit(parent: W
			  action: Ozcar # frameClick(nr:I frame:F tag:T))}
	  in
     	     {ForAll [tk(insert 'end'
		      {PrintF ' ' # I # ' ' # case F.name == ''
					      then '$' else F.name
					      end 35} #
		      {StripPath F.file} # FileLineSeparator # F.line # NL T)
		      tk(tag bind T '<1>' Ac)
		      tk(tag conf T font:BoldFont)] W}
	     case I == 1 andthen (Top orelse SL == 1)
		orelse I == 2 andthen {Not Top} then
		LastSelectedFrame <- undef
		Gui,SelectStackFrame(T)
	     else skip end
	  end}
	 {W tk(conf state:disabled)}
      end
   end
   
   meth printAppl(id:I name:N args:A builtin:B<=false time:Time<=0
		  file:F<=undef line:L<=undef)
      UpToDate = SourceManager,isUpToDate(Time $)
      W        = self.ApplText
   in
      case N == undef orelse A == undef then
	 Gui,Clear(W)
	 Gui,Disable(W)
      else
	 Args      = {FormatArgs A}
	 ApplColor = case B then BuiltinColor else ProcColor end 
	 T         = {TagCounter get($)}
      in
	 Gui,Clear(W)
	 {ForAll [tk(insert 'end' ' {')
		  tk(insert 'end' case N == '' then '$' else N end T)
		  tk(tag conf T foreground:ApplColor)] W}
	  
	 {ForAll Args
	  proc {$ A}
	     T = {TagCounter get($)}
	     Ac = {New Tk.action
		   tkInit(parent:W
			  action:proc{$}
				    S = A.3
				 in
				    {Browse S}
				 end)}
	  in
	     {ForAll [tk(insert 'end' ' ')
		      tk(insert 'end' A.2 T)
		      tk(tag bind T '<1>' Ac)
		      tk(tag conf T font:BoldFont)] W}
	  end}
	 {W tk(insert 'end' '}')}
	 Gui,Disable(W)
      end
      
      local
	 W = self.ApplFileText
      in
	 Gui,Clear(W)
	 case F \= undef then
	    S = ' ' # {StripPath F} # FileLineSeparator # L
	 in
	    case UpToDate then
	       {W tk(insert 'end' S)}
	       {OzcarMessage 'mtime ok.'}
	    else
	       T = {TagCounter get($)}
	    in
	       {W tk(insert 'end' S # '(?)' T)}
	       {W tk(tag conf T foreground:BuiltinColor)}
	       {OzcarMessage 'mtime NOT ok.'}
	    end 
	    
	 else skip end
	 Gui,Disable(W)
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

   meth status(I S<=nil)
      {self.StatusLabel tk(conf text:
			      case I of
				 ~1 then StatusEndText
			      elseof
				 0  then StatusInitText
			      else
				 'Current Thread:  #' # I # '  (' # S # ')'
			      end)}
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
	    B = ThreadManager,thrIsBuiltin(id:{Thread.id T} builtin:$)
	 in
	    case B then
	       {OzcarMessage '\'next\' on builtin - substituting by \'step\''}
	       skip
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
	    {Browse {Dbg.taskstack T 25}}
	 end
	 
      else skip end
   end
end

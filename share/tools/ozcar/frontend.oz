%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

class Frontend from Menu Dialog

   feat
      toplevel
      menuBar

      ThreadActionFrame
   
      ThreadTreeFrame
      StackFrame
      EnvFrame

      StatusLabel
   
      ThreadTree
      StackText
      EnvText

   meth init
      %% create the main window
      self.toplevel = {New Tk.toplevel tkInit(title:TitleName)}
      {Tk.batch [wm(iconname   self.toplevel IconName)
		 wm(iconbitmap self.toplevel BitMap)]}

      %% initialize sub classes
      Menu,init
      Dialog,init

      %% we have five different frames:
      %%  ---------------------------------------
      %% | menuBar                               |
      %% | ThreadActionFrame                     |
      %% | ThreadTreeFrame StackFrame EnvFrame   |
      %%  ---------------------------------------
      {ForAll [self.ThreadActionFrame  # ""
	       self.ThreadTreeFrame    # "Thread Tree"
	       self.StackFrame         # "Stack"
	       self.EnvFrame           # "Environment"]
       proc{$ F}
	  F.1 = {New TitleFrame tkInit(parent:self.toplevel
				       title:F.2
				       bd:BorderSize
				       relief:ridge)}
       end}
      {Tk.batch [pack(self.menuBar self.ThreadActionFrame
		      side:top fill:x)
		 pack(self.ThreadTreeFrame self.StackFrame self.EnvFrame
		      side:left expand:yes fill:both)]}
      
      %% the status line: text on the left, buttons on the right      
      self.StatusLabel = {New Tk.label tkInit(parent:self.ThreadActionFrame
					      text:'No current Thread')}
      local
	 Bs = {Map [step cont stack]
	       fun {$ B}
		  {New Tk.button tkInit(parent: self.ThreadActionFrame
					text:   B
					action: self # send(B))}
	       end}
      in
	 {Tk.batch [pack(b(Bs) side:right padx:2)
		    pack(self.StatusLabel side:left fill:x)]}
      end
      
      %% create the thread tree object
      self.ThreadTree = {New Tree init(parent: self.ThreadTreeFrame
				       width:  ThreadTreeWidth
				       height: ThreadTreeHeight)}

      %% the text widgets for stack and environment
      local
	 SBS SBE
      in
	 {ForAll [self.StackText # self.StackFrame # StackTextWidth # SBS
		  self.EnvText   # self.EnvFrame   # EnvTextWidth   # SBE]
	  proc {$ T}
	     T.1 = {New Tk.text tkInit(parent:T.2 width:T.3 cursor:TextCursor
				       font:DefaultFont bd:SmallBorderSize)}
	     T.4 = {New Tk.scrollbar tkInit(parent:T.2)}
	     {Tk.addYScrollbar T.1 T.4}
	  end}
	 {Tk.batch [pack(self.StackText self.EnvText
			 expand:yes fill:both side:left)
		    pack(SBS SBE expand:no fill:y side:right)]}
      end
   end
   
   meth send(Action)
      {self Action(@currentThread)}
   end

   meth switch(T I)
      %TS = {Dbg.taskstack T}
      %% todo: adapt highlighted lines in SourceWindows
   %in
      currentThread <- T
      Frontend,printStatus("Current Thread:  #" # I #
			   "  (" # {Thread.state T} # ")")
      {self.ThreadTree select(T)}
   end

   meth stepThread(file:F line:L thr:T name:N args:A)
      {self highlight(file:F line:L)}  % ignore T for now -- sometime later we
                                       % might have different colors for
                                       % different threads
      {self.StackFrame title('Stack of  #' # {Thread.id T})}
      local
	 W    = self.StackText
         Args = {FormatArgs A}
	 File = {Str.rchr {Atom.toString F} &/}.2
      in
         {ForAll [tk(conf state:normal)
		  tk(insert 'end' '{' # N)
		  tk(conf state:disabled)] W}
	  
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
	      {ForAll [tk(conf state:normal)
		       tk(insert 'end' ' ')
		       tk(insert 'end' A.2 T)
		       tk(conf state:disabled)
		       tk(tag bind T '<1>' Ac)
		       tk(tag conf T font:BoldFont)] W}
	   end}
	   
	  {ForAll [tk(conf state:normal)
		   tk(insert 'end' '}\n') 
		   tk(conf state:disabled)
		   tk(yview 'end')] W}
      end
   end

   meth newThread(T I)
      {ForAll [add(T I) display] self.ThreadTree}
      {self switch(T I)}
   end
   
   meth removeThread(T P I)
      {self.ThreadTree remove(T)}
      case @currentThread == T then
	 {self switch(P I)}
      else
	 {self.ThreadTree display}
      end
   end
   
   meth printStatus(S)
      {self.StatusLabel tk(conf text:S)}
   end
   
   meth stackThread(T)
      S = {Filter {Dbg.taskstack T} fun {$ E} E \= toplevel end}
   in
      {Browse S}
   end

end

%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

class Gui from Menu Dialog

   feat
      toplevel
      menuBar

      ButtonFrame
      StatusLabel
   
      ThreadTree
      StackText
      EnvText

   meth init
      %% create the main window
      self.toplevel = {New Tk.toplevel tkInit(title:TitleName)}
      {Tk.batch [wm(iconname   self.toplevel IconName)
		 wm(iconbitmap self.toplevel BitMap)]}

      Menu,init
      Dialog,init

      self.ButtonFrame = {New Tk.frame tkInit(parent:self.toplevel
					      bd:BorderSize
					      relief:ridge)}
      {Tk.batch [grid(self.menuBar     row:0 column:0 sticky:we columnspan:3)
		 grid(self.ButtonFrame row:1 column:0 sticky:we columnspan:3)]}
      
      %% buttons and status line
      self.StatusLabel = {New Tk.label tkInit(parent:self.ButtonFrame
					      text:'No current Thread')}
      local
	 Bs = {Map [step cont stack]
	       fun {$ B}
		  {New Tk.button tkInit(parent: self.ButtonFrame
					text:   B
					action: self # send(B))}
	       end}
      in
	 {Tk.batch [pack(b(Bs) side:left padx:2)
		    pack(self.StatusLabel side:right fill:x)]}
      end

      %% create the thread tree object...
      self.ThreadTree = {New Tree tkInit(parent: self.toplevel
					 title:  'Thread Tree'
					 width:  ThreadTreeWidth
					 bg:     DefaultBackground)}
      %% ...and the text widgets for stack and environment
      {ForAll [self.StackText # 'Stack'
	       self.EnvText   # 'Environment']
       proc {$ T}
	  T.1 = {New ScrolledTitleText tkInit(parent: self.toplevel
					      title:  T.2
					      width:  TextWidth
					      bd:     SmallBorderSize
					      cursor: TextCursor
					      font:   DefaultFont
					      bg:     DefaultBackground)}
       end}
      {Tk.batch [grid(self.ThreadTree row:2 column:0 sticky:nswe)
		 grid(self.StackText  row:2 column:1 sticky:nswe)
		 grid(self.EnvText    row:2 column:2 sticky:nswe)
		 grid(rowconfigure    self.toplevel 2 weight:1)
		 grid(columnconfigure self.toplevel 0 weight:3)
		 grid(columnconfigure self.toplevel 1 weight:1)
		 grid(columnconfigure self.toplevel 2 weight:1)]}
   end
   
   meth send(Action)
      {self Action(@currentThread)}
   end
   
   meth switch(T I)
      %TS = {Dbg.taskstack T}
      %% todo: adapt highlighted lines in SourceWindows
   %in
      currentThread <- T
      Gui,printStatus("Current Thread:  #" # I #
			   "  (" # {Thread.state T} # ")")
      {self.ThreadTree select(T)}
   end

   meth stepThread(file:F line:L thr:T name:N args:A)
      {self highlight(file:F line:L)}  % ignore T for now -- sometime later we
                                       % might have different colors for
                                       % different threads
      {self.StackText title('Stack of  #' # {Thread.id T})}
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

%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

class Frontend from Dialog

   feat
      toplevel

      MenuBar
   
      ThreadActionFrame
      ThreadTreeFrame
      CallTraceFrame
      BrowserFrame

      ThreadTree
      Browzcar
      StatusLabel
      CallTraceText

   attr
      CurrentThread
   
   meth init
      self.toplevel = {New Tk.toplevel tkInit(title:TitleName)}
      {Tk.batch [wm(geometry   self.toplevel '708x492')
		 wm(iconname   self.toplevel IconName)
		 wm(iconbitmap self.toplevel BitMap)]}

      %Menu,init
      Dialog,init

      self.MenuBar =
      {TkTools.menubar self.toplevel self.toplevel
       [menubutton(text: "Ozcar"
		   menu:
		      [command(label:   "About"
			       action:  self # about
			       feature: about)]
		   feature: ozcar)
	menubutton(text: "File"
		   menu:
		      [command(label:   "Quit"
			       action:  self # exit
			       key:     ctrl(c)
			       feature: quit)]
		   feature: file)
	menubutton(text: "Thread"
		   menu:
		      [separator]
		   feature: 'thread')
	menubutton(text: "Options"
		   menu:
		      [separator]
		   feature: options)
	menubutton(text: "Help"
		   menu:
		      [separator]
		   feature: help)]
       nil}
      
      {ForAll [self.ThreadActionFrame   # ""
	       self.ThreadTreeFrame     # "Thread Tree"
	       self.CallTraceFrame      # "Call Trace"
	       self.BrowserFrame        # "Environment Browser"]
       proc{$ F}
	  F.1 = {New TitleFrame tkInit(parent:self.toplevel
				       title:F.2
				       bd:BorderSize
				       relief:ridge)}
       end}

      {Tk.batch [pack(self.MenuBar self.ThreadActionFrame side:top fill:x)
		 pack(self.ThreadTreeFrame side:left fill:y)
		 pack(self.CallTraceFrame expand:yes fill:both)
		 pack(self.BrowserFrame expand:yes fill:both)]}

      self.ThreadTree = {New Tree init(parent:self.ThreadTreeFrame
				       width:ThreadTreeWidth
				       height:ThreadTreeHeight)}

      local
	 Bs = {Map ['step' 'cont' 'stack']
	       fun {$ B}
		  {New Tk.button tkInit(parent: self.ThreadActionFrame
					text:   B
					action: self # send(B))}
	       end}
      in
	 {Tk.send pack(b(Bs) side:right padx:2)}
      end

      self.StatusLabel = {New Tk.label tkInit(parent:self.ThreadActionFrame
					      text:"No current Thread")}

      self.CallTraceText = {New Tk.text tkInit(parent: self.CallTraceFrame
					       font:   DefaultFont
					       width:  CallTraceTextWidth
					       height: CallTraceTextHeight
					       cursor: CallTraceTextCursor
					       bd:     SmallBorderSize)}
      local
	 SB = {New Tk.scrollbar tkInit(parent:self.CallTraceText)}
      in
	 {Tk.addYScrollbar self.CallTraceText SB}
	 {Tk.batch [pack(self.CallTraceText expand:yes fill:both side:left)
		    %pack(SB expand:no fill:y side:right)
		    pack(self.StatusLabel side:left fill:x)]}
      end

      self.Browzcar = {New BrowserClass
		       init(origWindow: self.BrowserFrame)}
      {ForAll [createWindow] self.Browzcar}
   end

   meth send(Action)
      {self Action(@CurrentThread)}
   end

   meth switch(T I)
      %TS = {Dbg.taskstack T}
      %% todo: adapt highlighted lines in SourceWindows
   %in
      CurrentThread <- T
      Frontend,printStatus("Current Thread:  #" # I #
			   "  (" # {Thread.state T} # ")")
      {self.ThreadTree select(T)}
   end

   meth stepThread(file:F line:L thr:T name:N args:A)
      {self highlight(file:F line:L)}  % ignore T for now -- sometime later we
                                       % might have different colors for
                                       % different threads
      local
	 W    = self.CallTraceText
         Args = {FormatArgs A}
	 File = {Str.rchr {Atom.toString F} &/}.2
      in
         {ForAll [tk(conf state:normal)
		  tk(insert 'end'
          {Thread.id T} # ' ' # File # ':' # L #' {' # N)
		  tk(conf state:disabled)] W}
	  
	  {ForAll Args
	   proc {$ A}
	      T = {TagCounter get($)}
	      Ac = {New Tk.action
		    tkInit(parent:W
			   action:proc{$}
				     S = A.3
				  in
				     {self.Browzcar browse(S)}
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
      case @CurrentThread == T then
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
      {self.Browzcar browse(S)}
   end
   
end

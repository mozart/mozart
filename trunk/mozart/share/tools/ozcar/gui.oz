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
      %% create the main window, but delay showing it
      %% until everything has been packed inside
      self.toplevel = {New Tk.toplevel tkInit(title:TitleName withdraw:true)}
      {Tk.batch [wm(iconname   self.toplevel IconName)
		 wm(iconbitmap self.toplevel BitMap)
		 wm(geometry   self.toplevel ToplevelGeometry)]}

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
					action: self # action(B))}
	       end}
      in
	 {Tk.batch [pack(b(Bs) side:right padx:2)
		    pack(self.StatusLabel side:left fill:x)]}
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
		 grid(columnconfigure self.toplevel 0 weight:2)
		 grid(columnconfigure self.toplevel 1 weight:1)
		 grid(columnconfigure self.toplevel 2 weight:1)
		 wm(deiconify         self.toplevel)
		]}
   end

   meth stackText($)
      self.StackText
   end
   
   meth selectNode(T)
      {self.ThreadTree select(T)}
   end
   
   meth addNode(T I)
      {self.ThreadTree add(T I)}
   end
   
   meth removeNode(T)
      {self.ThreadTree remove(T)}
   end

   meth displayTree
      {self.ThreadTree display}
   end
   
   meth status(S)
      {self.StatusLabel tk(conf text:S)}
   end

   meth stackTitle(S)
      {self.StackText title(S)}
   end

   meth action(A)
      case {Label A}
      of step then
	 {Thread.resume @currentThread}
      elseof cont then
	 {Dbg.stepmode @currentThread false}
	 {Thread.resume @currentThread}
      elseof stack then
	 S = {Filter {Dbg.taskstack @currentThread 1}
	      fun {$ E} E \= toplevel end}
      in
	 {Browse S}
      end
   end
   
end

%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

class Gui from Menu Dialog

   feat
      toplevel
      menuBar

      ButtonFrame
      StatusLabel

      ApplFrame
      ApplPrefix
      ApplText
   
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

      {ForAll [self.ButtonFrame self.ApplFrame]
       proc{$ F}
	  F = {New Tk.frame tkInit(parent:self.toplevel
				   bd:BorderSize
				   relief:ridge)}
       end}
      
      {Tk.batch [grid(self.menuBar     row:0 column:0 sticky:we columnspan:3)
		 grid(self.ButtonFrame row:1 column:0 sticky:we columnspan:3)
		 grid(self.ApplFrame   row:2 column:0 sticky:we columnspan:3)
		]}
      
      %% buttons and status line
      self.StatusLabel = {New Tk.label tkInit(parent:self.ButtonFrame
					      text:StatusInitText)}
      local
	 Bs = {Map [step next finish cont forget stack]
	       fun {$ B}
		  {New Tk.button tkInit(parent: self.ButtonFrame
					text:   B
					action: self # action(B))}
	       end}
      in
	 {Tk.batch [pack(b(Bs) side:right padx:2)
		    pack(self.StatusLabel side:left fill:x)]}
      end

      %% application frame
      self.ApplPrefix = {New Tk.label tkInit(parent: self.ApplFrame
					     text:   ApplPrefixText)}
      self.ApplText   = {New Tk.text  tkInit(parent: self.ApplFrame
					     state:  disabled
					     height: 1
					     bd:     SmallBorderSize
					     cursor: TextCursor
					     font:   DefaultFont
					     bg:     DefaultBackground)}
      {Tk.batch [pack(self.ApplPrefix side:left)
		 pack(self.ApplText side:left fill:x expand:yes)]}
      
      %% create the thread tree object...
      self.ThreadTree = {New Tree tkInit(parent: self.toplevel
					 title:  TreeTitle
					 width:  ThreadTreeWidth
					 bg:     DefaultBackground)}
      %% ...and the text widgets for stack and environment
      {ForAll [self.StackText # StackTitle
	       self.EnvText   # EnvTitle  ]
       proc {$ T}
	  T.1 = {New ScrolledTitleText tkInit(parent: self.toplevel
					      title:  T.2
					      state:  disabled
					      width:  TextWidth
					      bd:     SmallBorderSize
					      cursor: TextCursor
					      font:   DefaultFont
					      bg:     DefaultBackground)}
       end}
      {Tk.batch [grid(self.ThreadTree row:3 column:0 sticky:nswe)
		 grid(self.StackText  row:3 column:1 sticky:nswe)
		 grid(self.EnvText    row:3 column:2 sticky:nswe)
		 grid(rowconfigure    self.toplevel 3 weight:1)
		 grid(columnconfigure self.toplevel 0 weight:2)
		 grid(columnconfigure self.toplevel 1 weight:1)
		 grid(columnconfigure self.toplevel 2 weight:1)
		 wm(deiconify         self.toplevel)
		]}
   end

   meth printEnv(frame:I globals:G<=nil locals:Y<=unknown)
      E = self.EnvText
   in
      case I == 0 then
	 {self.EnvText title(EnvTitle)}
      else
	 {self.EnvText title(AltEnvTitle # I)}
      end
      
      %% G
      {ForAll [tk(conf state:normal)
	       tk(delete '0.0' 'end')
	       tk(insert 'end' 'G'#NL)] E}
      case G == nil orelse G == unknown then
	 {E tk(insert 'end' ' nil or unknown'#NL#NL)}
      else
	 {ForAll G proc{$ V}
		      T = {TagCounter get($)}
		      Ac = {New Tk.action
			    tkInit(parent: E
				   action: proc{$}{Browse V.2}end)}
		   in
		      {ForAll [tk(insert 'end' ' ' # {PrintF V.1 13})
			       tk(insert 'end' {ArgType V.2} # NL T)
			       tk(tag bind T '<1>' Ac)
			       tk(tag conf T font:BoldFont)] E}
		   end}
	 {E tk(insert 'end' NL)}
      end
      
      %% Y
      {E tk(insert 'end' 'Y'#NL)}
      case Y == nil orelse Y == unknown then
	 {E tk(insert 'end' ' nil or unknown'#NL#NL)}
      else
	 {ForAll Y proc{$ V}
		      T = {TagCounter get($)}
		      Ac = {New Tk.action
			    tkInit(parent: E
				   action: proc{$}{Browse V.2}end)}
		   in
		      {ForAll [tk(insert 'end' ' ' # {PrintF V.1 13})
			       tk(insert 'end' {ArgType V.2} # NL T)
			       tk(tag bind T '<1>' Ac)
			       tk(tag conf T font:BoldFont)] E}
		   end}
	 {E tk(insert 'end' NL)}
      end
      
      {E tk(conf state:disabled)}
   end

   meth frameClick(nr:I frame:F)
      Gui,printEnv(frame:I globals:F.'G')
      SourceManager,scrollbar(file:F.file line:F.line
			      color:ScrollbarStackColor what:stack)
   end
   
   meth printStack(id:ThrID stack:Stack)
      W = self.StackText
      S = {List.filter Stack fun{$ X}
				{Label X} == 'proc'
				% andthen X.name \= 'Toplevel abstraction'
			     end}
   in
      case S == nil then
	 {ForAll [tk(conf state:normal)
		  tk(delete '0.0' 'end')
		  tk(insert 'end' "  nil")
		  tk(conf state:disabled)] W}
	 {self.StackText title(StackTitle)}
	 Gui,printEnv(frame:0)
      else
	 {self.StackText title(AltStackTitle # ThrID)}
	 Gui,printEnv(frame:1 globals:S.1.'G')
	 %SourceManager,scrollbar(file:S.1.file line:S.1.line
		%		 color:ScrollbarStackColor what:stack)

	 {ForAll [tk(conf state:normal)
		  tk(delete '0.0' 'end')] W}
	 {List.forAllInd S
	  proc{$ I F}
	     T = {TagCounter get($)}
	     Ac = {New Tk.action
		   tkInit(parent: W
			  action: Ozcar # frameClick(nr:I frame:F))}
	  in
     	     {ForAll [tk(insert 'end' I # ' ' # case F.name == ''
						then '$' else F.name
						end # NL T)
		      tk(tag bind T '<1>' Ac)
		      tk(tag conf T font:BoldFont)] W}
	  end}
	 {W tk(conf state:disabled)}
      end
   end
   
   meth printAppl(name:N args:A builtin:B)
      case N == undef orelse A == undef then
	 {ForAll [tk(conf state:normal)
		  tk(delete '0.0' 'end')
		  tk(insert 'end' ApplLabelInit)
		  tk(conf state:disabled)] self.ApplText}
      else
	 W    = self.ApplText
	 Args = {FormatArgs A}
	 Type = case B then ' B' else ' P' end
      in
	 {ForAll [tk(conf state:normal)
		  tk(delete '0.0' 'end')
		  tk(insert 'end' Type # ' {' # case N == ''
						then '$' else N
						end)] W}
	  
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
	  
	  {ForAll [tk(insert 'end' '}') 
		   tk(conf state:disabled)] W}
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
	    
	 of step then
	    {Thread.resume T}
	    
	 elseof next then
	    B = ThreadManager,thrIsBuiltin(id:{Thread.id T} builtin:$)
	 in
	    case B then
	       {OzcarMessage '\'next\' on builtin - substituting by \'step\''}
	       skip
	    else
	       {Dbg.stepmode T false}
	    end
	    {Thread.resume T}
	    
	 elseof finish then
	    {Browse 'not yet implemented'}
	    skip
	    
	 elseof cont then
	    /*
	    {Dbg.stepmode T false}
	    {Thread.resume T}
	    */
	    {Browse 'not yet implemented'}
	    skip
	    
	 elseof forget then
	    {Dbg.trace T false}      %% thread is not traced anymore
	    {Dbg.stepmode T false}   %% no step mode, run as you like!
	    {Thread.resume T}        %% run, run to freedom!! :-)
	    ThreadManager,remove(T I kill)
	    
	 elseof stack then  %% will go away, someday...
	    {Browse {Dbg.taskstack T 25}}
	 end
	 
      else skip end
   end
end

%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

class Frontend from Dialog

   feat
      toplevel

      MenuBar
   
      ThreadActionFrame
      ThreadSelectorFrame
      ThreadInfoFrame

      StatusLabel
      ThreadInfoText

      V
   
   attr
      CurrentThread
   
   meth init
      self.toplevel = {New Tk.toplevel tkInit(title:TitleName)}
      {Tk.batch [wm(iconname   self.toplevel IconName)
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
			       action:  self # shutdown
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
	       self.ThreadSelectorFrame # "Thread Selector"
	       self.ThreadInfoFrame     # "Thread Information"]
       proc{$ F}
	  F.1 = {New TitleFrame tkInit(parent:self.toplevel
				       title:F.2
				       bd:BorderSize
				       relief:ridge)}
       end}
      
      {Tk.batch [pack(self.MenuBar self.ThreadActionFrame side:top fill:x)
		 pack(self.ThreadSelectorFrame side:left fill:y)
		 pack(self.ThreadInfoFrame expand:yes fill:both)]}

      self.V = {New Tk.variable tkInit(0)}

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

      self.ThreadInfoText = {New Tk.text tkInit(parent: self.ThreadInfoFrame
						font:   DefaultFont
						bd:     SmallBorderSize)}
      self.StatusLabel = {New Tk.label tkInit(parent:self.ThreadActionFrame
					      text:"No current Thread")}
      {Tk.batch [pack(self.ThreadInfoText expand:yes fill:both)
		 pack(self.StatusLabel side:left fill:x)]}
   end

   meth send(Action)
      {{Thread.getName @CurrentThread} Action}
   end

   meth newThread(T)
      Id = {Thread.id T}
      R  = {New Tk.radiobutton tkInit(parent:  self.ThreadSelectorFrame
				      variable:self.V
				      value:   Id   text: "ID " # Id
				      action:  self # switch(Id))}
   in
      {Tk.send pack(R side:top pady:3 fill:x anchor:w)}
      {self.V tkSet(Id)}
      self,switch(Id)
   end
   
   meth switch(Id)
      T = {Dbg.threadByID Id}
   in
      CurrentThread <- T
      {self.StatusLabel tk(conf text:"Current Thread:  #" # Id #
			   " (state: " # {Thread.state T} # ")")}
   end
   
   meth stack(T)
      S = {Dbg.taskstack T}
      S1 S2 S3
   in
      S1 = {List.filter S fun{$ T}  %% only show "proc" and "builtin" for now
			     L = {Label T}
			  in
			     L == builtin orelse L == 'proc' /* end */
			  end}
      S2 = {Map S1 fun {$ T}
		      L = {Label T}
		   in
		      case L
		      of     builtin then T
		      elseof 'proc' then   /* end (huhu Emacs!) */ 
			 {Record.filterInd T fun {$ F Gaga}
						F == file orelse
						F == line orelse
						F == name orelse
						F == 'G'
					     end}
		      end
		   end}
      {Browse S2}

      {self.ThreadInfoText tk(delete '1.0' 'end')}
      {ForAll S2
       proc{$ E}
	  case {Label E} == builtin
	  then
	     {self.ThreadInfoText tk(insert 'end' 'builtin(' # E.1 # ')\n')}
	  else
	     {self.ThreadInfoText tk(insert 'end' 'proc('# E.name # ')\n')}
	  end
       end}
   end
end

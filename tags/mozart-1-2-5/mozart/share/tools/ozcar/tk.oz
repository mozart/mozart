%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.

local
   Timeout = 15 % ms
in
   class Smoother
      prop
	 locking
      attr
	 QueueSync : _
	 MsgList   : nil
	 MsgListTl : nil
      meth tk(...)=M
	 Smoother,Enqueue(o(self d(M)))
      end
      meth Enqueue(Ticklet)
	 lock
	    case Ticklet
	    of nil  then skip
	    [] T|Tr then
	       Smoother,Enqueue(T)
	       Smoother,Enqueue(Tr)
	    else NewTl in
	       if {IsDet @MsgListTl} then
		  MsgList <- Ticklet|NewTl
	       else
		  @MsgListTl = Ticklet|NewTl
	       end
	       MsgListTl <- NewTl
	       Smoother,ClearQueue
	    end
	 end
      end
      meth ClearQueue
	 New in
	 QueueSync <- New = unit
	 thread
	    {WaitOr New {Alarm Timeout}}
	    if {IsDet New} then skip else
	       Smoother,DoClearQueue
	    end
	 end
      end
      meth DoClearQueue
	 lock
	    @MsgListTl = nil
	    try
	       {Tk.batch @MsgList}
	    catch _ then  %% maybe the window has been closed already?
	       skip
	    end
	    MsgList <- nil
	 end
      end
   end
end

class TitleFrame from Tk.frame
   feat Label
   meth tkInit(title:T<=unit ...)=M
      Tk.frame,{Record.subtract M title}
      if T \= unit then
	 self.Label = {New Tk.label
		       tkInit(parent: self
			      text:   T
			      font:   TitleFont
			      bd:     0
			      relief: raised)}
	 {Tk.send grid(self.Label row:0 column:0 sticky:we)}
      end
   end
   meth title(S)
      if {IsDet self.Label} then
	 {self.Label tk(conf text:S)}
      end
   end
end

class ScrolledTitleCanvas from Tk.canvas Smoother
   feat
      frame
   meth tkInit(parent:P title:T ...)=M
      self.frame = {New TitleFrame tkInit(parent:P title:T)}
      Tk.canvas,{Record.subtract {Record.adjoinAt M parent self.frame} title}
      local
	 SY = {New Tk.scrollbar
	       tkInit(parent: self.frame
		      width:  ScrollbarWidth)}
      in
	 {Tk.addYScrollbar self SY}
	 {Tk.batch [grid(self row:1 column:0 sticky:nswe padx:1 pady:1)
		    grid(SY   row:1 column:1 sticky:ns   padx:1 pady:1)
		    grid(rowconfigure    self.frame 1 weight:1)
		    grid(columnconfigure self.frame 0 weight:1)]}
      end
   end
   meth title(S)=M
      {self.frame M}
   end
   meth tk(...)=M
      Smoother,M
   end
end

class ScrolledTitleText from Tk.text Smoother
   feat
      frame TagBase
   attr
      NextTag
   meth tkInit(parent:P title:T ...)=M
      self.TagBase = ~1
      NextTag <- self.TagBase
      self.frame = {New TitleFrame tkInit(parent:P title:T)}
      Tk.text,{Record.subtract {Record.adjoinAt M parent self.frame} title}
      local
	 SY = {New Tk.scrollbar
	       tkInit(parent: self.frame
		      width:  ScrollbarWidth)}
      in
	 {Tk.addYScrollbar self SY}
	 {Tk.batch [grid(self row:1 column:0 sticky:nswe padx:1 pady:1)
		    grid(SY   row:1 column:1 sticky:ns   padx:1 pady:1)
		    grid(rowconfigure    self.frame 1 weight:1)
		    grid(columnconfigure self.frame 0 weight:1)]}
      end
      Tk.text,tk(conf pady:2)
   end
   meth title(S)=M
      {self.frame M}
   end
   meth newTag($)
      NextTag <- @NextTag - 1
   end
   meth resetTags
      proc {DoIt N}
	 if N < self.TagBase then N1 = N + 1 in
	    {self tk(tag delete N1)}
	    {DoIt N1}
	 end
      end
   in
      {DoIt NextTag<-self.TagBase}
   end
   meth tk(...)=M
      Smoother,M
   end
end

class StatusDisplay from Tk.text Smoother
   meth replace(Message Color<=unit)
      StatusDisplay,DoIt(Message true Color)
   end
   meth append(Message Color<=unit)
      StatusDisplay,DoIt(Message false Color)
   end
   meth DoIt(Message Clear Color)
      {self tk(conf state:normal)}
      if Clear then
	 {self tk(delete p(0 0) 'end')}
      end
      {self tk(insert 'end' Message)}
      if Color \= unit then
	 {self tk(conf fg:Color)}
      end
      {self tk(conf state:disabled)}
   end
   meth tk(...)=M
      Smoother,M
   end
end

%
% Authors:
%   Donatien Grolaux (2000)
%
% Copyright:
%   (c) 2000 Université catholique de Louvain
%
% Last change:
%   $Date$ by $Author$
%   $Revision$
%
% This file is part of Mozart, an implementation
% of Oz 3:
%   http://www.mozart-oz.org
%
% See the file "LICENSE" or
%   http://www.mozart-oz.org/LICENSE.html
% for information on usage and redistribution
% of this file, and for a DISCLAIMER OF ALL
% WARRANTIES.
%
%  The development of QTk is supported by the PIRATES project at
%  the Université catholique de Louvain.


functor

import
   Tk
   QTkImage
   QTkDevel(splitParams:        SplitParams
	    init:               Init
	    assert:             Assert
	    qTkDesc:            QTkDesc
	    convertToType:      ConvertToType
	    subtracts:          Subtracts
	    globalInitType:     GlobalInitType
	    globalUnsetType:    GlobalUnsetType
	    globalUngetType:    GlobalUngetType)

export
   Register
   
require QTkNumberentry_bitmap

prepare BL=QTkNumberentry_bitmap.buildLibrary

define
   
   WidgetType=numberentry
   Feature=false
   Lib={QTkImage.buildImageLibrary BL}
%   IncStep     = 10
%   IncTime     = 100
%   IncWait     = 500
%   Border      = 1
   
   class QTkNumberentry

      
      feat
	 widgetType:WidgetType
	 typeInfo:r(all:{Record.adjoin GlobalInitType
			 r(1:natural
			   init:natural
			   return:free
			   background:color bg:color
			   borderwidth:pixel
			   cursor:cursor
			   exportselection:boolean
			   font:font
			   foreground:color fg:color
			   highlightbackground:color
			   highlightcolor:color
			   highlightthickness:pixel
			   insertbackground:color
			   insertborderwidth:pixel
			   insertofftime:natural
			   insertontime:natural
			   insertwidth:pixel
			   justify:[left center right]
			   relief:relief
			   selectbackground:color
			   selectborderwidth:pixel
			   selectforeground:color
			   takefocus:boolean
			   show:vs
			   state:[normal disabled]
			   width:natural
			   action:action
			   min:natural max:natural
			  )}
		    uninit:r(1:unit)
		    unset:{Record.adjoin GlobalUnsetType
			   r(init:unit
			     min:unit
			     max:unit)}
		    unget:{Record.adjoin GlobalUngetType
			   r(init:unit
			     min:unit
			     max:unit
			     font:unit)}
		   )
	 action
	 Entry Inc Dec EReturn
	 Return

      attr Min Max LastVal ID:nil
   
      meth numberentry(...)=M
	 lock
	    {Assert self.widgetType self.typeInfo {Record.subtract {Record.adjoin M Init} QTkDesc}}
	    self.action={CondSelect M action proc{$} skip end}
 	    Min<-{CondSelect M min 1}
 	    Max<-{CondSelect M max 100}
 	    LastVal<-""
 	    self.Return={CondSelect M return _}
	    M.QTkDesc=lr({Record.adjoin
			  {Subtracts M [min max return action QTkDesc]}
			  entry(handle:self.Entry
				glue:nswe
				return:self.EReturn)}
			 td(glue:ns
			    button(image:{Lib get(name:'mini-inc.xbm' image:$)} handle:self.Inc glue:ns)
			    button(image:{Lib get(name:'mini-dec.xbm' image:$)} handle:self.Dec glue:ns)))
	 end
	 thread
	    {self.Entry bind(event:  '<KeyPress-Up>'
			     action: self # Inc(1))}
	    {self.Entry bind(event:  '<KeyPress-Down>'
			     action: self # Inc(~1))}
	    {self.Entry bind(event:  '<Shift-KeyPress-Up>'
			     action: self # Inc(10))}
	    {self.Entry bind(event:  '<Shift-KeyPress-Down>'
			     action: self # Inc(~10))}
	    {self.Entry bind(event:  '<KeyRelease-Up>'
			     action: self # IncStop)}
	    {self.Entry bind(event:  '<KeyRelease-Down>'
			     action: self # IncStop)}
	    {self.Entry bind(event:  '<FocusOut>'
			     action: self # CheckValue)}
	    {self.Inc bind(event:  '<ButtonPress-1>'
			   action: self # Inc(1))}
	    {self.Inc bind(event:  '<ButtonRelease-1>'
			   action: self # IncStop)}
	    {self.Dec bind(event:  '<ButtonPress-1>'
			   action: self # Inc(~1))}
	    {self.Dec bind(event:  '<ButtonRelease-1>'
			   action: self # IncStop)}
	    {self.Entry set({CondSelect M init @Min})}
	    {self CheckValue(exec:false)}
	 end
	 thread
	    {Wait self.EReturn}
	    self.Return={ConvertToType self.EReturn natural}
	 end
      end
      
      meth CheckValue(exec:E<=true)
	 V={self.Entry get($)}
	 N
      in
	 if V=="" then
	    LastVal<-""
	 else
	    if {List.all V fun{$ C} C>=48 andthen C=<57 end} then
	       try
		  N={ConvertToType V natural}
	       catch _ then skip end
	    end
	    if {IsDet N} andthen N>=@Min andthen N=<@Max then
	       LastVal<-V
	    else
	       {Tk.send bell}
	       {self.Entry set(@LastVal)}
	    end
	 end
	 if E then
	    {self Exec}
	 end
      end

      meth Exec
	    if {Procedure.is self.action} then
	       {self.action}
	    elsecase self.action of E#M then
	       if {Port.is E} then
		  {Send E M}
	       else
		  {E M}
	       end
	    end
      end

      meth Add(I)
	 V={ConvertToType {self.Entry get($)} natural}
	 N0=if V==false then
	       if I>0 then @Min else @Max end
	    else
	       V+I
	    end
	 N1=if N0<@Min then @Min else N0 end
	 N=if N1>@Max then @Max else N1 end
      in
	 {self.Entry set(N)}
	 LastVal<-N
	 {self Exec}
      end
   
      meth Inc(I)
	 proc{Loop IncWait}
	    {Delay IncWait}
	    {self Add(I)}
	    {Loop 100}
	 end
      in
	 {self IncStop}
	 {self Add(I)}
	 ID<-_
	 thread
	    @ID={Thread.this}
	    {Loop 500}
	 end
	 {Wait @ID}
      end

      meth IncStop
	 try
	    {Thread.terminate @ID}
	 catch _ then skip end
      end

      meth set(...)=M
	 lock
	    {Assert self.widgetType self.typeInfo M}
	    {self.Entry M}
	    {self CheckValue(exec:false)}
	 end
      end

      meth get(...)=M
	 lock
	    {Assert self.widgetType self.typeInfo M}
	    A B
	 in
	    {SplitParams M [1] A B}
	    {self.Entry A}
	    if {HasFeature B 1} then
	       {self CheckValue(exec:false)}
	       R={ConvertToType {self.Entry get($)} natural}
	    in
	       B.1=if R==false then @Min else R end
	    end
	 end
      end

      meth otherwise(M)
	 lock
	    {self.Entry M}
	 end
      end
      
   end

   Register=[r(widgetType:WidgetType
	       feature:Feature
	       widget:QTkNumberentry)]

end

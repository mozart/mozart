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
   QTkDevel(tkInit:             TkInit
	    init:               Init
	    qTkClass:           QTkClass
	    execTk:             ExecTk
	    returnTk:           ReturnTk
	    splitParams:        SplitParams
	    globalInitType:     GlobalInitType
	    globalUnsetType:    GlobalUnsetType
	    globalUngetType:    GlobalUngetType)

export
   Register
   
define

   fun{Scrollbar WidgetType}
      class $
	 
	 feat
	    widgetType:WidgetType
	    action
	    Orient:if WidgetType==tdscrollbar then vert else horiz end
	    typeInfo:r(all:{Record.adjoin GlobalInitType
			    r(1:float
			      2:float
			      activebackground:color
			      background:color bg:color
			      borderwidth:pixel
			      cursor:cursor
			      highlightbackground:color
			      highlightcolor:color
			      highlightthickness:pixel
			      jump:boolean
			      relief:relief
			      repeatdelay:natural
			      repeatinterval:natural
			      takefocus:boolean
			      troughcolor:color
			      activerelief:relief
			      action:action
			      elementborderwidth:pixel
			      width:pixel)}
		       uninit:r(1 2)
		       unset:GlobalUnsetType
		       unget:{Record.adjoin GlobalUngetType
			      r(2:unit)}
		      )
   
	 from Tk.scrollbar QTkClass
      
	 meth !Init(...)=M
	    lock
	       QTkClass,M
	       Tk.scrollbar,{Record.adjoin {TkInit M}
			     tkInit(action:self.toplevel.port#r(self Execute)
				    orient:self.Orient
				   )}
	    end
	 end
      
	 meth Execute(...)
	    lock
	       {self.action execute}
	    end
	 end

	 meth set(...)=M
	    lock
	       A B
	    in
	       {SplitParams M [1 2] A B}
	       QTkClass,A
	       if {HasFeature B 1} andthen {HasFeature B 2} then
		  {ExecTk self set(B.1 B.2)}
	       end
	    end
	 end
      
	 meth get(...)=M
	    lock
	       A B
	    in
	       {SplitParams M [1] A B}
	       QTkClass,A
	       {Record.forAllInd B
		proc{$ I V}
		   case I
		   of 1 then
		      {ReturnTk self get(V) listFloat}
		   end
		end}
	    end
	 end

	 meth activate(...)=M
	    lock
	       {ExecTk self M}
	    end
	 end

	 meth delta(...)=M
	    lock
	       {ReturnTk self M float}
	    end
	 end

	 meth fraction(...)=M
	    lock
	       {ReturnTk self M float}
	    end
	 end

	 meth identify(...)=M
	    lock
	       {ReturnTk self M atom}
	    end
	 end
     
      end
   end

   Register=[r(widgetType:tdscrollbar
	       feature:false
	       widget:{Scrollbar tdscrollbar})
	     r(widgetType:lrscrollbar
	       feature:false
	       widget:{Scrollbar lrscrollbar})]

end

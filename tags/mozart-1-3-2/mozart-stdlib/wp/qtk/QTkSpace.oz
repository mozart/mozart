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
   QTkDevel(splitParams:        SplitParams
	    tkInit:             TkInit
	    init:               Init
	    assert:             Assert
	    execTk:             ExecTk
	    returnTk:           ReturnTk
	    qTkClass:           QTkClass
	    globalInitType:     GlobalInitType
	    globalUnsetType:    GlobalUnsetType
	    globalUngetType:    GlobalUngetType)

export
   Register
   
define
   
   fun{QTkSpace WidgetType}
      class $
	 
	 feat
	    Return
	    widgetType:WidgetType
	    typeInfo:r(all:{Record.adjoin GlobalInitType
			    r(background:color
			      bg:color
			      width:pixel)}
		       uninit:r
		       unset:GlobalUnsetType
		       unget:GlobalUngetType
		      )
	    Horiz
	    Line
   
	 from Tk.canvas QTkClass
      
	 meth !Init(...)=M
	    lock
	       A
	       W={CondSelect M width 5}
	    in
	       QTkClass,M
	       {SplitParams M [width] A _}
	       self.Horiz#self.Line=case WidgetType
				    of lrspace then true#false
				    [] tdspace then false#false
				    [] lrline then true#true
				    else false#true
				    end
	       Tk.canvas,{Record.adjoin {TkInit A}
			  tkInit(borderwidth:0
				 selectborderwidth:0
				 highlightthickness:0
				 width:if self.Horiz then 1 else W end
				 height:if self.Horiz then W else 1 end)}
	       {ExecTk unit update}
	       {self DrawLine}
	    end
	 end

	 meth DrawLine
	    lock
	       if self.Line then
		  S
	       in
%	       {Tk.returnInt winfo(if self.Horiz then
%				      height
%				   else
%				      width
%				   end self) S}
		  {self tkReturnInt(cget(if self.Horiz then "-height" else "-width" end) S)}
		  Tk.canvas,tk(delete all)
		  Tk.canvas,if self.Horiz then
			       tk(crea line
				  0       (S div 2)
				  1000000 (S div 2)
				  fill:white)
			    else
			       tk(crea line
				  (S div 2) 0
				  (S div 2) 1000000
				  fill:white)
			    end
		  Tk.canvas,if self.Horiz then
			       tk(crea line
				  0       (S div 2)+1
				  1000000 (S div 2)+1
				  fill:black)
			    else
			       tk(crea line
				  (S div 2)+1 0
				  (S div 2)+1 1000000
				  fill:black)
			    end
	       end
	    end
	 end

	 meth set(...)=M
	    lock
	       A B
	    in
	       {SplitParams M [width] A B}
	       QTkClass,A
	       {Assert self.widgetType self.typeInfo B}
	       {Record.forAllInd B
		proc{$ I V}
		   case I
		   of width then
		      if self.Horiz then
			 {ExecTk self configure(height:V)}
		      else
			 {ExecTk self configure(width:V)}
		      end
		      {self DrawLine}
		   else skip end
		end}
	    end
	 end

	 meth get(...)=M
	    lock
	       A B
	    in
	       {SplitParams M [width] A B}
	       QTkClass,A
	       {Assert self.widgetType self.typeInfo B}
	       {Record.forAllInd B
		proc{$ I V}
		   case I
		   of width then if self.Horiz then
				    {ReturnTk self cget("-height" V) pixel}
				 else
				    {ReturnTk self cget("-width" V) pixel}
				 end
		   end
		end}
	    end
	 end
   
      end
   end
   
   Register=[r(widgetType:tdspace
	       feature:false
	       widget:{QTkSpace tdspace})
	     r(widgetType:lrspace
	       feature:false
	       widget:{QTkSpace lrspace})
	     r(widgetType:tdline
	       feature:false
	       widget:{QTkSpace tdline})
	     r(widgetType:lrline
	       feature:false
	       widget:{QTkSpace lrline})]

end
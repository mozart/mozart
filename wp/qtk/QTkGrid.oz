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
	    mapLabelToObject:   MapLabelToObject
	    grid:               Grid
	    builder:            Builder
	    execTk:             ExecTk
	    returnTk:           ReturnTk
	    qTkClass:           QTkClass
	    globalInitType:     GlobalInitType
	    globalUnsetType:    GlobalUnsetType
	    globalUngetType:    GlobalUngetType)

export
   Register
   
define

   WidgetType=grid
   Feature=true
   NoArgs={NewName}

   class QTkGrid
   
      from Tk.frame QTkClass

      prop locking

      feat
	 widgetType:WidgetType
	 typeInfo:r(all:{Record.adjoin GlobalInitType
			 r(borderwidth:pixel
			   cursor:cursor
			   highlightbackground:color
			   highlightcolor:color
			   highlightthickness:pixel
			   relief:relief
			   takefocus:boolean
			   background:color bg:color
			   'class':atom
			   colormap:no
			   height:pixel
			   width:pixel
			   visual:no)}
		    uninit:r
		    unset:{Record.adjoin GlobalUnsetType
			   r('class':unit
			     colormap:unit
			     container:unit
			     visual:unit)}
		    unget:{Record.adjoin GlobalUngetType
			   r(bitmap:unit)})
      attr Children Pack
	 
      meth !Init(...)=M
	 lock
	    A B C
	 in
	    {Record.partitionInd M
	     fun{$ I _} {Int.is I} end C A}
	    B={Record.toList C}
	    QTkClass,A
	    Tk.frame,{TkInit A}
	    %% B contains the structure of
	    %% creates the children
	    Children<-nil
	    local
	       proc{Loop X Y L}
		  case L
		  of empty(...)|Ls then
		     {Loop X+1 Y Ls}
		  [] newline(...)|Ls then
		     {Loop 1 Y+1 Ls}
		  [] L|Ls then
		     {self configure(L column:X row:Y sticky:{CondSelect L glue ""})}
		     {Loop X+1 Y Ls}
		  else skip end
	       end
	    in
	       {Loop 1 1 B}
	    end
	 end
      end

      meth bbox(...)=M
	 N={List.toTuple bbox self|{Record.toList M}}
      in
	 {ReturnTk grid N listInt}
      end

      meth rowconfigure(N minsize:_<=NoArgs weight:_<=NoArgs pad:_<=NoArgs)=M
	 {self.parent.Builder {Record.adjoin
			       {Record.filterInd M fun{$ I V} (I\=1) andthen (V\=NoArgs) end}
			       Grid(rowconfigure self N)}}
      end

      meth columnconfigure(N minsize:_<=NoArgs weight:_<=NoArgs pad:_<=NoArgs)=M
	 {self.parent.Builder {Record.adjoin
			       {Record.filterInd M fun{$ I V} (I\=1) andthen (V\=NoArgs) end}
			       Grid(columnconfigure self N)}}
      end

      meth configure(...)=M
	 A B C
      in
	 {Record.partitionInd M
	  fun{$ I _} {Int.is I} end A B}
	 C={List.toTuple Grid
	    configure|{List.map {Record.toList A}
		       fun{$ V}
			  NC
		       in
			  if {Object.is V} andthen {HasFeature V parent} then
			     if {List.member V @Children} then
				NC=V
			     end
			  else
			     NC={self.parent.Builder
				 MapLabelToObject({Record.adjoinAt V parent self} $)}
			     Children<-NC|@Children
			  end
			  if {IsFree NC} then {Exception.raiseError qtk(badParameter V self.widgetType M)} end
			  NC
		       end}}
	 {self.parent.Builder {Record.adjoin B C}}
      end
      meth forget(...)=M
	 {Record.forAllInd M
	  proc{$ I V}
	     if {Not {Int.is I}} then {Exception.raiseError qtk(badParameter I self.widgetType M)} end
	     if {Not {List.member V @Children}} then {Exception.raiseError qtk(badParameter V self.widgetType M)} end
	     {self.parent.Builder Grid(forget V)}
	     Children<-{List.subtract @Children V}
	     {V destroy}
	  end}
      end

      meth location(X Y $)
	 {ReturnTk grid location(self X Y) int}
      end

      meth propagate(B)
	 {self.parent.Builder grid(propagate self B)}
      end

      meth remove(...)=M
	 {Record.forAllInd M
	  proc{$ I V}
	     if {Not {Int.is I}} then {Exception.raiseError qtk(badParameter I self.widgetType M)} end
	     if {Not {List.member V @Children}} then {Exception.raiseError qtk(badParameter V self.widgetType M)} end
	     {self.parent.Builder Grid(remove V)}
	  end}
      end

      meth size($)
	 {ReturnTk grid size(self $) listInt}
      end

      meth slaves($)
	 @Children
      end
      
      meth destroy
	 lock
	    {ForAll @Children proc{$ C}
				 try
				    {C destroy}
				 catch _ then skip end
			      end}
	 end
      end

   end

   Register=[r(widgetType:WidgetType
	       feature:Feature
	       widget:QTkGrid)]

end

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


%declare
%
%T={New Tk.toplevel tkInit}
%C={New Tk.canvas tkInit(parent:T)}
%
%{Tk.send pack(C)}
%
%{Browse {C tkReturn(create(rectangle 10 10 100 100) $)}}
%
%{C tk(itemconfigure 1 fill:red)}

functor

import
   Tk
   QTkDevel(splitParams:        SplitParams
	    tkInit:             TkInit
	    lastInt:            LastInt
	    init:               Init
	    execTk:             ExecTk
	    returnTk:           ReturnTk
	    mapLabelToObject:   MapLabelToObject
	    builder:            Builder
	    qTkClass:           QTkClass
	    qTkAction:          QTkAction
	    globalInitType:     GlobalInitType
	    globalUnsetType:    GlobalUnsetType
	    globalUngetType:    GlobalUngetType)
   
export
   Register
   
define

   WidgetType=canvas
   Feature=scroll
   Bind={NewName}
   This={NewName}

   class Noop end
   
   fun{NewTagOrId From Parent}
      %% FromClass is either Tk.canvasTag
      %% or a number
      FromClass=if {Int.is From} then Noop else From end
      class TagOrId
	 from FromClass QTkClass
	 feat
	    cvtType:r(extent:natural
		      fill:colortrans
		      outline:color
		      outlinestipple:bitmap
		      start:natural
		      stipple:bitmap
		      style:atom
		      width:natural
		      anchor:nswe
		      background:color
		      bitmap:bitmap
		      foreground:color
		      image:image
		      arrow:atom
		      arrowshape:listInt
		      capstyle:atom
		      joinstyle:atom
		      smooth:boolean
		      splinesteps:natural
		      justify:atom
		      text:vs
		      height:natural)
	    widgetType:canvasTag
	    !This
	 meth !Init(...)=M
	    lock
	       QTkClass,M
	       if {Int.is From} then
		  self.This=From
	       else
		  self.This=self
		  Tk.canvasTag,{Record.adjoin M tkInit}
	       end
	    end
	 end
	 meth set(...)=M
	    lock
	       if {Record.someInd M
		   fun{$ I _}
		      {Int.is I}
		   end}
	       then
		  {Exception.raiseError qtk(badParameter 1 canvasTag M)}
	       else
		  {ExecTk Parent {Record.adjoin M itemconfigure(self.This)}}
	       end
	    end
	 end
	 meth get(...)=M
	    lock
	       {Record.forAllInd M
		proc{$ I R}
		   if {HasFeature self.cvtType I} then
		      R={ReturnTk Parent itemcget(self.This "-"#I $) self.cvtType.I}
		   elseif I==blackbox then
		      QTkCanvas,get(I:R)
		   else
		      {Exception.raiseError qtk(ungettableParameter I canvasTag M)}
		   end
		end}
	    end
	 end
	 meth TExecTk(M)
	    Lb={Label M}
	 in
	    if Lb==M then
	       {ExecTk Parent b([Lb self.This])}
	    else
	       {ExecTk Parent b([Lb self.This d(M)])}
	    end
	 end
	 meth TReturnTk(M Type)
	    L={LastInt M}
	    Lb={Label M}
	    R={Record.subtract M L}
	 in
	    if R==Lb then
	       {ReturnTk Parent o(Lb self.This M.L) Type}
	    else
	       {ReturnTk Parent o(Lb self.This d({Record.subtract M L}) M.L) Type}
	    end
	 end
	 meth TMapExecTk(M)
	    {self TExecTk({Record.map M
			   fun{$ P}
			      if {IsDet P} andthen {HasFeature P This} then P.This else P end
			   end})}
	 end
	 meth TMapReturnTk(M Type)
	    {self TReturnTk({Record.map M
			     fun{$ P}
				if {IsDet P} andthen {HasFeature P This} then P.This else P end
			     end} Type)}
	 end
	 meth addtag(...)=M
	    lock
	       {self TMapExecTk(M)}
	    end
	 end
	 meth bbox(...)=M
	    lock
	       {self TMapReturnTk(M listInt)}
	    end
	 end
	 meth bind(...)=M
	    lock
	       {Parent Bind(self.This M)}
	    end
	 end
	 meth delete(...)=M
	    lock
	       {self TExecTk(M)}
	    end
	 end
	 %% coords split in two to reflect whether we want to get or to set the coords
	 meth getCoords(...)=M
	    lock
	       {self TReturnTk({Record.adjoin M coords} listFloat)}
	    end
	 end
	 meth setCoords(...)=M
	    lock
	       {self TExecTk({Record.adjoin M coords})}
	    end
	 end
	 meth dchars(...)=M
	    lock
	       {self TExecTk(M)}
	    end
	 end
	 meth focus=M
	    lock
	       {self TExecTk(M)}
	    end
	 end
	 meth blur
	    lock
	       {self TExecTk(focus('""'))}
	    end
	 end
	 meth icursor(...)=M
	    lock
	       {self TExecTk(M)}
	    end
	 end
	 meth index(...)=M
	    lock
	       {self TReturnTk(M int)}
	    end
	 end
	 meth insert(...)=M
	    lock
	       {self TExecTk(M)}
	    end
	 end	    
	 meth lower(...)=M
	    lock
	       {self TMapExecTk(M)}
	    end
	 end
	 meth move(...)=M
	    lock
	       {self TExecTk(M)}
	    end
	 end
	 meth 'raise'(...)=M
	    lock
	       {self TMapExecTk(M)}
	    end
	 end
	 meth scale(...)=M
	    lock
	       {self TExecTk(M)}
	    end
	 end
	 meth type(...)=M
	    lock
	       {self TReturnTk(M atom)}
	    end
	 end
      end
   in	    
      {New TagOrId Init(parent:Parent)}
   end

   Id={NewCell 0}
   fun{GetUniqueId}
      O N
   in
      {Exchange Id O N}
      N=O+1
      {VirtualString.toString "cid"#N}
   end
   
   class QTkCanvas

      feat
	 widgetType:WidgetType
	 typeInfo:r(all:{Record.adjoin GlobalInitType
			 r(background:color bg:color
			   borderwidth:pixel
			   cursor:cursor
			   highlightbackground:color
			   highlightcolor:color
			   highlightthickness:pixel
			   insertbackground:color
			   insertborderwidth:pixel
			   insertofftime:natural
			   insertontime:natural
			   insertwidth:pixel
			   relief:relief
			   selectbackground:color
			   selectborderwidth:pixel
			   selectforeground:color
			   takefocus:boolean
			   closeenough:float
			   confine:boolean
			   height:pixel
			   scrollregion:scrollregion
			   width:pixel
			   xscrollincrement:pixel
			   yscrollincrement:pixel
			   lrscrollbar:boolean
			   tdscrollbar:boolean
			   scrollwidth:pixel)}
		    uninit:r
		    unset:{Record.adjoin GlobalUnsetType
			   r(lrscrollbar:unit
			     tdscrollbar:unit
			     scrollwidth:unit)}
		    unget:{Record.adjoin GlobalUngetType
			   r(lrscrollbar:unit
			     tdscrollbar:unit
			     scrollwidth:unit)}
		   )
      attr idList
	 
      from Tk.canvas QTkClass
      
      meth !Init(...)=M
	 lock
	    A
	 in
	    QTkClass,M
	    {SplitParams M [lrscrollbar tdscrollbar scrollwidth] A _}
	    Tk.canvas,{TkInit A}
	    idList<-nil
	 end
      end

      meth !Bind(What M)
	 if {HasFeature M event}==false then
	    {Exception.raiseError qtk(missingParameter event canvas M)}
	 else skip end
	 if {Int.is What} then
	    fun {GetFields Ts} % copy/paste from Tk module
	       case Ts of nil then ''
	       [] T|Tr then
		  ' %' # case T
			 of list(T) then
			    case T
			    of atom(A)   then A
			    [] int(I)    then I
			    [] float(F)  then F
			    [] string(S) then S
			    else T
			    end
			 [] string(S) then S
			 [] atom(A)   then A
			 [] int(I)    then I
			 [] float(F)  then F
			 else T
			 end # {GetFields Tr}
	       end
	    end
	    Id={GetUniqueId}
	    Args={CondSelect M args nil}
	    Command={Tk.defineUserCmd Id
		     {{New QTkAction init(parent:self
					  action:{CondSelect M action proc{$} skip end})} action($)}
		     Args}
	    O N
	 in
	    O=idList<-N
	    N=Command|O
	    {self tk(bind What M.event q(Id v({GetFields Args})))}
	 else
	    {What tkBind(event:M.event
			 args:{CondSelect M args nil}
			 action:{{New QTkAction init(parent:self
						     action:{CondSelect M action proc{$} skip end})} action($)})}
	 end
      end

      meth destroy
	 lock
	    {ForAll @idList proc{$ C} {C} end}
	 end
      end

      meth bind(...)=M
	 lock
	    {self Bind(self M)}
	 end
      end
	 
      meth newTag(Tag)
	 lock
	    Tag={NewTagOrId Tk.canvasTag self}
	 end
      end
      
      %% interface toward tk commands

      meth canvasx(...)=M
	 lock
	    {ReturnTk self M natural}
	 end
      end

      meth canvasy(...)=M
	 lock
	    {ReturnTk self M natural}
	 end
      end

      meth create(...)=M
	 lock
	    Mx=if {HasFeature M 1} andthen M.1==window andthen
		  {HasFeature M 2} andthen {HasFeature M 3} andthen
		  {HasFeature M window} then
		  %% window creation is a bit different
		  {Record.adjoinAt M window {self.toplevel.Builder MapLabelToObject({Record.adjoinAt M.window parent self} $)}}
	       else
		  M
	       end
	 in
	    if {HasFeature Mx handle} then
	       N={LastInt Mx}
	       H
	       {ReturnTk self {Record.adjoinAt {Record.subtract Mx handle} N+1 H} int}
	    in
	       Mx.handle={NewTagOrId H self}
	    else
	       {ExecTk self Mx}
	    end
	 end
      end

      % find not supplied : it returns Tk tags and they can't be transformed into the corresponding Oz objects

      meth focus=M
	 lock
	    {ExecTk self M}
	 end
      end

      % gettags not supplied : same reason as find

      % itemcget and itemconfigure are included in the tag object

      meth postscript(...)=M
	 lock
	    {ExecTk self M}
	 end
      end
      
      meth scan(...)=M
	 lock
	    {ExecTk self M}
	 end
      end

      meth select(...)=M
	 lock
	    {ExecTk self M}
	 end
      end
      
   end

   Register=[r(widgetType:WidgetType
	       feature:Feature
	       widget:QTkCanvas)]

end

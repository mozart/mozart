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
   QTkImage(getImageFromTclId:  GetImageFromTclId)
   QTkDevel(splitParams:        SplitParams
	    tkInit:             TkInit
	    lastInt:            LastInt
	    init:               Init
	    execTk:             ExecTk
	    returnTk:           ReturnTk
	    mapLabelToObject:   MapLabelToObject
	    newResource:        NewResource
	    getObjectFromTclId: GetObjectFromTclId
	    builder:            Builder
	    qTkClass:           QTkClass
	    qTkAction:          QTkAction
	    notifyResource:     NotifyResource
	    globalInitType:     GlobalInitType
	    globalUnsetType:    GlobalUnsetType
	    globalUngetType:    GlobalUngetType)
   QTkFont(getFont:             GetFont)
   TkBoot at 'x-oz://boot/Tk'
   System(show:Show)
   
export
   Register

define

   WidgetType=canvas
   Feature=scroll
   Bind={NewName}
   This={NewName}
   TclName
   {TkBoot.getNames _ _ TclName}

   class Noop end
   
   class TagOrId
      from QTkClass
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
	       {ExecTk self.parent {Record.adjoin M itemconfigure(self.This)}}
	    end
	 end
      end
      meth get(...)=M
	 lock
	    {Record.forAllInd M
	     proc{$ I R}
		if {HasFeature self.cvtType I} then
		   R={ReturnTk self.parent itemcget(self.This "-"#I $) self.cvtType.I}
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
	    {ExecTk self.parent b([Lb self.This])}
	 else
	    {ExecTk self.parent b([Lb self.This d(M)])}
	 end
      end
      meth TReturnTk(M Type)
	 L={LastInt M}
	 Lb={Label M}
	 R={Record.subtract M L}
      in
	 if R==Lb then
	    {ReturnTk self.parent o(Lb self.This M.L) Type}
	 else
	    {ReturnTk self.parent o(Lb self.This d({Record.subtract M L}) M.L) Type}
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
      meth dtag(...)=M
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
	    {self.parent Bind(self M)}
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

   class Tag
      from TagOrId Tk.canvasTag
      meth !Init(...)=M
	 TagOrId,M
	 self.This=self
	 Tk.canvasTag,{Record.adjoin M tkInit}
      end
   end

   class Id
      feat !TclName
      from TagOrId
      meth !Init(...)=M
	 TagOrId,{Record.subtract M 1}
	 self.This=M.1
	 self.TclName={VirtualString.toAtom {Tk.getTclName self.parent}#"."#self.This}
      end
   end

   fun{NewTag Parent}
      {Parent.Builder NewResource(Parent
				  ct
				  Init(parent:Parent) $)}
   end

   fun{NewId N Parent}
      {Parent.Builder NewResource(Parent
				  ch
				  Init(N parent:Parent) $)}
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
			   scrollwidth:pixel
			  )}
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
	 
      from Tk.canvas QTkClass
      
      meth !Init(...)=M
	 lock
	    A
	 in
	    QTkClass,M
	    {SplitParams M [lrscrollbar tdscrollbar scrollwidth] A _}
	    Tk.canvas,{TkInit A}
	 end
      end
      
      meth !Bind(What M)
	 if {HasFeature M event}==false then
	    {Exception.raiseError qtk(missingParameter event canvas M)}
	 else skip end
	 {What tkBind(event:M.event
		      args:{CondSelect M args nil}
		      action:{{New QTkAction init(parent:self
						  action:{CondSelect M action proc{$} skip end})} action($)})}
      end

      meth bind(...)=M
	 lock
	    {self Bind(self M)}
	 end
      end
	 
      meth newTag(Tag)
	 lock
	    Tag={NewTag self}
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
		  {Record.adjoinAt M window {self.parent.Builder MapLabelToObject({Record.adjoinAt M.window parent self} $)}}
	       else
		  M
	       end
	 in
	    if {HasFeature Mx handle} then
	       N={LastInt Mx}
	       H
	       {ReturnTk self {Record.adjoinAt {Record.subtract Mx handle} N+1 H} int}
	    in
	       Mx.handle={NewId H self}
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

      meth dump(Ret handles:Handle<=false tags:Tags<=false)
	 %% this method binds Ret to a list of commands
	 %% that can recreate the current content of the canvas
	 lock
	    Init={self tkReturnListInt(find(closest 0 0) $)}
	 in
	    if Init==nil then Ret=nil % canvas is empty
	    else
	       fun{Loop T}
		  N={self tkReturnListInt(find(below T) $)}
	       in
		  if N==nil then
		     T
		  else
		     {Loop N.1}
		  end
	       end
	       fun{Parse T}
		  fun{GetParams}
		     fun{Grab L G}
			L1={List.dropWhile L fun{$ C} C==&  end}
		     in
			if L1.1==&{ then
			   fun{Loop L Width R}
			      case L
			      of &{|Ls then
				 &{|{Loop Ls Width+1 R}
			      [] &}|Ls then
				 if Width==0 then
				    R=Ls.2
				    nil
				 else
				    &}|{Loop Ls Width-1 R}
				 end
			      [] L|Ls then
				 L|{Loop Ls Width R}
			      end
			   end
			in
			   {Loop L1.2 0 $ G}
			else
			   {List.takeDropWhile L1 fun{$ C} C\=&} andthen C\=& end G}
			end
		     end
		     fun{Loop L}
			%% from a string by the itemconfigure tk command
			%% create an equivalent Oz list of parameter#value
			case L
			of &{|&-|Ls then
			   Cmd R Remain P
			in
			   {List.takeDropWhile Ls fun{$ C} C\=& end Cmd R}
			   Remain={Grab {Grab {Grab {Grab R _} _} _} P}
			   {String.toAtom Cmd}#P|{Loop Remain}
			[] L|Ls then
			   {Loop Ls}
			else nil end
		     end
		     fun{Resource L}
			I#V|Ls=L
			Ob={self.parent.Builder GetObjectFromTclId(V $)}
		     in
			if Ob\=unit then
			   I#Ob|{MapAndFilter Ls}
			else
			   %% resource is unknown by QTk, parameter is ignored
			   {MapAndFilter Ls}
			end
		     end
		     fun{ImageResource L}
			I#V|Ls=L
			Ob={GetImageFromTclId V}
		     in
			if Ob\=unit then
			   I#Ob|{MapAndFilter Ls}
			else
			   %% image is unknown by QTk, parameter is ignored
			   {MapAndFilter Ls}
			end
		     end
		     fun{MapAndFilter L}
			%% parse the list of parameter#value
			case L
			of tags#V|Ls then
			   if Tags then
			      fun{Loop R}
				 A B
			      in
				 {List.takeDropWhile {List.dropWhile R fun{$ C} C==&  end}
				  fun{$ C} C\=&  end A B}
				 if A==nil then nil
				 else
				    Obj
				 in
				    {self.parent.Builder GetObjectFromTclId(A Obj)}
				    if Obj\=unit then
				       Obj|{Loop B}
				    else
				       {Loop B}
				    end
				 end
			      end
			      R={Loop V}
			   in
			      if R==nil then
				 {MapAndFilter Ls}
			      else
				 tags#{List.toTuple q R}|{MapAndFilter Ls}
			      end
			   else
			      {MapAndFilter Ls}
			   end
			[] image#_|Ls then {ImageResource L}
			[] activeimage#_|Ls then {ImageResource L}
			[] disabledimage#_|Ls then {ImageResource L}
			[] window#_|Ls then {Resource L}
			[] smooth#V|Ls then smooth#if V=="bezier" then "1" else "0" end|{MapAndFilter Ls}
			[] font#V|Ls then font#{GetFont V}|Ls
			[] _#nil|Ls then {MapAndFilter Ls}
			[] L|Ls then L|{MapAndFilter Ls}
			else nil end
		     end
		     R={self tkReturn(itemconfigure(T) $)}
		  in
		     {MapAndFilter {Loop R}}
		  end
		  Type={self tkReturnAtom(type(T) $)}
		  RetI=create(Type
			      b({self tkReturnListFloat(coords(T) $)}))
		  RetP={GetParams}
		  RetR={Record.adjoin {List.toRecord r RetP} RetI}
		  Ret=if Handle then
			 Ob={self.parent.Builder GetObjectFromTclId({VirtualString.toAtom {Tk.getTclName self}#"."#T} $)}
		      in
			 if Ob==unit then
			    RetR
			 else
			    {Record.adjoinAt RetR handle Ob}
			 end
		      else
			 RetR
		      end
		  N={self tkReturnListInt(find(above T) $)}
	       in
		  if N==nil then
		     Ret|nil
		  else
		     Ret|{Parse N.1}
		  end
	       end
	    in
	       Ret={Parse {Loop Init.1}}
	    end
	 end
      end
      
   end
   
   Register=[r(widgetType:WidgetType
	       feature:Feature
	       widget:QTkCanvas
	       resource:[ch#Id
			 ct#Tag]
	      )]
   
end

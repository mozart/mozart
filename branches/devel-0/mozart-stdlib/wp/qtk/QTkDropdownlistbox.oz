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
   QTkBare
   QTkImage
   QTkDevel(assert:             Assert
	    init:               Init
	    qTkDesc:            QTkDesc
	    subtracts:          Subtracts
	    globalInitType:     GlobalInitType
	    globalUnsetType:    GlobalUnsetType
	    globalUngetType:    GlobalUngetType)

export
   Register

require QTkDropdownbutton_bitmap

prepare BL=QTkDropdownbutton_bitmap.buildLibrary

define
   QTk=QTkBare
   WidgetType=dropdownlistbox
   Feature=false
   Lib={QTkImage.buildImageLibrary BL}
   
   fun{FilterButton Rec}
      %% pre : record with features whose name begin with button and other features
      %% post : a pair of record where all features that begin with button are in the
      %% second record and are removed the first 6 letters (i.e. button). The first
      %% record contains the remaining features
      A B
   in
      {Record.partitionInd Rec fun{$ I _}
				  {List.take {VirtualString.toString I} 6}=="button"
			       end B A}
      A#{List.toRecord {Label B}
	 {List.map {Record.toListInd B}
	  fun{$ I}
	     A B
	  in
	     A#B=I
	     {VirtualString.toAtom {List.drop {VirtualString.toString A} 6}}#B
	  end}}
   end
   
   class QTkDropdownlistbox

      feat
	 widgetType:WidgetType
	 typeInfo:r(all:{Record.adjoin GlobalInitType
			 r(1:listVs      %% parameters specific to the listbox
			   init:listVs   %% copy/paste from QTkListbox.oz :-)
			   return:free
			   reload:listVs
			   firstselection:natural
			   background:color bg:color
			   borderwidth:pixel
			   cursor:cursor
			   exportselection:boolean
			   font:font
			   height:natural
			   highlightbackground:color
			   highlightcolor:color
			   highlightthickness:pixel
			   relief:relief
			   selectbackground:color
			   selectborderwidth:pixel
			   selectforeground:color
			   setgrid:boolean
			   takefocus:boolean
			   width:natural
			   selectmode:[single browse multiple extended]
			   action:action
			   lrscrollbar:boolean
			   tdscrollbar:boolean
			   scrollwidth:pixel
			   %% parameters specific to the button
			   buttonactivebackground:color
			   buttonactiveforeground:color
			   buttonbackground:color
			   buttonforeground:color
			   buttondisabledforeground:color
			   buttonhighlightbackground:color
			   buttonhighlightcolor:color
			   buttonhighlightthickness:pixel
			   buttontakefocus:boolean
			   buttondefault:[normal disabled active]
			   buttonstate:[normal disabled active])}
		    uninit:r(1:unit
			     reload:unit
			     firstselection:unit)
		    unset:{Record.adjoin GlobalUnsetType
			   r(lrscrollbar:unit
			     tdscrollbar:unit
			     scrollwidth:unit
			     init:unit
			     reload:unit
			     firstselection:unit
			     )}
		    unget:{Record.adjoin GlobalUngetType
			   r(lrscrollbar:unit
			     tdscrollbar:unit
			     scrollwidth:unit
			     init:unit
			     selectmode:unit)}
		   )
	 action
	 Button
	 Window
	 tdscrollbar lrscrollbar

      attr inside
   
      meth dropdownlistbox(...)=M
	 lock
	    A B
	 in
	    {Assert self.widgetType self.typeInfo {Record.subtract {Record.adjoin M Init} QTkDesc}}
	    A#B={FilterButton {Record.subtract M QTkDesc}}
	    self.action={CondSelect A action proc{$} skip end}
	    M.QTkDesc={Record.adjoin B button(image:{Lib get(name:'mini-down.xbm' image:$)}
					      handle:self.Button
					      action:self#DropDown)}
	    self.Window={QTk.build td(overrideredirect:true
				      {Record.adjoin {Subtracts A [handle feature]}
				       listbox(glue:nswe
					       action:self#Execute
					       feature:list)})}
	    {self.Window bind(event:'<Enter>' action:self#Inside(true))}
	    {self.Window bind(event:'<Leave>' action:self#Inside(false))}
	    {self.Window bind(event:'<ButtonRelease-1>' action:self#CondClose)}
	    self.tdscrollbar={CondSelect self.Window.list tdscrollbar unit}
	    self.lrscrollbar={CondSelect self.Window.list lrscrollbar unit}
	 end
      end
      meth DropDown
	 lock
	    proc{D}
	       BX BY BW BH SW SH
	       {self.Button winfo(rootx:BX rooty:BY
				  width:BW height:BH
				  screenwidth:SW screenheight:SH)}
	       WW WH
	       {self.Window winfo(width:WW height:WH)}
	       X1=BX+BW-WW
	       X2=if X1<0 then 0 else X1 end
	       WX=if X2+WW>SW then SW-WW else X2 end
	       Y1=BY+BH
	       WY=if Y1+WH>SH then SH-WH else Y1 end
	    in
	       {self.Window.list set(selection:nil)}
	       {self.Window set(geometry:geometry(x:WX y:WY))}
	       {self.Window show(modal:true)}
	       {self.Window set(geometry:geometry(x:WX y:WY))}
	       {self.Window 'raise'}
	    end
	 in
	    {D}
	    {D}
	    inside<-false
	 end
      end

      meth Close
	 try
	    {self.Window releaseGrab}
	    {self.Window hide}
	 catch _ then skip end
      end
      meth Inside(Val)
	 inside<-Val
      end
      meth CondClose
	 if @inside==false then {self Close} end
      end

      meth Execute
	 lock
	    {self Close}
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
      end

      meth set(...)=M
	 lock
	    A B
	 in
	    {Assert self.widgetType self.typeInfo M}
	    A#B={FilterButton M}
	    {self.Button B}
	    {self.Window.list A}
	 end
      end

      meth get(...)=M
	 lock
	    A B
	 in
	    {Assert self.widgetType self.typeInfo M}
	    A#B={FilterButton M}
	    {self.Button B}
	    {self.Window.list A}
	 end
      end

      meth otherwise(M)
	 lock
	    {self.Window.list M}
	 end
      end
   
   end
   
   Register=[r(widgetType:WidgetType
	       feature:Feature
	       widget:QTkDropdownlistbox)]
   
end

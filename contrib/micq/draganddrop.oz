%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1998
%%%   Simon Lindblom, 1998
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
%%%

functor

import
   Property(get)
export
   dragAndDrop:DragAndDrop
define
   class DragAndDrop

      feat
	 Source:{NewCell unit}
	 Destination:{NewCell unit}

      meth dragAndDropInit() L=self in
	 {L tkBind(event:'<ButtonPress-1>'
		   action:proc{$} {Assign self.Destination _} {Assign self.Source L} end)}
	 {L tkBind(event:'<ButtonRelease-1>'
		   action:proc{$} L1={Access self.Destination} T0={Property.get 'time.total'} in
			     if L\=L1 andthen {Property.get 'time.total'}-T0<15 then
				try {L1 setState({L getState($)})} catch _ then skip end
			     end
			  end)}
	 {L tkBind(event:'<Enter>'
		   action:proc{$} L1={Access self.Destination} in
			     if {IsDet L1}==false andthen L\={Access self.Source} then L=L1 end
			  end)}
      end
   end
end


/*
%%
%% ozc -c draganddrop.oz -o draganddrop.ozf
%%

declare

[DD]={Module.link ['draganddrop.ozf']}
DragAndDrop=DD.dragAndDrop

class Label from Tk.label DragAndDrop
   prop final
   attr State
      
   meth tkInit(text:T ...)=M
      Tk.label, M
      DragAndDrop, dragAndDropInit()
      State<-T
   end
%    meth setState(T)
%       State<-T
%       {self tk(config text:T)}
%    end
   meth getState($) @State end
end

class Entry from Tk.entry DragAndDrop
   prop final
   attr Variable
   meth tkInit(textvariable:V ...)=M
      Tk.label, M
      DragAndDrop, dragAndDropInit()
      Variable<-V
   end
   meth getState($)
      {@Variable tkReturnString($)}
   end
   meth setState(X)
      {@Variable tkSet(X)}
   end
end

T={New Tk.toplevel tkInit(title:"Test")}

L1={New Label tkInit(parent:T text:"Hello")}

L2={New Label tkInit(parent:T text:"World")}

V4={New Tk.variable tkInit("Foo")}
L4={New Entry tkInit(parent:T textvariable:V4)}

V5={New Tk.variable tkInit("Bar")}
L5={New Entry tkInit(parent:T textvariable:V5)}

{Tk.send pack(L1 L2 L4 L5 fill:x)}
*/




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
require
   Meths(addApplication:S_addApplication) at 'methods.ozf'
import
   Tk
export
   start:Start
define
   proc{Start Author Server}
      T={New Tk.toplevel tkInit(title:"Add Application")}
      V1 V2 V3 V4 
      Index={NewCell 0}
      GO
      proc{Start2}
	 A=S_addApplication(name: {V1 tkReturnString($)}
			  serverurl:{V2 tkReturnString($)}
			  clienturl:{V3 tkReturnString($)}
			  author: Author
			  description: {V4 tkReturnString($)})
      in
	 {Wait A.name} {Wait A.serverurl} {Wait A.clienturl}
	 {Wait A.description}
	 {T tkClose}

	 {Server A}
      end

      proc{NewEntry Title Value V}
	 O N E L={New Tk.label tkInit(parent:T text:Title)}
      in
	 {Exchange Index O N} N=O+1
	 V={New Tk.variable tkInit(Value)}
	 E={New Tk.entry tkInit(parent:T width:50 textvariable:V)}
	 {Tk.batch [grid(L row:N column:0 sticky:e)
		    grid(E row:N column:1 sticky:w)]}
	 {E tkBind(event:'<Return>' action:proc{$} GO=unit end)}
	 if N==1 then {Tk.send focus(E)} else skip end
      end
   in
      V1={NewEntry "Application Name:" ""}
      V4={NewEntry "Description:" ""}
      V2={NewEntry "Server URL:" "http://"}
      V3={NewEntry "Client URL:" "http://"}
      
      {Wait GO}
      {Start2}
   end 
end

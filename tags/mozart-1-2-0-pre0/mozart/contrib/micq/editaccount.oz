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
   Meths(updateUser:S_updateUser) at 'methods.ozf'
import
   Tk
   OS(getEnv)
export
   start:Start
define
   Organization=case {OS.getEnv 'ORGANIZATION'} of false then "" elseof X then X end
   
   proc{Start Info Server Client}

      T={New Tk.toplevel tkInit(title:"Edit Account")}
      V2 V3 V4 V5 V6 V7
      Index={NewCell 0}
      GO
      proc{Start2}
	 A=S_updateUser(id:Info.id
			passwd:{V2 tkReturnAtom($)}
			firstname:{V3 tkReturnString($)}
			lastname:{V4 tkReturnString($)}
			organization:{V5 tkReturnString($)}
			email:{V6 tkReturnString($)}
			userlevel:Info.userlevel
			extra:{V7 tkReturnString($)})
      in
	 {Wait A.passwd} {Wait A.firstname} {Wait A.firstname} {Wait A.organization} {Wait A.email}
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
      V2={NewEntry "Password:" {CondSelect Info  passwd ""}}
      V3={NewEntry "Firstname:" {CondSelect Info firstname ""}}
      V4={NewEntry "Lastname:" {CondSelect Info lastname ""}}
      V5={NewEntry "Organization:" {CondSelect Info organization Organization}}
      V6={NewEntry "Email:" {CondSelect Info email ""}}
%      V7={NewEntry "Civic registration number:" {CondSelect Info extra "<personnummer>"}}
      V7={NewEntry "Phone:" {CondSelect Info extra ""}}
      
      {Wait GO}
      {Start2}
   end 
end

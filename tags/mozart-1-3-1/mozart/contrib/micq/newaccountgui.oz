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
   Meths(addUser:S_addUser) at 'methods.ozf'
import
   Tk
   OS(getEnv)
export
   start:Start
define
   Organization=case {OS.getEnv 'ORGANIZATION'} of false then "" elseof X then X end
		    
   proc{Start Args}
      T={New Tk.toplevel tkInit(title:"New Account...")}
      V1 V2 V3 V4 V5 V6 V7
      Index={NewCell 0}
      GO
      proc{Start2}
	 A=S_addUser(id:{V1 tkReturnAtom($)}
		     passwd:{V2 tkReturnAtom($)}
		     firstname:{V3 tkReturnString($)}
		     lastname:{V4 tkReturnString($)}
		     organization:{V5 tkReturnString($)}
		     email:{V6 tkReturnString($)}
		     userlevel:user
		     extra:{V7 tkReturnString($)})
      in
	 {Wait A.id} {Wait A.passwd} {Wait A.firstname} {Wait A.firstname} {Wait A.organization} {Wait A.email}
	 {T tkClose}
			  
	 try
	    {Args.server A}
	    {Args.client registerclient(id:A.id passwd:A.passwd)}
	 catch idAllreadyInUse(M) then
	    T={New Tk.toplevel tkInit(title:"Error")}
	    L={New Tk.label tkInit(parent:T text:"Login is allready '"#M#"' taken...")}
	    B1={New Tk.button tkInit(parent:T text:"Choose another login" action:proc{$}
										    {T tkClose}
										    {Start {Record.adjoin Args A}}
										 end)}
	    B2={New Tk.button tkInit(parent:T text:"Cancel" action:proc{$}
								      {T tkClose}
								   end)}
	 in
	    {Tk.batch [grid(L row:0 column:0 columnspan:2 sticky:we)
		       grid(B1 row:5 column:0 sticky:we)
		       grid(B2 row:5 column:1 sticky:we)
		       focus(B1)]}
	 end	    
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
      V1={NewEntry "Login:" Args.id}
      V2={NewEntry "Password:" Args.passwd}
      V3={NewEntry "Firstname:" {CondSelect Args firstname ""}}
      V4={NewEntry "Lastname:" {CondSelect Args lastname ""}}
      V5={NewEntry "Organization:" {CondSelect Args organization Organization}}
      V6={NewEntry "Email:" {CondSelect Args email ""}}
%      V7={NewEntry "Civic registration number:" {CondSelect Args extra "<personnummer>"}}
      V7={NewEntry "Phone:" {CondSelect Args extra ""}}
		       
      {Wait GO}
      {Start2}
   end 
end

/*
functor
   
import
   Tk
   OS(getEnv)
export
   start:Start
define
   Organization=case {OS.getEnv 'ORGANIZATION'} of false then "" elseof X then X end
   
   proc{Start Args}
      T={New Tk.toplevel tkInit(title:"New Account...")}
      V1 V2 V3 V4 V5 V6
      Index={NewCell 0}
      GO
      proc{Start2}
	 A=addUser(id:{V1 tkReturnAtom($)}
		   passwd:{V2 tkReturnAtom($)}
		   firstname:{V3 tkReturnString($)}
		   lastname:{V4 tkReturnString($)}
		   organization:{V5 tkReturnString($)}
		   email:{V6 tkReturnString($)}
		   userlevel: user)
      in
	 {Wait A.id} {Wait A.passwd} {Wait A.firstname} {Wait A.firstname} {Wait A.organization} {Wait A.email}
	 {T tkClose}
	 try
	    {Args.server A}
	    {Args.client registerclient(id:A.id passwd:A.passwd)}
	 catch idAllreadyInUse(M) then
	    T={New Tk.toplevel tkInit(title:"Error")}
	    L={New Tk.label tkInit(parent:T text:"Login is allready '"#M#"' taken...")}
	    B1={New Tk.button tkInit(parent:T text:"Choose another login" action:proc{$}
										    {T tkClose}
										    {Start {Record.adjoin Args A}}
										 end)}
	    B2={New Tk.button tkInit(parent:T text:"Cancel" action:proc{$}
								      {T tkClose}
								   end)}
	 in
	    {Tk.batch [grid(L row:0 column:0 columnspan:2 sticky:we)
		       grid(B1 row:5 column:0 sticky:we)
		       grid(B2 row:5 column:1 sticky:we)
		       focus(B1)]}
	 end	    
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
      V1={NewEntry "Login:" Args.id}
      V2={NewEntry "Password:" Args.passwd}
      V3={NewEntry "Firstname:" {CondSelect Args firstname ""}}
      V4={NewEntry "Lastname:" {CondSelect Args lastname ""}}
      V5={NewEntry "Organization:" {CondSelect Args organization Organization}}
      V6={NewEntry "Email:" {CondSelect Args email ""}}
      
      {Wait GO}
      {Start2}
   end 
end

*/





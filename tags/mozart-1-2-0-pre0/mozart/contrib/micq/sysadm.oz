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
   Meths(addUser:S_addUser
	 removeUser:S_removeUser
	 getInfo:S_getInfo
	 updateUser:S_updateUser
	 removeMessage:S_removeMessage
	 removeApplication:S_removeApplication) at 'methods.ozf'
import
   Browser(browse:Browse)
   Tk
   TkTools
   Connection(take)
   AddAppGUI(start) at 'addapplicationgui.ozf'
   EditAppGUI(start) at 'editapplicationgui.ozf'   
export
   start: Start
   stop: Stop
   
define
   Server WaitQuit T
   proc{Start Args Ticket}
      Server={Connection.take Ticket}
   
      thread
	 T1={New Tk.toplevel tkInit(title:"System Administrator" delete:Stop)}
	 T={New Tk.frame tkInit(parent:T1)}
	 proc{Separator Name F}
	    L fun{NS} {New Tk.frame tkInit(parent:F bd:1 relief:sunken height:2 width:10)} end
	 in 
	    F={New Tk.frame tkInit(parent:T)}
	    L={New Tk.label tkInit(parent:F text:Name)}
	    {Tk.batch [grid({NS} row:0 column:0 sticky:we)
		       grid(L    row:0 column:1 padx:4)
		       grid({NS} row:0 column:2 sticky:we)
		       grid(columnconfigure F 0 weight:1)
		       grid(columnconfigure F 2 weight:1)]}
	 end

	 %% Messages
	 MER=2
	 B0={New Tk.button tkInit(parent:T text:"Browse Messages"
				  action:proc{$} {Browse notImplemented} end)}
	 B0a={New Tk.button tkInit(parent:T text:"Remove Message"
				   action:proc{$} {RemoveMessage} end)}
	 B0b={New Tk.button tkInit(parent:T text:"Broadcast Message"
				   action:proc{$} {Browse notImplemented} end)}
	 {Tk.batch [grid({Separator "Messages"} row:MER column:0 columnspan:3 sticky:we pady:3)
		    grid(B0  row:MER+1 column:0 sticky:we)
		    grid(B0a row:MER+1 column:1 sticky:we)
		    grid(B0b row:MER+1 column:2 sticky:we)]}
	 
	 %% Users
	 UR=4
	 B1={New Tk.button tkInit(parent:T text:"Add User"
				  action:proc{$}
					    {AddUser user(id:""
							  passwd:""
							  userlevel:user
							  organization:""
							  email:""
							  firstname:""
							  lastname:"")}
					 end)}
	 B2={New Tk.button tkInit(parent:T text:"Edit User"
				  action:proc{$} {EditUser} end)}
	 B3={New Tk.button tkInit(parent:T text:"Remove user"
				  action:proc{$} {RemoveUser} end)}
	 
	 {Tk.batch [grid({Separator "Users"} row:UR column:0 columnspan:3 sticky:we pady:3)
		    grid(B1 row:UR+1 column:0 sticky:we)
		    grid(B2 row:UR+1 column:1 sticky:we)
		    grid(B3 row:UR+1 column:2 sticky:we)]}


	 %% Applications
	 AR=6
	 B4={New Tk.button tkInit(parent:T text:"Add application"
				  action:proc{$} {AddApplication} end)}
	 B5={New Tk.button tkInit(parent:T text:"Edit application"
				  action:proc{$} {EditApplication} end)}
	 B6={New Tk.button tkInit(parent:T text:"Remove application"
				  action:proc{$} {RemoveApplication} end)}

	 {Tk.batch [grid({Separator "Applications"} row:AR column:0 columnspan:3 sticky:we pady:3)
		    grid(B4 row:AR+1 column:0 sticky:we)
		    grid(B5 row:AR+1 column:1 sticky:we)
		    grid(B6 row:AR+1 column:2 sticky:we)]}

	 %% Done
	 DR=8
	 B7S={New Tk.frame tkInit(parent:T bd:1 relief:sunken height:2 width:10)}
	 B7={New Tk.button tkInit(parent:T fg:red activeforeground:red
				  text:"Halt Sysadministrator" action:proc{$}
									 {T1 tkClose}
									 WaitQuit=unit
								      end)}
      in
	 {Tk.batch [grid(B7S row:DR column:0 columnspan:3 sticky:we pady:7)
		    grid(B7 row:DR+1 column:0 columnspan:3 sticky:we) 
		    grid(T row:0 column:0 sticky:news padx:4 pady:1)
		    grid(columnconfigure T1 0 weight:1)
		    grid(columnconfigure T 0 weight:1)
		    grid(columnconfigure T 1 weight:1)
		    wm(resizable T1 1 0)]}
   
      end
      
      {Wait WaitQuit}
   end
   proc{Stop}
      {T tkClose}
      WaitQuit=unit
   end

   proc {RemoveMessage}
      E L  MDialog
   in
      MDialog = {New TkTools.dialog
		 tkInit(title:   'Remove Message' 
			buttons: ['Okay' # 
				  proc {$} Tmp in
				     Tmp = {E tkReturn(get $)}
				     {Wait Tmp}
				     try
					if {String.isInt Tmp} then 
					   {Server S_removeMessage( mid: {String.toInt Tmp})}
					   {MDialog tkClose}
					end
				     catch _ then skip 
				     end 
				  end 
				  'Cancel' # tkClose]
			default: 1)}
      
      L={New Tk.label tkInit(parent:MDialog text:'Message id:')}
      E={New Tk.entry tkInit(parent:MDialog bg:wheat width:20)}
      
      {Tk.batch [pack(L E side:left pady:2#m) focus(E)]}
   end

   proc{AddUser Args}
      T={New Tk.toplevel tkInit(title:"Add User")}
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
		     extra:nil
		     userlevel: {V7 tkReturnAtom($)})
      in
	 {Wait A.id} {Wait A.passwd} {Wait A.firstname} {Wait A.firstname}
	 {Wait A.organization} {Wait A.email} {Wait A.userlevel}
	 {T tkClose}
	 try
	    {Server A}
	 catch idAllreadyInUse(M) then
	    T={New Tk.toplevel tkInit(title:"Error")}
	    L={New Tk.label tkInit(parent:T text:"Login is allready '"#M#"' taken...")}
	    B1={New Tk.button tkInit(parent:T text:"Choose another login" action:proc{$}
										    {T tkClose}
										    {AddUser {Record.adjoin Args A}}
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
      F1 L1 R1 R2
   in
      V1={NewEntry "Login:" Args.id}
      V2={NewEntry "Password:" Args.passwd}
      V3={NewEntry "Firstname:" {CondSelect Args firstname ""}}
      V4={NewEntry "Lastname:" {CondSelect Args lastname ""}}
      V5={NewEntry "Organization:" {CondSelect Args organization ""}}
      V6={NewEntry "Email:" {CondSelect Args email ""}}

      %% User level
      V7={New Tk.variable tkInit({CondSelect Args userlevel user})}
      L1={New Tk.label tkInit(parent:T text:"User-level:")}
      F1={New Tk.frame tkInit(parent:T relief:sunken bd:2)}
      R1={New Tk.radiobutton tkInit(parent:F1 text:"User" value:user variable:V7)}
      R2={New Tk.radiobutton tkInit(parent:F1 text:"Superuser" value:sysadm variable:V7)}
      {Tk.batch [grid(L1 row:10 column:0 sticky:e)
		 grid(F1 row:10 column:1 sticky:we)
		 grid(R1 row:0 column:0 sticky:we)
		 grid(R2 row:0 column:1 sticky:we)
		]}
      
      %V7={NewEntry "Userlevel:" {CondSelect Args userlevel user}}
      
      {Wait GO}
      {Start2}
   end

   proc{RemoveUser}
      E L  MDialog
   in
      MDialog = {New TkTools.dialog
		 tkInit(title:   'Remove User'
			buttons: ['Okay' # 
				  proc {$} Tmp in
				     Tmp = {E tkReturn(get $)}
				     try
					{Server S_removeUser( id: {String.toAtom Tmp})}
					{MDialog tkClose}
				     catch _ then skip 
				     end 
				  end 
				  'Cancel' # tkClose]
			default: 1)}
      
      L={New Tk.label tkInit(parent:MDialog text:'User id:')}
      E={New Tk.entry tkInit(parent:MDialog bg:wheat width:20)}
      
      {Tk.batch [pack(L E side:left pady:2#m) focus(E)]}
   end

 
   fun{GetUser}
      E L  MDialog Id 
   in
      MDialog = {New TkTools.dialog
		 tkInit(title:   'Edit User'
			buttons: [
				  'Okay' # 
				  proc {$} Tmp in
				     Tmp = {E tkReturn(get $)}
				     {Wait Tmp}
				     Id={String.toAtom Tmp}
				  end 
				  
				  'Cancel' # proc {$}
						Id=unit
					     end
				 ]
			default: 1)}
      
      L={New Tk.label tkInit(parent:MDialog text:'User id:')}
      E={New Tk.entry tkInit(parent:MDialog bg:wheat width:20)}
      
      {Tk.batch [pack(L E side:left pady:2#m) focus(E)]}
      {Wait Id}
      {MDialog tkClose}
      Id
   end
   
   proc {EditUser} Id Info
      Id={GetUser}
      
      T={New Tk.toplevel tkInit(title:"Edit Account")}
      V2 V3 V4 V5 V6 V7 V8
      Index={NewCell 0}
      GO
      proc{Start2}
	    A=S_updateUser(id: Info.id
			   passwd:{V2 tkReturnAtom($)}
			   firstname:{V3 tkReturnString($)}
			   lastname:{V4 tkReturnString($)}
			   organization:{V5 tkReturnString($)}
			   email:{V6 tkReturnString($)}
			   extra:{V7 tkReturnString($)}
			   userlevel:{V8 tkReturnAtom($)})
      in
	 {Wait A.passwd} {Wait A.firstname} {Wait A.firstname} {Wait A.organization}
	 {Wait A.email} {Wait A.userlevel}
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
      F1 L1 R1 R2
   in
      try
	 Info = {Server S_getInfo( id: Id info: $ )}
  
	 V2={NewEntry "Password:" {CondSelect Info  passwd ""}}
	 V3={NewEntry "Firstname:" {CondSelect Info firstname ""}}
	 V4={NewEntry "Lastname:" {CondSelect Info lastname ""}}
	 V5={NewEntry "Organization:" {CondSelect Info organization ""}}
	 V6={NewEntry "Email:" {CondSelect Info email ""}}
	 V7={NewEntry "Extra:" {CondSelect Info extra ""}}

	 %% User level
	 V8={New Tk.variable tkInit({CondSelect Info userlevel user})}
	 L1={New Tk.label tkInit(parent:T text:"User-level:")}
	 F1={New Tk.frame tkInit(parent:T relief:sunken bd:2)}
	 R1={New Tk.radiobutton tkInit(parent:F1 text:"User" value:user variable:V8)}
	 R2={New Tk.radiobutton tkInit(parent:F1 text:"Superuser" value:sysadm variable:V8)}
	 {Tk.batch [grid(L1 row:10 column:0 sticky:e)
		    grid(F1 row:10 column:1 sticky:we)
		    grid(R1 row:0 column:0 sticky:we)
		    grid(R2 row:0 column:1 sticky:we)
		   ]}
	 {Wait GO}
	 {Start2}
      catch _ then {T tkClose} end
   end

   proc{AddApplication}  {AddAppGUI.start sysadm Server} end

   proc{GetAppId Title Id}
      E L  MDialog
   in
      MDialog = {New TkTools.dialog
		 tkInit(title:   Title#' Application'
			buttons: [
				  'Okay' # 
				  proc {$} Tmp in
				     Tmp = {E tkReturn(get $)}
				     {Wait Tmp}
				     if {String.isInt Tmp} then 
					Id={String.toInt Tmp}
					{MDialog tkClose}
				     end
				  end 
				  
				  'Cancel' # proc {$}
						Id=unit
					     end
				 ]
			default: 1)}
      
      L={New Tk.label tkInit(parent:MDialog text:'Application id:')}
      E={New Tk.entry tkInit(parent:MDialog bg:wheat width:20)}
      
      {Tk.batch [pack(L E side:left pady:2#m) focus(E)]}
      {Wait Id}
      {MDialog tkClose}
   end
   
   proc{EditApplication} Id = {GetAppId "Edit"} in
      {EditAppGUI.start Id Server}
   end

   proc{RemoveApplication} Id = {GetAppId "Remove"} in
      {Server S_removeApplication(aid: Id id:sysadm)}
   end
end
     
	 
    


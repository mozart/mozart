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
   Meths(addFriends:S_addFriends searchFriends:S_searchFriends) at 'methods.ozf'

import
   Tk
export
   start:Start
define
   proc{Start Args}
      T={New Tk.toplevel tkInit(title:"Add friends...")}
      T1={New Tk.frame tkInit(parent:T)}
      L={New Tk.label tkInit(parent:T
			     text:"Fill in one or more of the fields given below,\n"#
			     "The information need only to be partial complete!")}
      V1 V2 V3 V4 V5
      Index={NewCell 0}
      GO

      proc{PresentResult H}
	 NotAHit={Append [Args.id] Args.friends}
	 Hits = {Filter H fun {$ X} {List.member X.id NotAHit}==false end}
	 F={New Tk.frame tkInit(parent:T)}
	 F1={New Tk.frame tkInit(parent:F bd:2 relief:sunken)}
	 LB={New Tk.listbox tkInit(parent:F
				   setgrid:true
				   width:40
				   height:15
				   selectmode:extended)}
	 SY = {New Tk.scrollbar tkInit(parent:F width:7)}
	 Ok = {New Tk.button tkInit(parent:F1 text:"Add Selected Friends"
				    action:proc {$}
					      Is={Map {LB tkReturnListInt(curselection $)} fun{$ X} X+1 end}
					      %% Stupied algorithm:)
					      Es={Map Is fun{$ X} {Nth Hits X}.id end}
					   in
					      if Es\=nil then
						 {Args.server S_addFriends(id:Args.id friends:Es)}
					      end
					      {T tkClose}
					   end)}
	 Cancel = {New Tk.button tkInit(parent:F1 text:"Cancel"
					action:proc {$}
						  {T tkClose}
					       end)}
	 LH={Length Hits}
      in
	 {L tk(configure text:"The search gave "#
	       if LH==0 then "no" else LH end#" hit"#
	       if LH\=1 then "s " else " " end#"...")}
	 {Tk.addYScrollbar LB SY}
	 {Tk.batch [grid(LB row:0 column:0 sticky:news columnspan:1)
		    grid(SY row:0 column:1 sticky:ns)
		    grid(Ok row:0 column:0 sticky:we)
		    grid(Cancel row:0 column:1 sticky:we)
		    grid(F1 row:1 column:0 sticky:we)
		    grid(columnconfigure T 0 weight:1)
		    grid(rowconfigure T 1 weight:1)
		    grid(columnconfigure F 0 weight:1)
		    grid(rowconfigure F 0 weight:1)
		    grid(columnconfigure F1 0 weight:1)
		    grid(columnconfigure F1 1 weight:1)
		    grid(rowconfigure F1 0 weight:1)
		    grid(F row:1 column:0 sticky:news)]}
	 
	 {ForAll Hits proc{$ A}
			 {LB tk(insert 'end' "["#A.id#"] "#A.firstname#" "#A.lastname#", email:"#A.email)}
		      end}
      end

      proc{Start2}
	 Hits
	 A=S_searchFriends(id:{V1 tkReturnString($)}
			   email:{V2 tkReturnString($)}
			   firstname:{V3 tkReturnString($)}
			   lastname:{V4 tkReturnString($)}
			   organization:{V5 tkReturnString($)}
			   hits:Hits)
      in
	 {Wait A.id} {Wait A.email} {Wait A.firstname} {Wait A.lastname} {Wait A.organization}
	 if A.id==nil andthen A.email==nil andthen A.firstname==nil
	    andthen A.lastname==nil andthen A.organization==nil then
	    B1={New Tk.button tkInit(parent:T text:"Try Again!"
				     action:proc{$}
					       {T tkClose}
					       {Start Args}
					    end)}
	 in
	    {Tk.send grid(forget(T1))}
	    {L tk(configure text:"\n You can't leave ALL fields empty!! \n")}
	    {Tk.send grid(B1 row:2 column:0 sticky:we)}
	 else
	    {Tk.send grid(forget(T1))}
	    {L tk(configure text:"\n Searching the database, please wait... \n")}
	    {Args.server A}
	    {Wait Hits}
	    {PresentResult Hits}
	 end
      end

      proc{NewEntry Title Value V}
	 O N E L={New Tk.label tkInit(parent:T1 text:Title)}
      in
	 {Exchange Index O N} N=O+1
	 V={New Tk.variable tkInit(Value)}
	 E={New Tk.entry tkInit(parent:T1 width:50 textvariable:V)}
	 {Tk.batch [grid(L row:N column:0 sticky:e)
		    grid(E row:N column:1 sticky:we)]}
	 {E tkBind(event:'<Return>' action:proc{$} GO=unit end)}
	 if N==1 then {Tk.send focus(E)} else skip end
      end
   in
      V1={NewEntry "Login:" ""}
      V2={NewEntry "Email:" ""}
      V3={NewEntry "Firstname:" ""}
      V4={NewEntry "Lastname:" ""}
      V5={NewEntry "Organization:" ""}
      
      {Tk.batch [grid(L row:0 column:0 sticky:we)
		 grid(T1 row:1 column:0 sticky:news)
		 grid(columnconfigure T 0 weight:1)
		 grid(rowconfigure T 1 weight:1)
		 grid(columnconfigure T1 1 weight:1)]}
      {Wait GO}
      {Start2}
   end 
end



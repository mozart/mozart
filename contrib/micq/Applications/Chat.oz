%%%
%%% Authors:
%%%   Lars Rasmusson (lra@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%   Nils Franzen   (nilsf@sics.se)
%%%
%%% Copyright:
%%%   Lars Rasmusson, 1998
%%%   Simon Lindblom, 1998
%%%   Nils Franzen,   1998
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
export
   Start
   Stop
import
   Tk
   Connection(take gate)
   Open(file)
define
   MyLock={NewLock}
   Gate  Port
   proc {Start User Ticket}
      if {Not {IsDet Ticket}} then {StartServer Ticket Gate} end
      {StartClient User Ticket}
   end

   proc {Stop}
      if {IsDet Gate} then
	 {Send Port logout(now)}
	 {Wait 5000}
	 {Gate close}
      end
   end
	
   proc {StartServer Ticket Gate} S C in
      {NewPort S Port}
      C = {NewCell S}
      Gate = {New Connection.gate init(Port#fun {$} {Access C} end Ticket)}
      thread
	 try          % eat strings from the cell
	    {ForAll {List.drop S 40} % remember 40 strings
	     proc {$ _} T in {Exchange C _|T T} end}
	 catch _ then skip end
      end
   end
   
   proc {StartClient User Ticket}
      P#GetS = {Connection.take Ticket}
      S    = {GetS}
      proc {LogOff}
	 Msg = "Walk in peace!\n" in
	 {Send P msg(User.name#" ["#User.id#"] leaves... "#Msg)}
	 {Top tkClose}
      end
      Top  = {New Tk.toplevel tkInit(title:"ChatterbOz" bg:white
				     delete:LogOff)}
      Text = {New Tk.text tkInit(parent:Top bg:white)}
      Edit = {New Tk.entry tkInit(parent:Top bg:white)}
      ScrollBar={New Tk.scrollbar tkInit(parent:Top width:8 orient:vertical)}
      proc {SendStr}
	 S = "["#User.id#"]  "#{Edit tkReturn(get $)}#"\n"
      in
	 {Send P msg(S)}
	 {Edit tk(delete 0 'end')}
      end
      proc {PrintStr Str|S}
	 if {Label Str} == msg then
	    lock MyLock then
	       {Text tk(configure state:normal)}
	       {Text tk(insert 'end' Str.1)}
	       {Text tk(see 'end')}
	       {Text tk(configure state:disabled)}
	    end
	    {PrintStr S}
	 else {Top tkClose} end
      end
      proc{SaveDialog}
	 proc{NotifyUser M}
	    lock MyLock then
	       {Text tk(configure state:normal)}
	       {Text tk(insert 'end' M)}
	       {Text tk(see 'end')}
	       {Text tk(configure state:disabled)}
	    end
	 end
      in
	 case {Tk.return tk_getSaveFile(title:'Save Dialog'
					filetypes:q(q('Text file' q('.txt')) q('All Files' '*')))}
	 of nil then skip
	 elseof S then
	    File
	    Str={Text tkReturn(get p(1 0) 'end' $)}
	 in
	    try
	       File={New Open.file init(name:S flags:[write create truncate])}
	       {File write(vs:Str)}
	       {File close}
	       {NotifyUser "* Saved Dialog in file "#S#"!\n"}
	    catch X then
	       case X of operationCanceled(...) then
		  {NotifyUser "* Save Dialog in file "#S#" Canceled!\n"}
	       else
		  {NotifyUser "* Error occured when trying to save "#S#"!\n"}
	       end
	    end
	 end
      end

      Top1={New Tk.frame tkInit(parent:Top)}
      B1={New Tk.button tkInit(parent:Top1 text:"Leave Chat" action:LogOff)}
      B2={New Tk.button tkInit(parent:Top1 text:"Save Dialog" action:SaveDialog)}
   in
      {Text tk(configure state:disabled)}
      {Tk.addYScrollbar Text ScrollBar}
      {Tk.batch [grid(Text row:5 column:0 sticky:news)
		 grid(Edit row:6 column:0 columnspan:2 sticky:we)
		 grid(ScrollBar row:5 column:1 sticky:ns)
		 grid(Top1 row:3 colum:0 columnspan:2 sticky:we)
		 grid(B1 row:0 colum:0 sticky:we)
		 grid(B2 row:0 colum:1 sticky:we)
		 grid(columnconfigure Top1 0 weight:1)
		 grid(columnconfigure Top1 1 weight:1)
		 grid(columnconfigure Top 0 weight:1)
		 grid(rowconfigure Top 5 weight:1)
		 focus(Edit)
		]}
      {Send P msg(User.name#" ["#User.id#"] enters...\n")}
      {Edit tkBind(event:'<Return>' args:nil action:SendStr)}
      
      thread
	 try
	    {PrintStr S}
	 catch _ then {Top tkClose} end
      end
   end
end





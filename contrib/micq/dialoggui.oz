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
   Tk
export
   view:View
define
   fun {MakeDialogList Msg}
      fun {MakeDialogListR Root Other Ack}
	 if Root == nil then
	    Ack
	 else R|Rs = Root Thread NoThread in
	    {List.partition Other fun{$ X}
				     R.mid == X.reply_to
				  end Thread NoThread}
	    {MakeDialogListR {Append Thread Rs} NoThread {Append Ack [R]}}
	 end
      end
      
      RootMsg OtherMsg
      AllMid = {Map Msg fun {$ X} X.mid end}
      {List.partition Msg fun{$ X}
			     X.reply_to == nil orelse
			     {Member X.reply_to AllMid} == false
			  end RootMsg OtherMsg}
      
   in
      {MakeDialogListR {Sort RootMsg fun{$ X Y} X.lid < Y.lid end}
       {Sort OtherMsg fun{$ X Y} X.lid < Y.lid end} nil}
   end
   
   proc{View E CEntry MessagesIn DB ClearAll}
      T={New Tk.toplevel tkInit(title:"Message dialog with "#E.name)}
      LB={New Tk.listbox tkInit(parent:T
				setgrid:true
				width:40
				height:15
				selectmode:single)}
      SY = {New Tk.scrollbar tkInit(parent:T width:7)}
      {Tk.addYScrollbar LB SY}
      F1={New Tk.frame tkInit(parent:T bd:0)}
      B1={New Tk.button tkInit(parent:F1 text:"Close Dialog Window" action:T#tkClose)}
      B2={New Tk.button tkInit(parent:F1 text:"Clear All Messages" action:proc{$}
									    {T tkClose}
									    {ClearAll}
									 end)}
      
      Ms = {MakeDialogList MessagesIn}

      fun{GetFirstLine X}
	 {List.takeWhile X fun{$ C} C\=&\n end}
      end
   in
      {LB tkBind(event:'<Double-1>'
		 action: proc {$} I={LB tkReturnInt(curselection $)} in
			    {Wait I}
			    if I\=false andthen I\=nil then
			       M={Nth Ms I+1}
			       E={Dictionary.get DB CEntry.id} 
			    in
			       if M.type==received then
				  {CEntry readMessage(entry:E message:M)}
			       elseif M.type==sent then
				  {CEntry writeNewMessage(E.id message:M.message)} 
			       end
			    end
			 end)}
	 
      {Tk.batch [grid(LB row:1 column:0 columnspan:2 sticky:news)
		 grid(SY row:1 column:1 sticky:ns)
		 grid(B2 row:0 column:0 sticky:we)
		 grid(B1 row:0 column:1 sticky:we)
		 grid(F1 row:5 column:0 sticky:we columnspan:2)
		 grid(columnconfigure T 0 weight:1)
		 grid(columnconfigure F1 0 weight:1)
		 grid(columnconfigure F1 1 weight:1)
		 grid(rowconfigure T 1 weight:1)]}
      
      %% This is not a very nice solution.
      
      local
	 proc {FindPrev X Prev Ind Nprev Nind}
	    if Prev==nil then Nprev=[X.mid] Nind=" "
	    else P|Ps=Prev in
	       if X.reply_to==P then
		  Nind={Append ">" Ind}
		  Nprev={Append [X.mid] Prev}
	       else _|Is=Ind in {FindPrev X Ps Is Nprev Nind} end
	    end
	 end
	 proc{InsertMessages M Prev Ind}
	    if M==nil then skip
	    else X|Xs=M Nprev Nind in
	       {FindPrev X Prev Ind Nprev Nind}
	       {LB tk(insert 'end' Nind#" "#
		      X.date.date#" "#X.date.time
		      #" "#X.type#" "#{GetFirstLine X.message}#"...")}
	       {InsertMessages Xs Nprev Nind}
	    end
	 end
      in
	 {InsertMessages Ms nil ""}
      end
   end
end



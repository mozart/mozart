%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1998
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
   Meths(updateFAQ:S_updateFAQ) at 'methods.ozf'
import
   Tk
export
   Start
define
   proc{EditPost X Server}
      T={New Tk.toplevel tkInit(title:"Edit FAQ")}
      L0={New Tk.label tkInit(parent:T text:"Question:")}
      L1={New Tk.label tkInit(parent:T text:"Answer:")}

      TB0={New Tk.text tkInit(parent:T width:50 height:5 bg:white wrap:word fg:red insertbackground:black exportselection:true)}
      SY0={New Tk.scrollbar tkInit(parent:T width:8 orient:vertical)}
      TB1={New Tk.text tkInit(parent:T width:50 height:5 bg:white wrap:word fg:black insertbackground:black exportselection:true)}
      SY1={New Tk.scrollbar tkInit(parent:T width:8 orient:vertical)}

      B0={New Tk.button tkInit(parent:T text:"Submit Change" action:proc{$}
								       Q={TB0 tkReturnString(get p(1 0) 'end' $)}
								       A={TB1 tkReturnString(get p(1 0) 'end' $)}
								       X1=S_updateFAQ(id:X.id
										      data:{Adjoin X.data faq(answer:A question:Q)})
								    in
								       {T tkClose}
								       {Server X1}
								    end)}
      B1={New Tk.button tkInit(parent:T text:"Cancel" action:T#tkClose)}
   in
      {Tk.addYScrollbar TB0 SY0}
      {Tk.addYScrollbar TB1 SY1}
      {TB0 tk(insert p(1 0) X.data.question)}
      {TB1 tk(insert p(1 0) X.data.answer)}
      {Tk.batch [grid(L0 row:0 column:0 columnspan:3 sticky:w)
		 grid(TB0 row:1 column:0 columnspan:2 sticky:we pady:3)
		 grid(SY0 row:1 column:2 sticky:ns pady:3)
		 grid(L1 row:2 column:0 columnspan:3 sticky:w)
		 grid(TB1 row:3 column:0 columnspan:2 sticky:we pady:3)
		 grid(SY1 row:3 column:2 sticky:ns pady:3)

		 grid(B0 row:4 column:0 sticky:we pady:3)
		 grid(B1 row:4 column:1 sticky:we pady:3)
		]}
      skip
   end
   proc{Start Fs1 Server}
      class MyListBox from Tk.listbox
	 prop final
	 attr list:nil
	 meth addItem(X) O N in
	    O=list<-N
	    {Wait O}
	    {LB tk(insert 'end' X.data.poster#" ["#X.id#"] "#{List.take X.data.question 20}#"...")}
	    N={Append O [X]}
	 end
	 meth delItem(id:ID) O N in
	    O=list<-N
	    N={Filter {List.mapInd O fun{$ I X}	if X.id\=ID then X else {LB tk(delete I-1)} nil end end} fun{$ X} X\=nil end}
	 end
	 meth getN(N $) {Nth @list N+1} end
      end
      Fs={Sort Fs1 fun{$ N O} N.id<O.id end}
      
      T={New Tk.toplevel tkInit(title:"Edit FAQ")}
      F0={New Tk.frame tkInit(parent:T)}
      LB={New MyListBox tkInit(parent:F0 width:20 height:7 bg:white fg:black)}
      SY={New Tk.scrollbar tkInit(parent:F0 orient:vertical width:9)}
      {Tk.addYScrollbar LB SY}

      F1={New Tk.frame tkInit(parent:T)}
      B0={New Tk.button tkInit(parent:F1 text:"Edit Selected" action:proc{$} I={LB tkReturnInt(curselection $)} in
									if I\=false then F={LB getN(I $)} in
									   {EditPost F Server}
									else
									   {Tk.send bell}
									end
								     end)}
      B1={New Tk.button tkInit(parent:F1 text:"Delete Selected" action:proc{$} I={LB tkReturnInt(curselection $)} in
									  if I\=false then F={LB getN(I $)} in
									     {LB delItem(id:F.id)}
									     {Server S_updateFAQ(id:F.id data:unit)}
									  else
									     {Tk.send bell}
									  end
								       end)}
      B2={New Tk.button tkInit(parent:F1 text:"Close FAQ Edit" action:T#tkClose)}
   in
      {Tk.batch [grid(LB row:0 column:0 sticky:news)
		 grid(SY row:0 column:1 sticky:ns)
		 grid(B0 row:0 column:0 sticky:we)
		 grid(B1 row:0 column:1 sticky:we)
		 grid(B2 row:1 column:0 sticky:we columnspan:2)

		 grid(F0 row:0 column:0 sticky:news)
		 grid(F1 row:1 column:0 sticky:we)

		 grid(columnconfigure T 0 weight:1)
		 grid(columnconfigure F0 0 weight:1)
		 grid(columnconfigure F1 0 weight:1)
		 grid(columnconfigure F1 1 weight:1)

		 grid(rowconfigure T 0 weight:1)
		 grid(rowconfigure F0 0 weight:1)
		]}
      
      {ForAll Fs proc{$ X} {LB addItem(X)} end}
   end
end


/*
Fs1= [store(data:faq(answer:[79 107 44 32 100 111 32 108 105 107 101 32 116 104 105 115 46 46 46 46 10 10]
		     date:date(date:17#[47]#([48]#3) time:15#[58]#19 year:1999)
		     poster:nilsf
		     question:[72 101 108 108 111 44 32 104 101 108 112 32 109 101 33 10 80 108 101 97 115 101 33 10 10])
	    id:48)
      store(data:faq(answer:[79 107 44 32 100 111 32 108 105 107 101 32 116 104 105 115 46 46 46 46 10 10]
		     date:date(date:17#[47]#([48]#3) time:15#[58]#19 year:1999)
		     poster:nilsf
		     question:[72 101 108 108 111 44 32 104 101 108 112 32 109 101 33 10 80 108 101 97 115 101 33 10 10])
	    id:58)]
[A]={Module.apply [AA]}

{A.start Fs1}

*/

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
%   Browser(browse:Browse)
   DD(dragAndDrop:DragAndDrop) at 'draganddrop.ozf'
%   Pop(popup:Popup) at 'popup.ozf'
   BrowserControl
export
   new:NewMess
   read:ReadMess
define
   class DDEntry from Tk.entry DragAndDrop
      prop final
      attr
	 Variable
	 SendTo:nil
	 
      meth tkInit(textvariable:V ...)=M
	 Tk.label, M
	 DragAndDrop, dragAndDropInit()
	 Variable<-V
      end
      meth setState(X) O N N1 in
	 if {Label X}==user then
	    O=SendTo<-N
	    if {Some O fun{$ Y} Y.id==X.id end}==false then
	       N1={Append [X] O}
	       {self tk(config state:normal)}
	       if O==nil then {@Variable tkSet(X.name)}
	       else {@Variable tkSet({@Variable tkReturnString($)}#", "#X.name)} end
	       {self tk(config state:disabled)}
	       N1=N
	    else N=O end
	 end
      end
      meth getReceivers($) {Map @SendTo fun{$ X} X.id end} end
   end

   proc{Start Arg Type}
%      {Browse Arg#Type}
      T={New Tk.toplevel tkInit(title:"Message ("#if Arg.user.name\=nil then Arg.user.name else
							  Arg.user.id end#")")}
      F1={New Tk.frame tkInit(parent:T)}
      LT={New Tk.label tkInit(parent:F1 text:if Type==new then "To:" else "From:" end)}
      DT={New Tk.label tkInit(parent:F1 text:if Type==new then "<Should not be here!>" else "Date:" end)}
      DV={New Tk.variable tkInit(if Type==new then "<Should not be here!>"
				 else D=Arg.date in D.date#"-"#D.year#", "#D.time end)}
      DE={New Tk.entry tkInit(parent:F1 textvariable:DV state:disabled)}
      VT={New Tk.variable tkInit(if Type==new then ''
				 elseif  Arg.user.name\=nil then Arg.user.name
				 else '['#Arg.user.id#'] unknown' end)}
      ET=if Type==new then 
	    A={New DDEntry tkInit(parent:F1 textvariable:VT state:disabled)}
	 in
	    {A setState(Arg.user)}
	    A
	 else
	    {New Tk.entry tkInit(parent:F1 textvariable:VT state:disabled)}
	 end
      TB={New Tk.text tkInit(parent:T width:50 height:10 bg:white wrap:word fg:black insertbackground:black exportselection:true)}
      SY={New Tk.scrollbar tkInit(parent:T width:8 orient:vertical)}
      B1 B2={New Tk.button tkInit(parent:T text:if Type==new then "Cancel!" else "Close Window!" end
				  action:proc{$} {T tkClose} end)}

      fun{MySplit In}
	 case In of nil then nil
	 elseof &h | &t | &t | &p | &: | &/ | &/ | Ss then Url Rs={List.takeDropWhile Ss Char.isGraph Url} in
	    http(Url)|{MySplit Rs}
	 elseof A|As then
	    A|{MySplit As}
	 end
      end
      
      fun{MyCollect Xs}
	 R Rs={List.takeDropWhile Xs fun{$ X} {IsRecord X}==false end R}
      in
	 case Rs of http(X)|Ss then
	    [R]|http(X)|{MyCollect Ss}
	 elseof nil then
	    [R]
	 end
      end

      fun{FindHttp Xs}
	 {MyCollect {MySplit Xs}}
      end
      TBB
   in
      if Type==new then
	 proc{GO}
	    Mess={TB tkReturnString(get p(1 0) 'end' $)}
	    Mess2=if {IsDet TBB} then {TBB tkReturnString(get p(1 0) 'end' $)} else nil end
	 in
	    {Wait Mess} {T tkClose}
	    {Arg.send {ET getReceivers($)} Mess Mess2}
	 end
      in
	 {TB tkBind(event:'<Alt-Return>' action:GO)}
	 B1={New Tk.button tkInit(parent:T text:"Send Message!" action:GO)}
      else
	 B1={New Tk.button tkInit(parent:T text:"Reply Message!"
				  action:proc{$}
					    Mess={Map Arg.message fun{$ C}
								     if C==&\n then "\n>" else C end
								  end}
					 in
					    {T tkClose} {Arg.send ">"#{Flatten Mess}#"\n\n" false}
					 end)}
      end

      {Tk.addYScrollbar TB SY}

      if {CondSelect Arg faq false}\=false then
	 L0={New Tk.label tkInit(parent:T text:"Question:")}
	 L1={New Tk.label tkInit(parent:T text:"Answer:")}
	 TB1={New Tk.text tkInit(parent:T width:50 height:5 bg:white wrap:word fg:red insertbackground:black exportselection:true)}
	 SY1={New Tk.scrollbar tkInit(parent:T width:8 orient:vertical)}
      in
	 {Tk.addYScrollbar TB1 SY1}
	 {TB1 tk(insert p(1 0) Arg.message)}
%	 {TB1 tk(config state:disabled)}
	 {Tk.batch [grid(L0 row:2 column:0 columnspan:3 sticky:w)
		    grid(TB1 row:3 column:0 columnspan:2 sticky:we pady:3)
		    grid(SY1 row:3 column:2 sticky:ns pady:3)
		    grid(L1 row:4 column:0 columnspan:3 sticky:w)]} 
	 TBB=TB1
      end
      
      if Type==read then
	 if Arg.su==true then
	    FAQB={New Tk.button tkInit(parent:T text:"Reply to FAQ" action:proc{$} {T tkClose} {Arg.send Arg.message true} end)}
	 in
	    {Tk.send grid(FAQB row:30 column:0 columnspan:2 sticky:we)}
	 end
	 {Tk.batch [grid(DT row:1 column:0 sticky:e)
		    grid(DE row:1 column:1 sticky:we)]}
      end
      
      {Tk.batch [grid(LT row:0 column:0 sticky:e)
		 grid(ET row:0 column:1 sticky:we)
		 grid(F1 row:0 column:0 columnspan:3 sticky:we)
		 grid(TB row:5 column:0 sticky:news columnspan:2 pady:3)
		 grid(SY row:5 column:2 sticky:ns pady:3)
		 grid(B1 row:10 column:0 sticky:we)
		 grid(B2 row:10 column:1 sticky:we)
		 grid(columnconfigure T 0 weight:1)
		 grid(columnconfigure T 1 weight:1)
		 grid(columnconfigure F1 1 weight:1)
		 grid(rowconfigure T 5 weight:1)
		 focus(TB)]}

      if {CondSelect Arg faq false}==false then
	 if {CondSelect Arg browser false}==true then
	    {ForAll {FindHttp {VirtualString.toString Arg.message}}
	     proc{$ X}
		if {IsRecord X} andthen {Label X}==http then Tag FI LI in
		   Tag={New Tk.textTag tkInit(parent:TB)}
		   FI={TB tkReturn(index insert $)}
		   {TB tk(insert 'end' "http://"#X.1)}
		   LI={TB tkReturn(index insert $)}
		   {TB tk(tag add Tag FI LI)}
		   {TB tk(tag config Tag underline:true foreground:blue)}
		   {Tag tkBind(event:'<1>' action:proc{$} {BrowserControl.displayUrl "http://"#X.1} end)}
		elseif {IsInt X} then
		   {TB tk(insert 'end' {Char.toAtom X})}
		else
		   {TB tk(insert 'end' {Flatten X})}
		end
	     end}
	 else
	    {TB tk(insert p(1 0) Arg.message)}
	 end
      end
      {TB tk(config state:if Type==new then normal else disabled end)}
      if Type==new andthen Arg.message\=nil then {TB tk(see 'end')} end
   end
   proc{NewMess X} {Start X new} end
   proc{ReadMess X} {Start X read} end
end

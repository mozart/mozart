%%%
%%% Authors:
%%%   Nils Franz�n (nilsf@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%
%%% Copyright:
%%%   Nils Franz�n, 1998
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
   DD(dragAndDrop:DragAndDrop) at 'draganddrop.ozf'
%   Pop(popup:Popup) at 'popup.ozf'
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
      TB={New Tk.text tkInit(parent:T width:50 height:10)}
      SY={New Tk.scrollbar tkInit(parent:T width:8 orient:vertical)}
      B1 B2={New Tk.button tkInit(parent:T text:if Type==new then "Cancel!" else "Close Window!" end
                                  action:proc{$} {T tkClose} end bd:1 relief:groove)}
   in
      if Type==new then
         proc{GO} Mess={TB tkReturnString(get p(1 0) 'end' $)} in
            {Wait Mess} {T tkClose} {Arg.send {ET getReceivers($)} Mess}
         end
      in
         {TB tkBind(event:'<Alt-Return>'
                    action:GO)}
         B1={New Tk.button tkInit(parent:T text:"Send Message!" bd:1 relief:groove
                                  action:GO)}
      else
         B1={New Tk.button tkInit(parent:T text:"Reply Message!" bd:1 relief:groove
                                  action:proc{$}
                                            Mess={Map Arg.message fun{$ C}
                                                                     if C==&\n then "\n>" else C end
                                                                  end}
                                         in
                                            {T tkClose} {Arg.send ">"#{Flatten Mess}#"\n\n"}
                                         end)}
      end

      {Tk.addYScrollbar TB SY}

      if Type==read then
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
      {TB tk(insert p(1 0) Arg.message)}
      {TB tk(config state:if Type==new then normal else disabled end)}
      if Type==new andthen Arg.message\=nil then {TB tk(see 'end')} end
   end
   proc{NewMess X} {Start X new} end
   proc{ReadMess X} {Start X read} end
end

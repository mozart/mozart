%%%
%%% Authors:
%%%   Lars Rasmusson (lra@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%
%%% Copyright:
%%%   Lars Rasmusson, 1998
%%%   Simon Lindblom, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
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

define

   Gate
   Port
   proc {Start User Ticket}
      if {Not {IsDet Ticket}} then
         {StartServer Ticket Gate}
      end
      {StartClient User Ticket}
   end

   proc {Stop}
      if {IsDet Gate} then
         {Send Port logout(now)}
         {Wait 4000}
         {Gate close}
      end
   end

   proc {StartServer Ticket Gate}
      S C in
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
            {Text tk(configure state:normal)}
            {Text tk(insert 'end' Str.1)}
            {Text tk(see 'end')}
            {Text tk(configure state:disabled)}
            {PrintStr S}
         else {Top tkClose} end
      end
   in
      {Text tk(configure state:disabled)}
      {Tk.addYScrollbar Text ScrollBar}
      {Tk.batch [grid(Text row:0 column:0)
                 grid(Edit row:1 column:0 columnspan:2 sticky:ew)
                 grid(ScrollBar row:0 column:1 sticky:ns)]}
      {Send P msg(User.name#" ["#User.id#"] enters...\n")}
      {Edit tkBind(event:'<Return>' args:nil action:SendStr)}

      thread
         try
            {PrintStr S}
         catch _ then {Top tkClose} end
      end
   end
end

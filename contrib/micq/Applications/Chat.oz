%%%
%%% Authors:
%%%   Lars Rasmusson (lra@sics.se)
%%%
%%% Copyright:
%%%   Lars Rasmusson, 1998
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
   proc {Start User Ticket}
      if {Not {IsDet Ticket}} then
         {StartServer Ticket Gate}
      end
      {StartClient User Ticket}
   end

   proc {StartServer Ticket Gate}
      P S C in
      {NewPort S P}
      C = {NewCell S}
      Gate = {New Connection.gate init(P#fun {$} {Access C} end Ticket)}
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
         {Send P User.name#" ["#User.id#
          "] leaves... "#Msg#"\n"}
         {Top tkClose}
      end
      Top  = {New Tk.toplevel tkInit(title:"ChatterbOz" bg:white
                                    delete:LogOff)}
      Text = {New Tk.text tkInit(parent:Top bg:white)}
      Edit = {New Tk.entry tkInit(parent:Top bg:white)}
      proc {SendStr}
         S = "["#User.id#"]  "#{Edit tkReturn(get $)}#"\n"
      in
         {Send P S}
         {Edit tk(delete 0 'end')}
      end
      proc {PrintStr Str|S}
         {Text tk(configure state:normal)}
         {Text tk(insert 'end' Str)}
         {Text tk(see 'end')}
         {Text tk(configure state:disabled)}
         {PrintStr S}
      end
   in
      {Text tk(configure state:disabled)}
      {Tk.send pack(Text expand:y fill:y)}
      {Tk.send pack(Edit fill:x)}
      {Send P User.name#" ["#User.id#"] enters...\n"}
      {Edit tkBind(event:'<Return>' args:nil action:SendStr)}
      thread
         try
            {PrintStr S}
         catch _ then skip end
      end
   end

   proc {Stop}
      if {IsDet Gate} then {Gate close} end
   end
end

%%%
%%% Authors:
%%%   Denys Duchier <duchier@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Denys Duchier, 1998
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
   Application(exit) Connection(gate) Pickle(save) Tk
define
   NewsPort Ticket
   {New Connection.gate init(NewsPort Ticket) _}
   File
   local
      Top = {New Tk.toplevel tkInit}
      L1  = {New Tk.label tkInit(parent:Top text:'Save ticket in:')}
      E1  = {New Tk.entry tkInit(parent:Top width:40)}
      {E1 tkBind(event:'<KeyPress-Return>'
		 action:proc{$}{E1 tkReturn(get File)}end)}
      B1  = {New Tk.button tkInit(parent:Top text:'Abort'
				  action:proc{$}
					    {Application.exit 0}
					 end)}
      {Tk.send pack(L1 E1 B1 side:left)}
   in
      {Wait File}
      try {Top tkClose} catch _ then skip end
   end
   {Pickle.save Ticket File}
   local
      Top = {New Tk.toplevel tkInit(title:'Chat Server')}
      B1  = {New Tk.button tkInit(parent:Top
				  text:'Shutdown Chat Server'
				  action:proc{$}
					    {Application.exit 0}
					 end)}
   in
      {Tk.send pack(B1)}
   end
   {List.forAllTail {Port.new $ NewsPort}
    proc {$ H|T}
       case H of connect(Messages) then Messages=T else skip end
    end}
end

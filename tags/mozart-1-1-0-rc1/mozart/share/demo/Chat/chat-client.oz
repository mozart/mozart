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
   Pickle(load) Connection(take)
   Viewer(chatWindow) at 'chat-gui.ozf'
   OS(getEnv) Application(exit) Tk
define
   Url User Host
   local
      Top = {New Tk.toplevel tkInit}
      L1  = {New Tk.label tkInit(parent:Top text:'Chat server url:')}
      L2  = {New Tk.label tkInit(parent:Top text:'User name:')}
      L3  = {New Tk.label tkInit(parent:Top text:'Host name:')}
      E1  = {New Tk.entry tkInit(parent:Top width:40)}
      E2  = {New Tk.entry tkInit(parent:Top width:40)}
      E3  = {New Tk.entry tkInit(parent:Top width:40)}
      USER = case {OS.getEnv 'USER'} of false then 'unknown'
	     elseof X then X end
      HOST = case {OS.getEnv 'HOSTNAME'} of false then
		case {OS.getEnv 'HOST'} of false then 'unknown'
		elseof X then X end
	     elseof X then X end
      B1 = {New Tk.button tkInit(parent:Top text:'Accept'
				 action:
				    proc{$}
				       {E1 tkReturn(get Url)}
				       {E2 tkReturn(get User)}
				       {E3 tkReturn(get Host)}
				       try {Top tkClose}
				       catch _ then skip end
				    end)}
      B2 = {New Tk.button tkInit(parent:Top text:'Abort'
				 action:
				    proc{$}
				       {Application.exit 0}
				    end)}
   in
      {E2 tk(insert 0 USER)}
      {E3 tk(insert 0 HOST)}
      {Tk.batch [grid(L1 column:0 row:0 sticky:nw)
		 grid(L2 column:0 row:1 sticky:nw)
		 grid(L3 column:0 row:2 sticky:nw)
		 grid(E1 column:1 row:0 sticky:new)
		 grid(E2 column:1 row:1 sticky:new)
		 grid(E3 column:1 row:2 sticky:new)
		 grid(B1 column:0 row:3 sticky:nw)
		 grid(B2 column:1 row:3 sticky:ne)]}
   end
   {Wait Url}
   {Wait User}
   {Wait Host}
   NAME = User#'@'#Host
   NewsPort={Connection.take {Pickle.load Url}}
   SelfPort
   thread
      {ForAll {Port.send NewsPort connect($)}
       proc {$ Msg} {Port.send SelfPort Msg} end}
   end
   Chat = {New Viewer.chatWindow init(SelfPort)}
   {ForAll {Port.new $ SelfPort}
    proc {$ Msg}
       case Msg of msg(FROM TEXT) then
          {Chat show(FROM#':\t'#TEXT)}
       elseof say(TEXT) then
          {Port.send NewsPort msg(NAME {ByteString.make TEXT})}
       else skip end
    end}
end

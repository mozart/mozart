%%%
%%% Authors:
%%%   Erik Klintskog <erik@sic.se>
%%%
%%% Contributors:
%%%   Anna Neiderud <annan@sics.se>
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
   DPStatistics(perdioStatistics)
export
   MessageDiffInfoClass
define
   class MessageDiffInfoClass
      feat
	 messageReceive
	 messageSend
	 diffReceive
	 diffSend
      meth init(GUI)
	 self.messageReceive = GUI.messageReceived
	 self.messageSend = GUI.messageSent
	 self.diffReceive = GUI.diffReceived
	 self.diffSend = GUI.diffSent
      end

      meth display
	 S={DPStatistics.perdioStatistics}
      in
	 {self.messageReceive update(S.recv.messages)}
	 {self.messageSend update(S.send.messages)}
	 {self.diffReceive update(S.recv.dif)}
	 {self.diffSend update(S.send.dif)}
	 
      end
   end
end

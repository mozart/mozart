functor
import
   DPPane(perdioStatistics) at 'x-oz://boot/DPPane'
export
   MessageInfoClass
define
   class MessageInfoClass
      feat
	 receive
	 send 
      meth init(GUI)
	 self.receive = GUI.messageReceived
	 self.send = GUI.messageSent
      end
      meth display
	 S={DPPane.perdioStatistics}
      in
	 {self.receive update(S.recv.messages)}
	 {self.send update(S.send.messages)}
      end
   end
end
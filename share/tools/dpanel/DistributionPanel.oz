functor
import
   Main(server:Mserver open:Mopen openNetInfo:MnetInfo)  at 'x-oz://system/DistributionPanelSrc.ozf'
   DPPane(siteStatistics) at 'x-oz://boot/DPPane'
   Connection
export
   Open
   OpenNetInfo
   Client
   Server
define

      class DPclient
      prop locking
      attr
	 state:starting
	 
      feat
	 serverPort
	 site
	 
      meth init(Tick)
	 S P = {NewPort S}
	 Site = {Filter {DPPane.siteStatistics} fun{$ M} M.state == mine end}.1
      in
	 self.site = site(ip:Site.ip port:Site.port pid:Site.pid)
	 self.serverPort = {Connection.take Tick}
	 {Send self.serverPort connecting(P self.site Site.siteid)}
	 thread {ForAll S self} end
      end

      meth connected
	 state <- waiting
      end

      meth start(Time)
	 %% skiping accumulated data 
	 _ = {DPPane.siteStatistics}
	 state <- running(Time)
	 {self update(Time)}
      end
	  
      meth update(Time)
	 if @state == running(Time) then
	    try 
	       {Send self.serverPort data({DPPane.siteStatistics} self.site)}
	       if Time == 0 then skip
	       else {Delay Time} end
	    catch _ then
	       state <- crashed 
	    end
	    {self update(Time)}
	 end
      end
      
      meth stop
	 state <- waiting
      end
   end
   
   proc{Client Tick}
      _ = {New DPclient init(Tick)}
   end
   
   
   Open=Mopen
   OpenNetInfo=MnetInfo
   Server=Mserver
end







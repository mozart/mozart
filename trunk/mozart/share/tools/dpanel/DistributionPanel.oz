functor
import
   Main(server:Mserver open:Mopen openNetInfo:MnetInfo)  at 'x-oz://system/DistributionPanelSrc.ozf'
   DPStatistics(siteStatistics) at 'x-oz://boot/DPStatistics'
   Error
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
	 
      meth init(ServerPort)
	 S P = {NewPort S}
	 Site = {Filter {DPStatistics.siteStatistics}
		 fun{$ M} M.state == mine end}.1
      in
	 self.site = site(ip:Site.ip port:Site.port pid:Site.pid)
	 self.serverPort = ServerPort
	 {Send self.serverPort connecting(P self.site Site.siteid)}
	 thread
	    %% If the dpClient should fail
	    %% print the exception to stdout
	    try
	       {ForAll S self}
	    catch X then
	       {Error.printException  X}
	    end
	 end
      end

      meth connected
	 state <- waiting
      end

      meth start(Time)
	 %% skiping accumulated data 
	 _ = {DPStatistics.siteStatistics}
	 state <- running(Time)
	 {self update(Time)}
      end
	  
      meth update(Time)
	 if @state == running(Time) then
	    try 
	       {Send self.serverPort data({DPStatistics.siteStatistics}
					  self.site)}
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
   
   proc{Client ServerPort}
      _ = {New DPclient init(ServerPort)}
   end
   
   
   Open=Mopen
   OpenNetInfo=MnetInfo
   Server=Mserver
end







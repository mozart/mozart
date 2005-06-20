%%%
%%% Authors:
%%%   Erik Klintskog (erik@sics.se)
%%%   Anna Neiderud (annan@sics.se)
%%%
%%% Copyright:
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

\define DBG
functor
export
   Connectionfunctor
define
   functor Connectionfunctor
   export
      Connect
      ConnectWithRoute
   import
      ConnectionWrapper at 'x-oz://connection/ConnectionWrapper.ozf'
\ifdef DBGroute
      System
\endif
\ifdef TSTroute 
      System
      Property
\endif
\ifdef DBG
      System(show:Show showInfo showError)
      Property
\endif
   define
      proc{Dshow M}
\ifdef DBG
	 {Show M}
\endif
	 skip
      end
      
      RetryTimes=10
      RetryWaitTime=100

      ConnId
      
      proc {Parse P ?Address ?IPPort}
      % Get Address and Port to connect to out of P
	 ip_addr(addr:Address port:IPPort)=P
      end

      %%bmc: Method GetNegChannel it was previously local to Connect
      proc{GetNegChannel Address Port Time FD}
	 if Time>0 then
	    try
	       FD={ConnectionWrapper.socket 'PF_INET' 'SOCK_STREAM' "tcp"}
	       if FD==~1 then raise no_fd end end
	       {ConnectionWrapper.connect FD Address Port}
	    catch X then
	       {Dshow X}
	       case X of system(os(_ _ _ "Connection refused") ...) then
		  raise perm end
	       elseof system(os(_ _ _ "In progress") ...) then
		  {Dshow dont_worry}
	       elseof system(kernel(terminate) ...) then
		  raise terminated end
	       elseof _ then 
		  if FD\=~1 then {ConnectionWrapper.close FD} end
		  {Dshow caught(X)}
		  {Delay RetryWaitTime}
		  {GetNegChannel Address Port Time-1 FD}
	       end
	    end
	 else
	    raise failed_to_connect end
	 end
      end

      proc{Connect P}       
	 FD Address Port ReadS
      in
	 try
	    {Parse P ?Address ?Port}
	    {Dshow connecting(Address Port)}
	    {GetNegChannel Address Port RetryTimes FD}
	    {Dshow connected(FD)}
	    try
	       {ConnectionWrapper.writeSelect FD}
	       _={ConnectionWrapper.write FD "tcp"}
	       % Since connection establishment is assynchronous, 
	       % the fault codes can appear at the first write. 
	    catch X then
	       case X of  system(os(_ _ _ "Broken pipe") ...) then
		  raise perm end
	       elseof system(os(_ _ _ "Connection refused") ...) then
		  raise perm end
	       elseof X then
		  raise X end
	       end
	    end
	    
	    {ConnectionWrapper.readSelect FD}
	    _ = {ConnectionWrapper.read FD 2 ReadS nil}
	    if ReadS == "ok" then 
	       {ConnectionWrapper.handover settings(fd:FD)}
	    else
	       % Here we should choose another protocol
	       raise choose_protocol end
	    end
	
	    % If we catch an exception here (other than perm, se above)
	    % it means the connection
	    % was somehow corrupted. Report this as a temp error and let
	    % the requestor try again.
	 catch perm then
	    {ConnectionWrapper.close FD}
	    {ConnectionWrapper.connFailed perm}
	 [] system(kernel(terminate) ...) then
	    {ConnectionWrapper.close FD}
	    raise terminated end
	 [] abort then
	    raise terminated end 
	 [] failed_to_connect then
	    %{Delay 1000}
	    skip
	    {Dshow could_not_connect}
	    raise failed_to_connect end
	 [] E then 
	    {ConnectionWrapper.close FD}
	    {Dshow tcp_did_not_work}
	    %{Delay 1000}
	    raise E end 
	 end
      end

      %% Connect to TargetSite by discovering a route to it.
      proc {ConnectWithRoute CId ThisSite TargetSite NghbLst
	    SendMsgToSite RcvProc}
	 fun {GetRoute Strm}
	    case Strm of M|Mr then
	       case M of route_found(r:R) then
\ifdef DBGroute
		  {System.showInfo 'con:: route discovered'}
\endif		  
		  R
	       else
		  {GetRoute Mr}
	       end
	    end
	 end

	 MsgPort
	 Route  %% a list of sites
\ifdef TSTroute 
	 T0 = {Record.filter {Property.get time} IsInt}
\endif
      in
	 %%! we might have to use P here /V

	 ConnId = CId
	 RcvProc = proc {$ M} {Send MsgPort M} end
	 
	 for S in NghbLst do
\ifdef DBGroute	    
	    {System.showInfo 'con:: send discover_route'}
\endif	    
	    {SendMsgToSite S m(mt:prot pt:flood ttl:5
			       src:ThisSite dst:nil via:ThisSite cid:ConnId
			       pl:discover_route(trg:TargetSite))  nil}
	 end
	 
	 {GetRoute {NewPort $ MsgPort} Route}
\ifdef TSTroute 
	 {System.show connectRoute#
	  {Record.zip {Record.filter {Property.get time} IsInt} T0 Number.'-'}}
\endif
	 
	 {ConnectionWrapper.handoverRoute {Reverse Route} TargetSite}	 
      end
      
   end
end

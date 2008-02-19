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
import
   Glue at 'x-oz://boot/Glue'
   Module
   System
%   Browser
   Pickle
   OS
export
   GetAppState
   PutAppState
   CondGetAppState
   InitConnection
%   InitAccept
   LastSite
   GetAllSites
   GetConSites
   GetThisSite
   GetSiteInfo
   GetChannelStatus
   SendMsgToSite
define
   AppState = {NewDictionary}
   GetAppState
   PutAppState
   CondGetAppState
   InitConnection
%   InitAccept
   
   ThisSite = {Glue.getThisSite}
   
   LastSiteCell = {NewCell none}
   LastSite
   GetAllSites
   GetConSites
   GetThisSite
   GetSiteInfo
   GetChannelStatus
   SendMsgToSite
   SendMsgToSitePlus
   ProcessMsg
   NewMsgId
   
   ConnId = {NewCell 0}
   MsgTrailer = {NewCell 0}
   InMsgIdSoftState = {NewDictionary} %% msgId: msgType+input_interface
   OutMsgIdSoftState = {NewDictionary} %% msgId: list_of_output_interfaces
   RcvProcs = {NewDictionary}  %% connId: rcvProc

   fun {NewConnId} T in
      {Exchange ConnId T thread T+1 end}
      T+1
   end

   fun {NewMsgTrailer} T in
      {Exchange MsgTrailer T thread T+1 end}
      T+1
   end 
   
   %% Gets a unique id out of the Requsteor structure
   fun{GetIdFromRequestor Requestor}
      Requestor.id
   end

   fun {SiteKey Site}
      site(ip:IP port:PN id:ID) = Site.info
   in
      {VirtualString.toAtom IP#':'#PN#':'#ID}
   end

   %% Check the functor, it is only allowed to importthe ConnectionWrapper
   fun{CheckFunctor Func} 
      true
   end
   fun{GetConnectionWrapper Obj TargetSite}
      {Module.apply
       [functor 
	export
	   Handover
	   HandoverRoute
	   ConnFailed
	   GetLocalState
	   PutLocalState
	   GetAppState
	   CondGetAppState
	   PutAppState
	   Socket
	   Connect
	   Write
	   Read
	   WriteSelect
	   ReadSelect
	   Close
	define
	   Key={SiteKey TargetSite}

	   proc{Handover SetUpParameter}
	      % This is a fix and not a solution. If another
	      % ConnectionFunctor than the default one keeps a
	      % filedescriptor via handover, it must use the same
	      % syntax in SetUpParameter.
	      case SetUpParameter of settings(fd:FD) then
		 {Obj unregisterResource(fd(FD))}
	      else skip end
		 
	      if {Dictionary.member OngoingRequests Key} then
		 {Glue.setConnection TargetSite SetUpParameter.fd}
	      end
	   end
      
	   proc {HandoverRoute IntermediaryRouter PeerSite}
	      {Glue.handoverRoute IntermediaryRouter PeerSite}
	   end
      
	   proc{ConnFailed Reason}
	      {Obj freeResources}
	      if {Dictionary.member OngoingRequests Key} then
		 {Glue.connFailed TargetSite Reason}
	      end
	   end

	   % Could be a consistency problem if abort happens... AN!
	   % Add a Dictionary.member test?
	   proc{GetLocalState State}
	      {Obj getLocalState(State)}
	   end
	   proc{PutLocalState State}
	      {Obj putLocalState(State)}
	   end

	   proc{Socket A B C ?FD}
	      FD={OS.socket A B C}
	      if FD\=~1 then
		 {Obj registerResource(fd(FD))}
	      end
	   end

	   proc{Close FD}
	      {OS.close FD}
	      {Obj unregisterResource(fd(FD))}
	   end
	   
	   Connect=OS.connect%Nonblocking
	   Write=OS.write
	   Read=OS.read
	   WriteSelect=OS.writeSelect
	   ReadSelect=OS.readSelect
	end]}.1
   end
    

   class ConnectionController
      prop
	 locking
      feat
	 %id
	 %requestor
	 moduleManager
      attr
	 allocatedResources:nil
	 localState 
      meth init(ToRoute TargetSite ConnectionFunctor)
	 %ConnectionFunctor
	 ConnectModule
      in
	 try
	    %self.id=Id
	    %self.requestor = Requestor
	    %case DistOzState.type of
	    %ordinary then
	    %ConnectionFunctor = LocalOzState.connectionFunctor
	    %elseof dynamic then
	    %   ConnectionFunctor = {Pickle.load  DistOzState.location}
	    %elseof replicated then
	    %   raise notImplementedYet end
	       %% Do a lot of loading and shit ...
	    %end
	    %if {Not {CheckFunctor ConnectionFunctor}} then
	    %   raise malishiousFunctor end
	    %end
	    self.moduleManager ={New Module.manager init}
	    {System.show cc_after_mmm}
	    {self.moduleManager
	     enter(url:'x-oz://connection/ConnectionWrapper.ozf'
		   {GetConnectionWrapper self TargetSite})}
	    {System.show cc_after_enter(ConnectionFunctor)}
	    {self.moduleManager apply(ConnectionFunctor ConnectModule)} 
	    %localState <- LocalOzState.localState  
	    {System.show cc_after_modules}
	    local
	       NConId = {NewConnId}
	       RProc
	       CWR = proc {$}
\ifdef DBGroute 
			{System.showInfo 'connect by routing'}
\endif
			{ConnectModule.connectWithRoute 
			 NConId
			 ThisSite
			 TargetSite
			 {GetConSites}
			 SendMsgToSitePlus
			 RProc}
		     end
	    in
	       {Dictionary.put RcvProcs NConId RProc}
	       
	       %% temporary use the route intention here ... /V
	       %if ToRoute andthen {Length {GetConSites}} > 0 then
	       %{CWR}
	       %{Dictionary.remove RcvProcs NConId}
	       %else %% connect directly
\ifdef DBGroute 
\endif
		  {System.showInfo 'connect directly'}
		  try
		     {ConnectModule.connect TargetSite}
		  catch failed_to_connect then
		     %{CWR}
		     skip
		  [] abort then
		     raise abort end
		  [] terminated then
		     raise terminated end
		  [] _ then
		     %{CWR}
		     skip
		  finally
		     {Dictionary.remove RcvProcs NConId}
		  end
	       %end	       
	    end
\ifdef DBG
	 catch X then 
	    {System.show warning(X)}
%	    thread raise X end end % Use only in OPI
\else
	 catch _ then
	    skip
\endif
	 end
	 {self freeResources}
      end
      
      meth getLocalState($) @localState end
      meth putLocalState(S)
	 localState <- S
	 {Glue.putLocalState self.requestor S}
      end
      meth registerResource(R)
	 lock
	    allocatedResources<-{Append @allocatedResources [R]}
	 end
      end
      meth unregisterResource(R)
	 lock
	    allocatedResources<-{Filter @allocatedResources
				  fun{$ RC} RC\=R end}
	 end
      end
      meth freeResources
	 lock
	    {ForAll @allocatedResources
	     proc{$ R}
		case R of fd(FD) then
		   {OS.deSelect FD}
		   {OS.close FD}
		end
	     end}
	    allocatedResources<-nil
	 end
      end
   end
   OngoingRequests
in
   proc{InitConnection Stream ToRoute ConFun}
\ifdef DBG
      PID={OS.getPID}
   in
\endif
      OngoingRequests = {NewDictionary}
%      {Browser.browse Stream}
      {System.show going_to_start_the_forAllInd}
      {List.forAllInd Stream
       proc{$ Ind Request}
\ifdef DBG
	  {System.show waiting(pid:PID ind:Ind)}
	  {Wait Request}
	  {System.show got(Request PID Ind)}
\endif
	  case Request of
	     connect(TargetSite) then
	     {System.show {Glue.getSiteInfo TargetSite}}
	     %Id = {GetIdFromRequestor Requestor}
	     Key={SiteKey TargetSite}
	  in
	     if {Dictionary.member OngoingRequests Key} then
		{System.show dropped}
		skip
%		thread raise already_connecting(Id) end end
	     else
		OngoingRequests.Key:=r(thr:_)
		{System.show will_connect}
		thread
		   try
		      case OngoingRequests.Key of r(thr:T) then
			 T={Thread.this}
		      end
		      {System.show before_cc_init}
		      _ = {New ConnectionController init(ToRoute
							 TargetSite ConFun)}
		      {System.show after_cc_init}
\ifdef DBG
		   catch X then
		      {System.show warning(X)}
%	    thread raise X end end % Use only in OPI
\else
		   catch _ then
		      skip
\endif
		   end
		end
	     end
	     
	  elseof connection_received(TargetSite FD) then
	     {Dictionary.remove OngoingRequests {SiteKey TargetSite}}
	     
	  elseof new_site(S) then
	     {Assign LastSiteCell S}
% 	  elseof abort(Requestor) then
% 	     Id = 'a'%{GetIdFromRequestor Requestor}
% 	  in
% 	     try
% 		case {CondSelect OngoingRequests Id notfound}
% 		of r(thr:T) then
% 		   {Thread.terminate T}
% 		   {Dictionary.remove OngoingRequests Id}
% 		else
% 		   skip
% 		end
% 	     catch _ then skip end
	     
	  elseof deliver(src:S msg:Msg) then
	     {ProcessMsg S Msg}
	     
	  else 
	     {System.showError "Warning Connection Wrapper called with wrong parameters"}
%	     {System.showError {Value.toVirtualString Request 100 100}}
	     raise error end
	  end
       end}
   end


   proc{GetAppState Key Val}
      Val = AppState.Key
   end

   proc{PutAppState Key Val}
      AppState.Key:=Val
   end

   proc{CondGetAppState Key AltVal Val}
      {Dictionary.condGet AppState Key AltVal Val}
   end

%    proc{InitAccept AcceptFunc}
%       {AcceptProc.accept}
%    end

   fun{LastSite}
      {Access LastSiteCell}
   end

   fun {GetAllSites}
      {Glue.getAllSites}
   end

   fun {GetConSites}
      {Glue.getConSites}
   end

   fun {GetThisSite}
      ThisSite
   end

   fun {GetChannelStatus S}
      {Glue.getChannelStatus S}
   end
   
   fun {GetSiteInfo S}
      {Glue.getSiteInfo S}
   end

   %% Send Msg to site S
   proc {SendMsgToSite S Msg}
      {Glue.sendMsgToSite S Msg}
   end

   fun {NewMsgId}
      {VirtualString.toAtom {OS.getPID}#{OS.time}#{NewMsgTrailer}}
   end

   %% Like SendMsgToSite but it keeps a soft state of the msg with id MId
   proc {SendMsgToSitePlus S Msg MId}
      TMId LS
   in
      if MId \= nil then TMId = MId
      else TMId = {NewMsgId} end
      
      LS = {Dictionary.condGet OutMsgIdSoftState TMId nil}
      
      if LS \= nil then
	 {Dictionary.put OutMsgIdSoftState TMId S|LS}
      else
	 {Dictionary.put OutMsgIdSoftState TMId [S]}
      end

      thread
	 {Delay 15000}
	 %% remove the soft state out
	 {Dictionary.remove OutMsgIdSoftState TMId} 
      end

%\ifdef DBGroute        
%      {System.show send_msg_id#TMId}
%\endif      
      {SendMsgToSite S {Adjoin m(mid:TMId) Msg}}
   end

   %% process the msgs received from other sites
   proc {ProcessMsg SrcSite Msg}
      % given a msg patern and a list of msgs get the first corresponding site
      fun {GetInterface MsgP LM}
	 case LM of X|Xr then
	    case X of MsgP(E) then E
	    else {GetInterface MsgP Xr} end
	 [] nil then nil end
      end

      fun {AlreadyForwarded MId O}
	 LS = {Dictionary.condGet OutMsgIdSoftState MId nil}
      in
	 {List.sub [O] LS}
      end
   in
\ifdef DBGroute      
      {System.show {GetSiteInfo SrcSite}.siteId#Msg}
\endif      
      case Msg of
	 m(mt:Mt pt:Pt ttl:Ttl src:Src dst:Dst via:Via
	   mid:MId cid:CId pl:Pl ...) then

	 if Ttl > 0 then
	    case Pl of discover_route(trg:Trg) then
	       ConSites = {GetConSites}
	    in
\ifdef DBGroute	       
		  {System.showInfo 'con:: receive discover_route, msgid:'#MId}
\endif
	       
	       if {Member Trg ConSites} then
\ifdef DBGroute	       
		  {System.showInfo 'con:: target '#{GetSiteInfo Trg}.siteId#
		   ' found, msgid:'#MId}
\endif
		  {SendMsgToSitePlus Via
		   m(mt:Mt pt:Pt ttl:Ttl src:ThisSite dst:Src via:ThisSite
		     cid:CId pl:route_found(r:[ThisSite Trg])) MId}
	       else
		  NghbLst = {Filter ConSites fun {$ E} E\=Via   end}
	       in
		  for S in NghbLst do
		     %% do not forward twice a msg to the same site
		     if {Not {AlreadyForwarded MId S}} then
\ifdef DBGroute		     
			{System.showInfo 'con:: forward discover to: '#
			 {GetSiteInfo S}.siteId#', msgid:'#MId}
\endif		     
			{SendMsgToSitePlus S
			 m(mt:Mt pt:Pt ttl:Ttl-1 src:Src dst:nil
			   via:ThisSite cid:CId pl:Pl) MId}

			{Dictionary.put InMsgIdSoftState MId
			 discover_route(SrcSite)|
			 {Dictionary.condGet InMsgIdSoftState MId nil}}
		     end
		  end
	       end
	    [] route_found(r:R) then V in
\ifdef DBGroute	       
	       {System.showInfo 'con:: receive route_found, msgid:'#MId}
\endif
	       if Dst == ThisSite then 
		  {Dictionary.condGet RcvProcs CId nil V}
		  
		  if V \= nil then
		     %% call the corresponding rcvProc
		     {V Pl}
		     
		     %% the first in, the only served
		     {Dictionary.remove RcvProcs CId}
		  end
	       else
		  {GetInterface discover_route
		   {Dictionary.get InMsgIdSoftState MId} V}
		  
		  if V \= nil then
\ifdef DBGroute	       		     
		     {System.showInfo 'con:: forward route to: '#
		      {GetSiteInfo V}.siteId#',  msgid:'#MId}
\endif		  		     
		     {SendMsgToSitePlus V
		      m(mt:Mt pt:Pt ttl:Ttl-1 src:Src dst:Dst via:Via
			cid:CId pl:route_found(r:ThisSite|R))
		      MId}
		  end
	       end
	       {Dictionary.put InMsgIdSoftState MId
		route_found(SrcSite)|
		{Dictionary.condGet InMsgIdSoftState MId nil}}
	    else
	       %!! do not care about eventual other messages
	       skip
	    end
	    
	    thread
	       %% remove the soft state after a certain time
	       {Delay 15000}
	       {Dictionary.remove InMsgIdSoftState MId} 
	    end
	 end
      else
	 skip
\ifdef DBGroute
	 {System.show misc_msg#Msg}
\endif
      end
   end
   
end

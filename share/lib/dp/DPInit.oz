%%%
%%% Authors:
%%%   Erik Klintskog (erik@sics.se)
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
   Glue     at 'x-oz://boot/Glue'
   ConnectAcceptModule
   ConnectionFunctor
   AcceptFunctor
   OS
\ifdef DBG
   System
\endif
export
   Init
   GetSettings
   GetLastSite
   GetAllSites
   GetConSites
   GetThisSite
   GetChannelStatus
   GetSiteInfo
   SendMsgToSite   
define

\ifdef DBG
   Show = System.show
\else
   Show = proc{$ _} skip end
\endif
  
   {Wait Glue}
   ConnectState = {NewCell notStarted}

   proc{CheckSettings R}
      if {Value.hasFeature R ip} then
	 try
	    {List.foldL {String.tokens R.ip &.}
	     fun{$ Acc In}
		I = {String.toInt In} in
		if I>256 orelse I<0 then raise toLarge end end
		Acc - 1
	     end
	     4 0}
	 catch _ then
	    raise badFormatedIpNo(R.ip) end
	 end
      end
   end

   fun{StartDP Settings}
      %% Here an AccMod is needed to start distribution.
      %% For c level a connection functor is also needed and has to be
      %% added to settings if not specified.
      AccMod = {CondSelect Settings acceptProc AcceptFunctor}
      IpPort
      NodeName
      InSettings
      LP Stream
   in
      {CheckSettings Settings}

\ifdef DBG
      try
	 {AccMod.accept {CondSelect Settings port default} IpPort NodeName}
      catch X then {System.show accept_ex(X)} raise X end end
\else
      {AccMod.accept {CondSelect Settings port default} IpPort NodeName}
\endif

      {Show IpPort}

      % Adding up what is missing to the settings structure
      
      InSettings = {FoldL [connectProc#fun{$} ConnectionFunctor.connectionfunctor end
			   port#fun{$} IpPort end
			   ip#fun{$}
				 AddrList = try
					       {OS.getHostByName NodeName}.addrList
					    catch _ then 
					       nil
					    end
			      in
				 case AddrList of nil then "127.0.0.1"
				 elseof [nil] then "127.0.0.1"
				 elseof L then L.1 end
			      end
			   id#fun{$} 1 end
			   %% temporary set the route intention here ... /V
			   toRoute#fun{$} false end]
		    fun{$ Ind F#P}
		       if  {HasFeature Ind F} then Ind
		       else{AdjoinAt Ind F {P}} end
		    end
		    Settings}
      
      LP = {NewPort Stream}
      {Show InSettings}
      {Glue.initIPConnection InSettings.ip InSettings.port InSettings.id LP}
      
      %% set RPC wrapper procedure
      {Glue.getRPC} = proc {$ P Args Ret}
			 try
			    {Procedure.apply P Args}
			    Ret=unit
			 catch E then
			    Ret={Value.failed E}
			 end
		      end
            
      {Delay 500}
      
      thread
	 %% temporary set the route intention here ... /V
	 {ConnectAcceptModule.initConnection Stream InSettings.toRoute InSettings.connectProc}
      end
      Settings
   end

   fun{Init Settings} O N in
      {Show dpInit(Settings)}
      {Exchange ConnectState O N}
      {Show exchanged(O)}
      case O of
	 notStarted then
	 {Show starting}
	 N={StartDP Settings}
	 true
      else
	 N=O
	 false
      end
   end

   fun{GetSettings}
      S={Access ConnectState}
   in
      case S of notStarted then
	 S
      else
	 MySite={Filter {Glue.siteStatistics}
		 fun{$ X} X.state==mine end}.1
      in
	 init(ip:MySite.ip
	      port:MySite.port
	      firewall:{CondSelect S firewall false}
	      address:MySite.addr
	      connectProc:{CondSelect S connectProc default}
	      acceptProc:{CondSelect S acceptProc default})
      end
   end
   fun{GetLastSite}
      {ConnectAcceptModule.lastSite}
   end
   
   fun {GetAllSites}
      {ConnectAcceptModule.getAllSites}
   end

   fun {GetConSites}
      {ConnectAcceptModule.getConSites}
   end

   fun {GetThisSite}
      {ConnectAcceptModule.getThisSite}
   end

   fun {GetChannelStatus S}
      {ConnectAcceptModule.getChannelStatus S}
   end

   fun {GetSiteInfo S}
      {ConnectAcceptModule.getSiteInfo S}
   end

   proc {SendMsgToSite S Msg}
      {ConnectAcceptModule.sendMsgToSite S Msg}
   end
end





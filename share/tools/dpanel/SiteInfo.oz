%%%
%%% Authors:
%%%   Nils Franzen <nilsf@sics.se>
%%%   Erik Klintskog <erik@sic.se>
%%%   Andreas Sundstroem 
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
   DPStatistics(siteStatistics)
export
   sitesDict:SitesDict
   sites:SiteInfo
define
   fun{FilterNew SiteStats SiteDict}
      {Filter SiteStats
       proc{$ SS Res}
	  Res = {Not {SiteDict member(SS.siteid $)}}
	  % Hack to make make sure to update the rtt of recently
	  % become nonactive sites.
	  if {Not Res} then
	     if SS.lastRTT==~1 then
		CurS={SiteDict getSite(SS.siteid $)}
	     in
		if {CurS getLastRTT($)}\=~1 then
		   {CurS setLastRTT(~1)}
		end
	     end
	  end
       end}
   end

   fun{FilterActive SiteStats SiteDict}
      {Filter SiteStats
       fun{$ SS}
	  SS.sent>0 orelse SS.received>0
       end}
   end
   
   fun{FilterOld SiteStats NewEntries SiteDict}
      if({Length SiteStats} - {Length NewEntries} ==
	 {SiteDict getNrOS($)}) then
	 nil
      else
	 {Filter {SiteDict getKeys($)}
	  fun{$ Key}
	     {All SiteStats fun{$ SS}
			       SS.siteid \= Key
			    end}
	  end}
      end
   end

   proc{UpdateStates GUISSites SiteStats SiteDict}
      {ForAll SiteStats
       proc{$ SS}
	  CurS={SiteDict getSite(SS.siteid $)}
       in
	  if {CurS updateState(SS.state $)} orelse SS.state==connected then
	     {GUISSites updateEntry(CurS.key {CurS getText($)})}
	  end
       end}
   end
   
   class Site
      feat
	 info
	 key 
	 GUI
	 sd
	 color
	 index
      attr
	 gen
	 sent
	 received
	 lastRTT
	 deltaSent
	 deltaReceived
	 graphKey
	 state
	 paneClient:false
	 selected:false
      meth init(S G SD Color Index)
	 self.key = S.siteid
	 self.info = S
	 self.GUI = G
	 self.sd = SD
	 self.index=Index
	 self.color=Color
	 gen <- 0
	 sent <- S.sent
	 received <- S.received
	 lastRTT <- {IntToFloat ~1}
	 deltaSent <- ~S.sent
	 deltaReceived <- ~S.received
	 state <- S.state
      end

      meth getColor($)
	 self.color
      end

      meth select selected <- true end
      meth deselect selected <- false end
      meth isSelected($) @selected  end
      
      meth paneClient paneClient <- true end 
      meth getGen($) @gen end
      meth setGen(G) gen<-G end
      meth getSent($) @deltaSent end
      meth sent(G) sent <- @sent + G deltaSent <- G end
      meth getReceived($) @deltaReceived end
      meth received(G) received <- @received + G deltaReceived <- G end
      meth getTotReceived($) @received end
      meth getTotSent($) @sent end
      meth setLastRTT(RTT) lastRTT<-{IntToFloat RTT} end
      meth getLastRTT($) @lastRTT end
      meth setGraphKey(K) graphKey<-K end
      meth getGraphKey($) @graphKey end
      meth getText($)
	 ClientIndicator = if @paneClient then "* " else "  " end
	 Id = self.index#"\t" %self.info.ip#":"#self.info.port#"\t"
	 State = @state#if @state==connected then
			   "("#{self getLastRTT($)}#")"
			else "" end
      in
	 ClientIndicator#Id#State
      end
      meth updateState(NewS ?Updated)
	 Updated = NewS \= (state<-NewS)
      end
   end
   
   class SitesDict
      prop
	 locking
      feat
	 GlobalDict:{NewDictionary} % 'Global' dictionary for use of same
	                            % color and index in server and clients
	 Colours:{NewCell unit}     % 'Global' color list
	 Index:{NewCell 0}          

	 Sites
	 GUI
      attr
	 NrOfSites
      meth init(C G)
	 self.Sites = {NewDictionary}
	 NrOfSites <- 0
	 lock
	    if {Access self.Colours}==unit then
	       {Assign self.Colours C}
	    end
	 end
	 self.GUI = G
      end

      meth getSite(S $)
	 {Dictionary.get self.Sites S}
      end

      meth newSite(S AS)
	 Color Index SS
      in
	 {self getColorAndIndex(S.siteid Color Index)}
	 SS={New Site init(S self.GUI self Color Index)}
	 {Dictionary.put self.Sites S.siteid SS}
	 NrOfSites <- @NrOfSites + 1
	 AS = entry(key:S.siteid text:{SS getText($)} fg:Color) 
      end
      
      meth removeSite(S AS)
	 AS = {Dictionary.get self.Sites S $}
	 {Dictionary.remove self.Sites S}
	 NrOfSites <- @NrOfSites - 1
      end
      
      meth getNrOS($) @NrOfSites end
      meth getKeys($) {Dictionary.keys self.Sites} end
      meth member(S A) A = {Dictionary.member self.Sites S} end

      meth getColorAndIndex(Key ?C ?I)
	 lock
	    E={Dictionary.condGet self.GlobalDict Key createnew}
	 in
	    if E ==createnew then
	       S = {Access self.Colours}
	       O
	    in
	       % Color
	       {Assign self.Colours S.2}
	       C=S.1
	       
	       % Index
	       {Exchange self.Index O I}
	       I=O+1

	       self.GlobalDict.Key:=C#I
	    else
	       C#I=E
	    end
	 end
      end
   end
	 
   class SiteInfo
      prop locking
      feat
	 GUI
	 sd
	 ActiveSites
      attr
	 Generation
	 notStarted:true
	 
      meth init(SD)
	 self.sd = SD
	 Generation <- 0
	 self.ActiveSites={NewDictionary}
      end

      meth setGui(G)
	 self.GUI = G
      end

      meth Update(active:ActiveEntries site_stats:SiteStats)
	 NewEntries = {FilterNew SiteStats self.sd}
	 !ActiveEntries = {FilterActive SiteStats self.sd}
	 DeleteEntries = {FilterOld SiteStats NewEntries self.sd}
      in      
	 {self.GUI.ssites deleteEntries(DeleteEntries)}
	 {self.GUI.ssites addEntries({Map NewEntries proc{$ X Y}
							{self.sd newSite(X Y)}
						     end})}
	 {UpdateStates self.GUI.ssites SiteStats self.sd}
	 {Map DeleteEntries proc{$ X Y} {self.sd removeSite(X Y)} end _}
      end

      meth display(I <= none)
	 AS
	 SSTtmp
	 SST
      in
	 lock
	    Generation<-(@Generation + 1) mod 100
	    if I == none then
	       SSTtmp = {DPStatistics.siteStatistics}
	    else
	       SSTtmp  = I
	    end
	    
	    %% Mine should be first in the list.
	    if @notStarted then
	       SST = {List.sort SSTtmp fun{$ S1 _} S1.state == mine end}
	       notStarted <- false 
	    else
	       SST = SSTtmp
	    end
	    
	    SiteInfo,Update(active:AS site_stats:SST)
	    SiteInfo,activeSites(AS)
	 end
      end
   
      meth activeSites(AS)
	 G  =  @Generation
	 GG = (@Generation + 1) mod 100
      in
	 % Add any new active sites and update the information of
	 % others present in AS
	 {ForAll AS
	  proc{$ S}
	     Si={self.sd getSite(S.siteid $)}
	  in
	     {Si setGen(G)}
	     {Si sent(S.sent)}
	     {Si received(S.received)}
	     {Si setLastRTT(S.lastRTT)}
	     if {Not {Dictionary.member self.ActiveSites S.siteid}} then
		Id  = {NewName}
		Col= {Si getColor($)} 
	     in
		self.ActiveSites.(S.siteid):=Si
		{Si setGraphKey(Id)}
		{Si sent(0)}
		{Si received(0)}
		{self.GUI.sactivity addGraph(key:Id col:Col stp:'' val:0.0)}
		{self.GUI.srtt addGraph(key:Id col:Col stp:'' val:0.0)}
	     end		
	  end}
	 % Remove any sites that have not been active for a long time
	 {ForAll {Dictionary.entries self.ActiveSites}
	  proc{$ K#SS}
	     if {SS getGen($)} == GG then
		{Dictionary.remove self.ActiveSites K}
		{self.GUI.sactivity
		 rmGraph(key:{SS getGraphKey($)})}
	     end
	  end}
	 local
	    % Make a list of activity display information
	    ActivityDL=
	    {Map {Dictionary.items self.ActiveSites}
	     fun{$ SS}
		if  {SS getGen($)} == G then
		   {IntToFloat
		    {SS getSent($)}+
		    {SS getReceived($)}}#{SS getGraphKey($)}
		else
		   0.0#{SS getGraphKey($)}
		end
	     end}
	    % Make a list of rtt display information
	    RTTDL=
	    {Filter
	     {Map {Dictionary.items self.ActiveSites}
	      fun{$ SS}
		 {SS getLastRTT($)}#{SS getGraphKey($)}
	      end}
	      fun{$ RTT#_} RTT\=~1.0 end}
	 in
	    {self.GUI.sactivity display(ActivityDL)}
	    {self.GUI.srtt display(RTTDL)}
	    %% Find eventual selected site and raise its line
	    case {Filter {Dictionary.items self.ActiveSites}
		  fun{$ S} {S isSelected($)} end}
	    of [S] then
	       {self.GUI.sactivity raise_line({S getGraphKey($)})}
	       {self.GUI.srtt raise_line({S getGraphKey($)})} 
	       
	    else skip
	    end

	 end
      end
   end
end

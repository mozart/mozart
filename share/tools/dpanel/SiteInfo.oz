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
   DPPane(siteStatistics) at 'x-oz://boot/DPPane'
   DPB at 'x-oz://boot/DPB'
   System
export
   sitesDict:SitesDict
   sites:SiteInfo
define
   {Wait DPB}
   
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
      attr
	 col
	 own
	 bor
	 act
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
      meth init(S G SD)
	 self.key = S.siteid
	 self.info = S
	 self.GUI = G
	 self.sd = SD
	 own <- false
	 bor <- false
	 act <- false
	 col <- none
	 gen <- 0
	 sent <- S.sent
	 received <- S.received
	 lastRTT <- {IntToFloat ~1}
	 deltaSent <- ~S.sent
	 deltaReceived <- ~S.received
	 state <- S.state
      end

      meth getCol(who:W $)
	 if @col == none then
	    col <- {self.sd getACol($)}
	    {self.GUI.ssites setColour(key:self.key fg:@col bg:lightgrey)}
	 else skip end
	 W <- true
	 @col
      end

      meth select selected <- true end
      meth deselect selected <- false end
      meth isSelected($) @selected  end
      
      
      meth retCol(who:W)
	 @W = true
	 W <- false
	 if @own andthen @bor andthen @act then
	    {self.GUI.ssites setColour(key:self.key fg:black bg:lightgrey)}
	    {self.sd retACol(@col)}
	    col <- none
	 else skip end
      end
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
      meth getText($) if @paneClient then "*" else "" end#self.info.ip#":"#self.info.port#"\t"#@state#if @state==connected then "("#{self getLastRTT($)}#")" else "" end
      end
      meth updateState(NewS ?Updated)
	 Updated = NewS \= (state<-NewS)
      end
   end
   
   class SitesDict
      feat
	 Sites
	 GUI
      attr
	 NrOfSites
	 Colours 
      meth init(C G)
	 self.Sites = {NewDictionary}
	 NrOfSites <- 0
	 Colours <- C
	 self.GUI = G
      end

      meth getSite(S $)
	 {Dictionary.get self.Sites S}
      end

      meth newSite(S AS)
	 SS={New Site init(S self.GUI self)}in
	 {Dictionary.put self.Sites S.siteid SS}
	 NrOfSites <- @NrOfSites + 1
	 AS = site(key:S.siteid text:{SS getText($)}) 
      end
      
      meth removeSite(S AS)
	 AS = {Dictionary.get self.Sites S $}
	 {Dictionary.remove self.Sites S}
	 NrOfSites <- @NrOfSites - 1
      end
      
      meth getNrOS($) @NrOfSites end
      meth getKeys($) {Dictionary.keys self.Sites} end
      meth member(S A) A = {Dictionary.member self.Sites S} end

      meth getACol($)
	 S = @Colours in
	 Colours <- S.2
	 S.1
      end

      meth retACol(C) Colours <- C|@Colours  end
   end
	 
   class SiteInfo
      prop locking
      feat
	 GUI
	 sd
	 ActiveSites
      attr
	 Generation
	 
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
	 {self.GUI.ssites deleteSite(DeleteEntries)}
	 {self.GUI.ssites addSite({Map NewEntries proc{$ X Y}
						     {self.sd newSite(X Y)}
						  end})}
	 {UpdateStates self.GUI.ssites SiteStats self.sd}
	 {Map DeleteEntries proc{$ X Y} {self.sd removeSite(X Y)} end _}
      end

      meth display(I <= none)
	 AS
	 SST
      in
	 lock
	    Generation<-(@Generation + 1) mod 100
	    if I == none then
	       SST = {DPPane.siteStatistics}
	    else
	       SST = I
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
		Col= {Si getCol(who:act $)} 
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
		{SS retCol(who:act)}
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

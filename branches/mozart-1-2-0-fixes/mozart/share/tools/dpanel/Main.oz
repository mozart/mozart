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

/*
Data types: 
 Site: site(ip:<ATOM>
            lastRTT:<INT>
            pid:<INT>
            port:<INT>
            received:<INT>
            sent:<INT>
            siteid:<ATOM>
            state:<ATOM>
            table:<ATOM>
            timestamp:<INT>)
*/

functor
import
   GUI
   TableInfo(ownerTable borrowTable fetchInfo)
   SiteInfo(sitesDict sites)
   Colour(list)
   Finalize(everyGC)
   NetInfo(netInfo) 
   MessageInfo 
   Browser
export
   Open
   OpenManualUpdate
   openNetInfo:OpenNetInfo
   server:Server
define
   %%
   %%TODO:
   %%
   %% Activity of entities does not work. An entity was
   %% said to be active when its credit count changed from
   %% one sample to another. This relied on implist message
   %% credit that has been removed from the system.
   %%
   %% To get proper values a sample should be taken before
   %% dpPane starts to get rid of accumulated information.
   %%

   MainLock = {NewLock}
   Running={NewCell false}
   RunSync = {NewCell unit}
   OpenNetInfo 
   
   %% Used by the distributed cntrl func
   ServerPort
   
   SelectSite
   
   class ClientClass
      feat
	 site
	 port
      attr
	 st:none
	 sd:none
	 gui:none
	 state:connected

      meth init(S P)
	 self.site = S
	 self.port = P
      end

      meth setup(sd:SD st:ST gui:GUI) = M 
	 {Record.forAllInd M proc{$ Ind V} Ind <- V end}
      end
      
      meth setState(S) state <- S end
      meth getState($) @state end 
      meth getST($) @st end 
      meth getGui($) @gui end
   end
   
   class ClientCntrler
      feat sd
      attr
	 clients:nil
	 
      meth getClient(S $)
	 {Filter @clients fun{$ CS} CS.site == S end}.1
      end

      meth memberClient(S $)
	 {Filter @clients fun{$ CS} CS.site == S end} \= nil
      end

      meth getOpenClients($)
	 {Filter @clients fun{$ CS} {Label {CS getState($)}} == start end} 
      end

      meth init(SD)  self.sd = SD end
      meth connecting(P S Id)
	 if {Not {self memberClient(S $)}} then
	    {Send P connected}
	    clients<-{New ClientClass init(S P)}|@clients 
	    thread
	       %% Ok, this is UGLY
	       %% The site might not be present in the sitedict yet. One could
	       %% offcourse think of syncronisation, but I'm feed up with this
	       %% poorly designed shit. I'll do some old fashined polling.
	       %% Erik, proud of producing ugly code.
	       
	       {For 1 10 1 proc{$ _}
			      {Delay 1000}
			      try 
				 {{self.sd getSite(Id $)} paneClient}
			      catch _ then skip end
			   end}
	    end
	 end
      end
      
      meth data(Sstat Site)
	 if {self memberClient(Site $)} then
	    Client =  {self getClient(Site $)}
	 in
	    if {Label {Client getState($)}} == start then 
	       {{Client getST($)} display(Sstat)}
	    end
	 end
      end


      
      meth open(Site)
	 if {self memberClient(Site $)} then
	    Client = {self getClient(Site $)}
	 in
	    if {Client getState($)} == connected then 
	       MGUI = {GUI.remoteSiteWin Site proc{$} {self close(Site)} end}
	       SD = {New SiteInfo.sitesDict init(Colour.list MGUI)}
	       ST = {New SiteInfo.sites init(SD)}
	       Action1 = proc{$ M}
			    S = {SD getSite(M $)}
			 in 
			    {SelectSite M}
			    {Browser.browse trafic(sent:{S getTotSent($)}
						   received:{S getTotReceived($)})}
			 end
	    in
	       {ST setGui(MGUI)}
	       {MGUI.ssites setAction(Action1)}
	       {Client setup(gui:MGUI sd:SD st:ST)}
	       {Client setState(start(1000))}
	       try
		  {Send Client.port start(1000)}
	       catch _ then skip end
	    end
	 end
      end
      
      meth close(Site)
	 if {self memberClient(Site $)} then
	    Client = {self getClient(Site $)}
	 in
	    if {Client getState($)} \= connected then 
	       {Client setState(connected)}
	       {{Client getGui($)}.top tkClose}
	       {Client setup(gui:none sd:none st:none)}
	       try 
		  {Send Client.port stop}
	       catch _ then skip end
	    end
	 end
      end

      meth selectSite(key:Key) 
	 {ForAll {self getOpenClients($)}
	  proc{$ S}
	     {{S getGui($)}.ssites select( key:Key)}
	  end}
      end

      meth deselectSite(key:Key) 
	 {ForAll {self getOpenClients($)}
	  proc{$ S}
	     {{S getGui($)}.ssites deselect( key:Key)}
	  end}
      end
      
   end

   
   proc {Open}
      Updater
   in
      {Start Updater}
      thread {UpdateLoop Updater} end
   end

   proc {OpenManualUpdate ?Updater}
      {Start Updater}
   end
   
   proc{Start ?UpdateObj} O N 
      proc{GCLineDraw}
	 {Finalize.everyGC proc{$}
			      lock MainLock then
				 if {Not {IsFree {Access RunSync}}} then 
				    {GUI.oactive   divider(col:darkred)}
				    {GUI.sactivity divider(col:darkred)}
				    {GUI.bactive   divider(col:darkred)}
				    {GUI.onumber   divider(col:darkred)}
				    {GUI.srtt   divider(col:darkred)}
				    {GUI.bnumber   divider(col:darkred)}
				 end
			      end
			   end}
      end

   in
      OpenNetInfo=GUI.openNetInfo
      {Exchange Running O N}
      if O\=false then
	 {GUI.reOpen}
	 %% Start the thread again
	 {Access RunSync} = unit
	 N=O
      else ST OT BT NI SD MI
	 ClientControler
	 SelectedSite = {NewCell none}
      in
	 SelectSite = proc{$ Key}
			 O in
			 %% Deselect the selected and select S
			 %% If S was selected deselct S
			 {Exchange SelectedSite O Key}
			 if O \= Key then 
			    {{SD getSite(Key $)} select}
			    {GUI.ssites select( key:Key)}
			    %% Select the site at all remote wins
			    {ClientControler selectSite(key:Key)}
			 else
			    {Assign SelectedSite none}
			 end
			 if O \= none then 
			    %% The site might have been removed due to gc
			    try{{SD getSite(O $)}deselect} catch _ then skip end
			    {GUI.ssites deselect(key:O )}
			    %% Deselect the site at all remote wins
			    {ClientControler deselectSite(key:O)}
			 end
		      end
	 
	 {GUI.open RunSync}
	 
	 N = true
	 
	 SD = {New SiteInfo.sitesDict init(Colour.list GUI)}
	 ST = {New SiteInfo.sites init(SD)}
	 OT = {New TableInfo.ownerTable init}
	 BT = {New TableInfo.borrowTable init(SD)}
	 NI = {New NetInfo.netInfo init(GUI)}
	 MI = {New MessageInfo.messageDiffInfoClass init(GUI)}
	 {ST setGui(GUI)}
	 {OT setGui(GUI.osites GUI.oactive GUI.onumber)}
	 {BT setGui(GUI.bsites GUI.bactive GUI.bnumber)}



	 {GUI.ssites setAction(proc{$ M} S =  {SD getSite(M $)}in
				  {Browser.browse S.info#trafic(sent:{S getTotSent($)}
								received:{S getTotReceived($)})}
				  {SelectSite M}
				  
			       end)}
	 {GCLineDraw}
	 {GUI.ssites setRightAction(proc{$ M} S =  {SD getSite(M $)} in
				       {ClientControler open(site(ip:S.info.ip port:S.info.port pid:S.info.pid))}
				    end)}
	 ClientControler = {New ClientCntrler init(SD)}

	 UpdateObj={New Updater init(ST OT BT NI MI)}
	 thread
	    {ForAll {NewPort $ ServerPort} proc{$ M}
					      try 
						  {ClientControler M}
					      catch _ then
						 skip
					      end
					   end}
	 end
      end
   end 

   class Updater
      feat
	 SiteTable
	 OwnerTable
	 BorrowTable
	 NetInfo
	 MessageInfo
      meth init(ST OT BT NI MI)
	 self.SiteTable=ST
	 self.OwnerTable=OT
	 self.BorrowTable=BT
	 self.NetInfo=NI
	 self.MessageInfo=MI
      end

      meth update(communication:ST<=true
		  exportedEntities:OT<=true
		  importedEntities:BT<=true
		  netInfo:NI<=true
		  messageInfo:MI<=true)
	 {Wait {Access RunSync}}
	 lock MainLock then
	    if ST then
	       {self.SiteTable display}
	    end
	    if OT orelse BT then 
	       {TableInfo.fetchInfo self.OwnerTable self.BorrowTable}
	    end
	    if OT then
	       {self.OwnerTable display}
	    end
	    if BT then
	       {self.BorrowTable display}
	    end
	    if NI then
	       {self.NetInfo display}
	    end
	    if MI then
	       {self.MessageInfo display}
	    end
	 end
      end
   end
   
   proc {UpdateLoop Updater}
      {Updater update}
      {Delay 2000}   
      {UpdateLoop Updater}
   end

   fun{Server}
      ServerPort
   end
end











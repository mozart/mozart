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
   GUI
   TableInfo(ownerTable borrowTable fetchInfo)
   SiteInfo(sitesDict sites)
   Colour(list)
   Finalize(everyGC)
   NetInfo(netInfo) 
   MessageInfo 
   Connection
   Pickle
   System
   Browser
export
   open:Start
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
   ServerList = {NewCell nil}
   PickledPort = {Connection.offerUnlimited ServerPort}

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
      attr
	 clients:nil
      meth getClient(S $)
	 {Filter @clients fun{$ CS} CS.site == S end}.1
      end

      meth memberClient(S $)
	 {Filter @clients fun{$ CS} CS.site == S end} \= nil
      end
      meth init skip end
      meth connecting(P S)
	 {System.show connectionAtempt#S}
	 if {Not {self memberClient(S $)}} then
	    {Send P connected}
	    clients<-{New ClientClass init(S P)}|@clients 
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
	    in
	       {ST setGui(MGUI)}
	       {Client setup(gui:MGUI sd:SD st:ST)}
	       {Client setState(start(1000))}
	       {Send Client.port start(1000)}
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
	       {Send Client.port stop}
	    end
	 end
      end
   end
   proc {Start} O N 
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

      ClientControler = {New ClientCntrler init}
   in
      OpenNetInfo=GUI.openNetInfo
      {System.show apa}
      {Exchange Running O N}
      if O\=false then
	 {GUI.reOpen}
	 %% Start the thread again
	 {Access RunSync} = unit
	 N=O
      else ST OT BT NI SD MI in
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
				  {ClientControler open(site(ip:S.info.ip port:S.info.port))}
				  {Browser.browse S.info#trafic(sent:{S getTotSent($)}
							received:{S getTotReceived($)})}
			       end)}
	 {GCLineDraw}
	 
	 thread {Updater ST OT BT NI MI} end
	 thread
	    {ForAll {NewPort $ ServerPort} ClientControler}
	 end
      end
   end 
   
   proc {Updater ST OT BT NI MI}
      {Wait {Access RunSync}}
      lock MainLock then
	 {ST display}
	 {TableInfo.fetchInfo OT BT}
	 {MI display}
	 {OT display}
	 {BT display}
	 {NI display}
      end
      {Delay 2000}   
      {Updater ST OT BT NI MI}
   end

   
   
   fun{Server}
      PickledPort
   end
   {System.show loadingmain}
end











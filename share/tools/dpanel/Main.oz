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
export
   open:Start
   openNetInfo:OpenNetInfo
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
   OpenNetInfo=GUI.openNetInfo
   
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
   in
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

	 {GCLineDraw}
	 
	 thread {Updater ST OT BT NI MI} end
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
end










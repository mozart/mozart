functor
import
   GUI
   TableInfo(ownerTable borrowTable fetchInfo)
   SiteInfo(sitesDict sites)
   Colour(list)
   Finalize(everyGC)
export
   open:Start
define
   SD
   MainLock = {NewLock}

   proc {Start} ST OT BT in
      SD = {New SiteInfo.sitesDict init(Colour.list GUI)}
      ST = {New SiteInfo.sites init(SD)}
      OT = {New TableInfo.ownerTable init}
      BT = {New TableInfo.borrowTable init(SD)}
%      G={MM apply(url:'' Graph $)}
%      {MM enter(name:'Graph' G)}
%      GUI={MM apply(url:'' A $)}
      {ST setGui(GUI)}
      {OT setGui(GUI.osites GUI.oactive GUI.onumber)}
      {BT setGui(GUI.bsites GUI.bactive GUI.bnumber)}
      {Finalize.everyGC proc{$}
			   lock MainLock then
			      {GUI.oactive   divider(col:darkred)}
			      {GUI.sactivity divider(col:darkred)}
			      {GUI.bactive   divider(col:darkred)}
			      {GUI.onumber   divider(col:darkred)}
			      {GUI.snumber   divider(col:darkred)}
			      {GUI.bnumber   divider(col:darkred)}
			   end
			end }
      {Updater ST OT BT}
   end 

   proc {Updater ST OT BT}
      lock MainLock then
	 {ST display}
	 {TableInfo.fetchInfo OT BT}
	 {OT display}
	 {BT display}
      end
      {Delay 2000}   
      {Updater ST OT BT}
   end
end





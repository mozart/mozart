functor
import
   GUI
   TableInfo(ownerTable borrowTable fetchInfo)
   SiteInfo(sitesDict sites)
   Colour(list)
   Finalize(everyGC)
   NetInfo(netInfo)
export
   open:Start
define
   SD 
   MainLock = {NewLock}
   Running={NewCell false}
   
   proc {Start} O N in
      {Exchange Running O N}
      if O\=false then
	 {GUI.open true O}
	 N=O
      else ST OT BT NI in
	 N={Thread.this}
	 {GUI.open false N}
	 
	 SD = {New SiteInfo.sitesDict init(Colour.list GUI)}
	 ST = {New SiteInfo.sites init(SD)}
	 OT = {New TableInfo.ownerTable init}
	 BT = {New TableInfo.borrowTable init(SD)}
	 NI = {New NetInfo.netInfo init(GUI)}
	 
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

	 {Updater ST OT BT NI}
      end
   end 
   
   proc {Updater ST OT BT NI}
      lock MainLock then
	 {ST display}
	 {TableInfo.fetchInfo OT BT}
	 {OT display}
	 {BT display}
	 {NI display}
      end
      {Delay 2000}   
      {Updater ST OT BT NI}
   end
end





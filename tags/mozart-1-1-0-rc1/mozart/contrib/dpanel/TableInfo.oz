functor
import
   Browser(browse)
   DPPane(getTablesInfo) at 'x-oz://boot/DPPane'
   DPB at 'x-oz://boot/DPB'
export
   ownerTable:OwnerTable
   borrowTable:BorrowTable
   fetchInfo:FetchInfo
define
   {Wait DPB}

   proc {SetDiff Xs Ys Zs}
      {List.filter Xs fun {$ X} {List.member X Ys}==false end Zs}
   end
   
   class ColorAlloc
      feat
	 retCol
	 getCol
	 colorDict
	 guiActive
	 guiNumber
      attr
	 freeIndexList
	 nextIndex
	 
      meth init(retCol:RC getCol:GC guiActive:GuiActive guiNumber:GuiNumber)
	 self.retCol = RC
	 self.getCol = GC
	 self.colorDict = {NewDictionary}
	 self.guiActive = GuiActive
	 self.guiNumber = GuiNumber
	 freeIndexList <- nil
	 nextIndex <- 1
      end
      
      meth get(S C I)
	 if {Dictionary.member self.colorDict S} then
	    C#I = {Dictionary.get self.colorDict S}
	 else
	    {self.getCol S C}
	    case @freeIndexList
	    of X|Xr then
	       I = X
	       freeIndexList <- Xr
	    [] nil then
	       I = @nextIndex
	       nextIndex <- I+1
	    end
	    {Dictionary.put self.colorDict S C#I}
	    {self.guiActive addGraph(key:I col:C stp:'' )}
	    {self.guiNumber addGraph(key:I col:C stp:'')}
	 end
      end
      
      meth free(S) I in
	 _#I = {Dictionary.get self.colorDict S}
	 {Dictionary.remove self.colorDict S}
	 {self.guiActive rmGraph(key:I)}
	 {self.guiNumber rmGraph(key:I)}      
	 if I+1 == @nextIndex then
	    @nextIndex = I
	 else
	    freeIndexList <- I|freeIndexList
	 end
	 {self.retCol S}
      end
   end
   
   class Table
      feat
	 guiSites
	 guiActive
	 guiNumber
	 colorAlloc
	 table
	 counter
	 diff
	 makeKey
	 makeSite
	 getCredit
	 
      attr
	 size
	 new
	 remove
	 temp1
	 temp2
	 
      meth initialize
	 size <- 0
	 self.table = {NewDictionary}
	 self.counter = {NewDictionary}
	 self.diff = {NewDictionary}
      end

      meth setGui(Sites Active Number)
	 self.guiSites = Sites
	 self.guiActive = Active
	 self.guiNumber = Number
	 {self.guiSites setAction(
			   proc{$ K}
			      {Browser.browse {Dictionary.get self.table K}}
			   end)}
      end

      meth increment(Dict Key)
	 if {Dictionary.member Dict Key} then Old I in
	    {Dictionary.get Dict Key Old#I}
	    {Dictionary.put Dict Key Old+1#I}
	 else I in
	    {self.colorAlloc get(Key _ I)}
	    {Dictionary.put Dict Key 1#I}
	 end
      end

      meth resetDictionary(Dict)
	 {List.forAll {Dictionary.keys Dict}
	  proc {$ Key} I in
	     {Dictionary.get Dict Key _#I}
	     {Dictionary.put Dict Key 0#I}
	  end}
      end

      meth removeDictObsolete(Dict)
	 {List.forAll @remove
	  proc {$ Key}
	     {Dictionary.remove Dict Key}
	  end}
      end
      
      meth updateEntity(Data Key) Site Col Index in
	 Key =  {self.makeKey Data}
	 Site = {self.makeSite Data}
	 if {Dictionary.member self.table Key} then
	    OldCredit = {self.getCredit {Dictionary.get self.table Key}}
	 in
	    if OldCredit == {self.getCredit Data} then
	       skip 
	    else
	       Table, increment(self.diff Site)
	    end
	 else
	    {self.colorAlloc get(Site Col Index)}
	    new <- site(key:Key fg:Col
		  text:{VirtualString.toAtom Data.index#'   '#Data.type})|@new 
	    Table, increment(self.diff Site)
	 end 
	 Table, increment(self.counter Site)
	 {Dictionary.put self.table Key Data}
      end
      
      meth update(Data) CurrentKeys in
	 size <- Data.size
	 Table, resetDictionary(self.counter)
	 Table, resetDictionary(self.diff)
	 new <- nil
	 {List.map Data.list proc {$ E K} OwnerTable, updateEntity(E K) end
	  CurrentKeys}
	 OwnerTable, removeObsolete(CurrentKeys)
      end
      
      meth displayGraph(Dict Graph) Keys DisplayList in
	 Keys = {Dictionary.keys Dict}
	 temp2 <- 0
	 {List.map Keys fun {$ K} N I in
			   N#I = {Dictionary.get Dict K}
			   temp2 <- @temp2+N
			   {Int.toFloat @temp2}#I
			end DisplayList}
	 {Graph display({List.reverse DisplayList})}
      end
      
      meth display
	 if @new \= nil then
	    {self.guiSites addSite(@new)}
	 else skip end
	 if @remove \= nil then
	    {self.guiSites deleteSite(@remove)}
	 else skip end
	 Table, displayGraph(self.counter self.guiNumber)
	 Table, displayGraph(self.diff self.guiActive)
      end
   end

   class OwnerTable from Table
      attr
	 localized
	 
      meth init
	 fun {GetCol _}
	    c(22 155 0)
	 end
	 proc {RetCol _ _}
	    skip
	 end
      in
	 Table, initialize
	 localized <- 0
	 self.makeKey = fun {$ E} E.index end
	 self.makeSite = fun {$ _} mySite end
	 self.getCredit = fun {$ E} E.credit end
	 self.colorAlloc = {New ColorAlloc init(retCol:RetCol getCol:GetCol
						guiActive:self.guiActive
						guiNumber:self.guiNumber)}
      end

      meth update(Data) Localized Key in
	 Localized = Data.localized
	 Key = {self.makeSite _}
	 Table, update(Data)
	 if Localized > 0 then Old I in
	    {Dictionary.get self.diff Key Old#I}
	    {Dictionary.put self.diff Key Old+Localized#I}
	 end
      end
      
      meth removeObsolete(CurrentKeyList)
	 RemovedKeys
	 KeyListAll = {Dictionary.keys self.table}
      in
	 {SetDiff KeyListAll CurrentKeyList RemovedKeys}
	 remove <- _
	 {List.map RemovedKeys proc {$ K I}
				  I = {Dictionary.get self.table K}.index
				  {Dictionary.remove self.table K}
			       end @remove}
	 Table, removeDictObsolete(self.diff)
	 Table, removeDictObsolete(self.counter)
      end
   end		    
   
   class BorrowTable from Table
      meth init(SD)
	 fun {RetCol S}
	    {{SD getSite(S $)} retCol(who:bor $)}
	 end
	 proc {GetCol S C}
	    {{SD getSite(S $)} getCol(who:bor C)}
	 end
      in
	 Table, initialize
	 self.makeKey = fun {$ E} E.index end
	 self.makeSite = fun {$ E} E.na.site end
	 self.getCredit = fun {$ E} credit(E.secCred E.primCred) end
	 self.colorAlloc = {New ColorAlloc init(retCol:RetCol getCol:GetCol
						guiActive:self.guiActive
						guiNumber:self.guiNumber)}
      end
      
      meth removeObsolete(CurrentKeyList)
	 RemovedKeys
	 KeyListAll = {Dictionary.keys self.table}
      in
	 {SetDiff KeyListAll CurrentKeyList RemovedKeys}
	 remove <- _
	 {List.map RemovedKeys
	  proc {$ K I} E in
	     E = {Dictionary.get self.table K}
	     I = E.index
	     if {Dictionary.member self.counter E.na.site} then
		if {Dictionary.get self.counter E.na.site}==0 then
		   {self.colorAlloc free(E.na.site)}
		else skip end
	     else
		{self.colorAlloc free(E.na.site)}
	     end
	     {Dictionary.remove self.table K}
	  end @remove}
	 Table, removeDictObsolete(self.diff)
	 Table, removeDictObsolete(self.counter)
      end
   end
   
   proc {FetchInfo OT BT} BTData OTData in
      [BTData OTData] = {DPPane.getTablesInfo $}
      {BT update(BTData)}
      {OT update(OTData)}
   end
end

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
   Browser(browse)
   DPStatistics(getTablesInfo)
export
   ownerTable:OwnerTable
   borrowTable:BorrowTable
   fetchInfo:FetchInfo

define

   class ColorAlloc
      feat
	 getCol
	 colorDict
	 guiActive
	 guiNumber
      attr
	 freeIndexList
	 nextIndex
      meth init(getCol:GC guiActive:GuiActive guiNumber:GuiNumber)
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
	    try
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
	    catch _ then % Key not found
	       skip
	    end
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
      end
   end
   
   class Table
      feat
	 guiSites
	 guiActive
	 guiNumber
	 colorAlloc
	 table
	 usedCounter
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
	 updates 
	 
      meth initialize
	 size <- 0
	 self.table = {NewDictionary}
	 self.counter = {NewDictionary}
	 self.diff = {NewDictionary}
	 self.usedCounter = {NewDictionary}
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
      
      meth updateEntity(Data Key) Site Col Index Used in
	 try 
	    Key =  {self.makeKey Data}
	    Site = {self.makeSite Data}
	    if {Dictionary.member self.table Key} then
	       OldCredit = {self.getCredit {Dictionary.get self.table Key}}
	    in
	       if OldCredit \= {self.getCredit Data} then
		  Used = self.usedCounter.Key + 1
		  Table, increment(self.diff Site)
		  updates <- Key|@updates 
	       else
		  Used = self.usedCounter.Key
	       end
	    else %% Entry not present in DPpanel yet. 
	       Used = 1
	       try
		  %% Get the color for the orgin site,
		  %% if not present raise notFound
		  {self.colorAlloc get(Site Col Index)}
	       catch _ then
		  raise notFound end
	       end
	       
	       new <- entry(key:Key fg:Col
			   text: Key#'   '#Data.type#' exp/imp'#1)|@new 
	       self.usedCounter.Key:=Used
	       Table, increment(self.diff Site)
	    end 
	    Table, increment(self.counter Site)
	    self.usedCounter.Key:=Used
	    self.table.Key:=Data
	 catch notFound then
	    %% The site where the entity comes from might not be present yet. 
	    %% If so the color alloc will fail and raise the exception 
	    %% 'notFound'. The entry will be inserted in the next update. 
	    skip
	 end
      end
      
      meth update(Data) CurrentKeys in
	 size <- Data.size
	 Table, resetDictionary(self.counter)
	 Table, resetDictionary(self.diff)
	 new <- nil
	 updates <- nil
	 CurrentKeys={Map Data.list proc{$ E K} Table,updateEntity(E K) end}
	 {self removeObsolete(CurrentKeys)}
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
      
      meth removeObsolete(CurrentKeyList)
	 TmpTable = {NewDictionary}
	 {ForAll CurrentKeyList proc{$ E} TmpTable.E:=false end}
	 RemovedKeys = {Filter {Dictionary.keys self.table} fun{$ K} {CondSelect TmpTable K true} end}
      in
	 remove <- _
	 {List.map RemovedKeys proc {$ K I}
				  I = {self.makeKey {Dictionary.get self.table K}}
				  {Dictionary.remove self.table K}
			       end @remove}
	 Table, removeDictObsolete(self.diff)
	 Table, removeDictObsolete(self.counter)
      end
	    
      
      meth display
	 if @new \= nil then
	    {self.guiSites addEntries(@new)}
	 end
	 if @remove \= nil then
	    {self.guiSites deleteEntries(@remove)}
	 end
	 {ForAll @updates
	  proc{$ K}
	     Data = self.table.K
	     Us = self.usedCounter.K
	  in
	     {self.guiSites updateEntry(K {self.makeKey Data}#'   '#Data.type#' exp/imp:'#Us)}
	  end}
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
      in
	 Table, initialize
	 localized <- 0
	 self.makeKey = fun {$ E} E.odi end
	 self.makeSite = fun {$ _} mySite end
	 self.getCredit = fun {$ E} E.dist_gc end
	 self.colorAlloc = {New ColorAlloc init(getCol:GetCol
						guiActive:self.guiActive
						guiNumber:self.guiNumber)}
      end

      meth update(Data) 
	 Localized = Data.localized
	 Key = {self.makeSite _}
      in
	 Table, update(Data)
	 %% Calculates the total number of entries. All localized
	 %% entities should be acounted for during this this
	 if Localized > 0 then Old I in
	    {Dictionary.get self.diff Key Old#I}
	    {Dictionary.put self.diff Key Old+Localized#I}
	 end
      end
      
   end		    
   
   class BorrowTable from Table
      meth init(SD)
	 proc {GetCol S C}
	    Site = {SD getSite(S $)}
	 in
	    {Site getColor(C)}
	 end
      in
	 Table, initialize
	 self.makeKey = fun {$ E} E.na.index end
	 self.makeSite = fun {$ E} E.na.site end
	 self.getCredit = fun {$ E} credit(E.dist_gc) end
	 self.colorAlloc = {New ColorAlloc init(getCol:GetCol
						guiActive:self.guiActive
						guiNumber:self.guiNumber)}
      end
   end
   
   proc {FetchInfo OT BT} BTData OTData in
      [BTData OTData] = {DPStatistics.getTablesInfo $}
      {BT update(BTData)}
      {OT update(OTData)}
   end
end



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
   Colour(list)
   DPStatistics(getNetInfo)
export
   netInfo:NetInfo
define
   class InfoEntry
      attr
	 type
	 nr
	 size
	 color
	 string
	 key
	 
      meth init(Data Color)
	 color <- Color
	 type <- Data.type
	 nr <- Data.nr
	 size <- Data.size * Data.nr
	 string <- ''
	 key <- 'not set'
      end

      meth update(Data)
	 nr <- Data.nr
	 size <- Data.size * Data.nr
      end

      meth changedString(Delete Add Changed)
	 New = {VirtualString.toAtom
		@type#':   '#@nr#'  '#@size}
      in
	 if New \= @string then
	    Changed = true
	    Delete = @key
	    Add = entry(key:@key fg:@color text:New)
	    string <- New
	 else
	    Changed = false
	 end
      end 

      meth addToGraph(NGraph BGraph NextKey)
	 if @key == 'not set' then Old New in
	    thread {Cell.exchange NextKey Old New} end
	    New = Old+1
	    key <- Old
	    {NGraph addGraph(key:@key col:@color stp:'' val:0.0)}
	    {BGraph addGraph(key:@key col:@color stp:'' val:0.0)}
	 end
      end
      
      meth numberValue(V0 $)
	 {IntToFloat @nr}#@key
      end
      meth sizeValue(V0 $) Old New in
	 thread {Cell.exchange V0 Old New} end
	 New = Old + @size
	 {IntToFloat New}#@key
      end
   end
   
   class NetInfo
      feat
	 info
	 nextKey
	 guiList
	 guiNumber
	 guiByte
	 val0
      attr
	 colorList
	 
      meth init(GUI)
	 colorList <- Colour.list
	 self.info = {NewDictionary}
	 self.nextKey = {NewCell 1}
	 self.val0 = {NewCell 0}
	 self.guiList = GUI.nilist
	 self.guiNumber = GUI.ninumber
	 self.guiByte = GUI.nibyte 
      end
      
      meth update
	 Data
	 proc {Store Data}
	    if {Dictionary.member self.info Data.type} then O in
	       O = {Dictionary.get self.info Data.type}
	       {O update(Data)}
	    else
	       {Dictionary.put self.info Data.type
		{New InfoEntry init(Data @colorList.1)}}
	       colorList <- @colorList.2
	    end
	 end
      in
	 {DPStatistics.getNetInfo Data}
	 {List.forAll Data Store}
      end

      meth displayList
	 proc {DisplayList Ks OldList NewList}
	    case Ks
	    of K|Kr then
	       O = {Dictionary.get self.info K}
	       Old New
	    in
	       {O addToGraph(self.guiNumber self.guiByte self.nextKey)}
	       if {O changedString(Old New $)} then
		  OldList1 NewList1
	       in
		  OldList = Old|OldList1
		  NewList = New|NewList1
		  {DisplayList Kr OldList1 NewList1}
	       else
		  {DisplayList Kr OldList NewList}
	       end
	    [] nil then
	       OldList = nil
	       NewList = nil
	    end
	 end
	 OldList NewList
      in 
	 {DisplayList {Dictionary.keys self.info} OldList NewList}
	 if OldList \= nil then
	    {self.guiList deleteEntries(OldList)}
	    {self.guiList addEntries(NewList)}
	 end
      end
      meth displayNumber
	 fun {GetValue Key}
	    O = {Dictionary.get self.info Key}
	 in
	    {O numberValue(self.val0 $)}
	 end
	 Values
      in
	 {List.map {Dictionary.keys self.info} GetValue Values}
	 {self.guiNumber display({List.reverse Values})}
      end
      meth displayByte
	 fun {GetValue Key}
	    O = {Dictionary.get self.info Key}
	 in
	    {O sizeValue(self.val0 $)}
	 end
	 Values
      in
	 {List.map {Dictionary.keys self.info} GetValue Values}
	 {self.guiByte display({List.reverse Values})}
      end
      meth display
	 NetInfo, update
	 NetInfo, displayList
	 NetInfo, displayNumber
	 {Cell.assign self.val0 0}
	 NetInfo, displayByte
      end
   end

end

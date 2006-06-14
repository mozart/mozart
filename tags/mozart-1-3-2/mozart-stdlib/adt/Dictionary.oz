functor
export
   New NewFromRecord
prepare
   RemoveAll = Dictionary.removeAll
   Keys      = Dictionary.keys
   Items     = Dictionary.items
   Entries   = Dictionary.entries
   IsEmpty   = Dictionary.isEmpty
   Remove    = Dictionary.remove
   Member    = Dictionary.member
   Clone     = Dictionary.clone
   ToRecord  = Dictionary.toRecord
   ToDict    = Record.toDictionary
   fun {DictPack D}
      fun  {DictGet     K  } D.K end
      proc {DictPut     K V} D.K := V end
      fun  {DictCondGet K V} {CondSelect D K V} end
      proc {DictReset      } {RemoveAll D} end
      fun  {DictKeys       } {Keys D} end
      fun  {DictItems      } {Items D} end
      fun  {DictEntries    } {Entries D} end
      fun  {DictIsEmpty    } {IsEmpty D} end
      proc {DictRemove  K  } {Remove D K} end
      fun  {DictMember  K  } {Member D K} end
      fun  {DictClone      } {DictPack {Clone D}} end
      fun  {DictToRecord L } {ToRecord L D} end
   in
      dictionary(
	 get      : DictGet
	 put      : DictPut
	 condGet  : DictCondGet
	 reset    : DictReset
	 keys     : DictKeys
	 items    : DictItems
	 entries  : DictEntries
	 isEmpty  : DictIsEmpty
	 remove   : DictRemove
	 member   : DictMember
	 clone    : DictClone
	 toRecord : DictToRecord)
   end
   fun {New} {DictPack {NewDictionary}} end
   fun {NewFromRecord R} {DictPack {ToDict R}} end
end
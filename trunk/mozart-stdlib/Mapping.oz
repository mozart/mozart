functor
export
   Get Put CondGet condSelect:CondGet
   Is IsMutable
   Items   toList:Items
   Keys    arity:Keys
   Entries toListInd:Entries
   Clone Exchange ToRecord
prepare
   fun {Get M Key} M.Key end
   proc {Put M Key Value} M.Key := Value end
   fun {CondGet M Key Default} {CondSelect M Key Default} end
   fun {Is X}
      {IsRecord     X} orelse
      {IsArray      X} orelse
      {IsDictionary X}
   end
   fun {IsMutable X}
      {IsArray X} orelse {IsDictionary X}
   end
   RecToList = Record.toList
   ArrayHi = Array.high
   ArrayLo = Array.low
   fun {ArrayToList A}
      Lo = {ArrayLo A}
      Hi = {ArrayHi A}
   in
      {ArrayToListX A Lo Hi}
   end
   fun {ArrayToListX A Lo Hi}
      if Lo>Hi then nil else
	 A.Lo|{ArrayToListX A Lo+1 Hi}
      end
   end
   DictItems = Dictionary.items
   fun {Items X}
      if {IsDictionary X} then {DictItems X}
      elseif {IsRecord X} then {RecToList X}
      else {ArrayToList X} end
   end
   DictKeys = Dictionary.keys
   fun {Keys X}
      if {IsDictionary X} then {DictKeys X}
      elseif {IsRecord X} then {Arity X}
      else {ArrayArity X} end
   end
   ListNumber = List.number
   fun {ArrayArity A}
      Lo = {ArrayLo A}
      Hi = {ArrayHi A}
   in
      {ListNumber Lo Hi 1}
   end
   DictEntries = Dictionary.entries
   RecToListInd = Record.toListInd
   fun {Entries X}
      if {IsDictionary X} then {DictEntries X}
      elseif {IsRecord X} then {RecToListInd X}
      else {ArrayToListInd X} end
   end
   fun {ArrayToListInd A}
      Lo = {ArrayLo A}
      Hi = {ArrayHi A}
   in
      {ArrayToListIndX A Lo Hi}
   end
   fun {ArrayToListIndX A Lo Hi}
      if Lo>Hi then nil else
	 (Lo#A.Lo)|{ArrayToListX A Lo+1 Hi}
      end
   end
   DictClone = Dictionary.clone
   ArrayClone = Array.clone
   fun {Clone M}
      if {IsDictionary M} then {DictClone M}
      elseif {IsRecord M} then M
      else {ArrayClone M} end
   end
   DictExchange = Dictionary.exchange
   ArrayExchange = Array.exchange
   proc {Exchange M Key Old New}
      if {IsDictionary M} then {DictExchange M Key Old New}
      elseif {IsRecord M} then M.Key=Old=New
      else {ArrayExchange M Key Old New} end
   end
   DictToRecord = Dictionary.toRecord
   ArrayToRecord = Array.toRecord
   fun {ToRecord Label M}
      if {IsDictionary M} then {DictToRecord Label M}
      elseif {IsRecord M} then {Adjoin M Label}
      else {ArrayToRecord Label M} end
   end
end

functor
export
   ToURL ToString ToAtom
   ToBaseURL ToBase ToBaseAtom
   ToNonBaseURL ToNonBase ToNonBaseAtom
   ResolveURL Resolve ResolveAtom
   IsRelative
   IsAbsolute
   ExpandURL Expand ExpandAtom
   BasenameURL Basename BasenameAtom IsBasename
   DirnameURL Dirname DirnameAtom
   SplitExtension
   Extension ExtensionAtom
   DropExtensionURL DropExtension DropExtensionAtom
   AddExtensionURL AddExtension AddExtensionAtom
   ReplaceExtensionURL ReplaceExtension ReplaceExtensionAtom
   AddPlatformURL AddPlatform AddPlatformAtom
   MaybeAddPlatformURL MaybeAddPlatform MaybeAddPlatformAtom
   ToCacheURL ToCache ToCacheAtom
   Stat SafeStat Dir Exists IsDir IsFile
   UnresolveURL Unresolve UnresolveAtom
   IsAncestor
   MakeDirectory MakeDirectoryRecursive
   Remove RemoveRecursive
   Copy
import
   URL
   Misc(
      isWindowsAncient : IsWindowsAncient)
   at 'os/Misc.ozf'
   RESOLVE at 'x-oz://system/Resolve.ozf'
   OS
   Shell at 'Shell.ozf'
   Property
prepare
   VSToString = VirtualString.toString
   VSToAtom   = VirtualString.toAtom
   TakeDropWhile = List.takeDropWhile
   fun {NotIsDot S} S\="." andthen S\=".." end
   FULL_RAW_CACHE = o(cache:true full:true raw:true)
   NOT_FOUND = notFound(type:unknown size:0)
   IsPrefix = List.isPrefix
define

   ToURL
   local
      proc {State0 S U}
	 case S
	 of D|&:|T then U.device=D    {State1 T U}
	 else           U.device=unit {State1 S U} end
      end
      proc {State1 S U}
	 case S
	 of &/ |T then U.absolute=true  {State2 T nil nil U}
	 [] &\\|T then U.absolute=true  {State2 T nil nil U}
	 else          U.absolute=false {State2 S nil nil U} end
      end
      proc {State2 S Comp Comps U}
	 case S
	 of nil then U.path={Reverse {Reverse Comp}|Comps}
	 [] &/|T then {State2 T nil {Reverse Comp}|Comps U}
	 []  H|T then {State2 T H|Comp Comps U}
	 end
      end
   in
      fun {ToURL S}
	 if {URL.is S} then S
	 else U=url(device:_ absolute:_ path:_) in
	    {State0 {VSToString S} U}
	    U
	 end
      end
   end

   SLASH = if IsWindowsAncient then &\\ else &/ end

   ToString
   local
      fun {State0 U}
	 if U.device\=unit then
	    U.device|&:|{State1 U}
	 else {State1 U} end
      end
      fun {State1 U}
	 if U.absolute then SLASH|{State2 U.path}
	 else {State2 U.path} end
      end
      fun {State2 L}
	 case L
	 of nil then nil
	 [] H|T then {Append H {State2 T}}
	 end
      end
   in
      fun {ToString U}
	 {State0 {ToURL U}}
      end
   end
   fun {ToAtom P} {VSToAtom {ToString P}} end

   fun {ToBaseURL P} {URL.toBase {ToURL P}} end
   fun {ToBase    P} {ToString {ToBaseURL P}} end
   fun {ToBaseAtom P} {VSToAtom {ToBase P}} end

   fun {ToNonBaseURL P} U={ToURL P} in
      case {Reverse {CondSelect U path nil}}
      of [nil] then U
      [] nil|T then {AdjoinAt U path {Reverse T}}
      else U end
   end
   fun {ToNonBase P} {ToString {ToNonBaseURL P}} end
   fun {ToNonBaseAtom P} {VSToAtom {ToNonBase P}} end

   fun {ResolveURL Base Rel}
      {URL.resolve {ToBaseURL Base} {ToURL Rel}}
   end
   fun {Resolve Base Rel}
      {ToString {ResolveURL Base Rel}}
   end
   fun {ResolveAtom Base Rel}
      {VSToAtom {Resolve Base Rel}}
   end

   fun {IsRelative P} {URL.isRelative {ToURL P}} end
   fun {IsAbsolute P} {URL.isAbsolute {ToURL P}} end

   fun {ExpandURL  P} {RESOLVE.expand {ToURL P}} end
   fun {Expand     P} {ToString {ExpandURL P}} end
   fun {ExpandAtom P} {VSToAtom {Expand P}} end

   fun {BasenameURL P} U={ToURL P} in
      case {Reverse {CondSelect U path nil}}
      of H|_ then url(path:[H])
      else url(unit) end
   end
   fun {Basename P} {ToString {BasenameURL P}} end
   fun {BasenameAtom P} {VSToAtom {Basename P}} end

   fun {IsBasename P} U={ToURL P} in
      {IsRelative U} andthen
      case {CondSelect U path unit}
      of [S] then S\=nil
      else false end
   end

   fun {DirnameURL P} U={ToURL P} in
      case {Reverse {CondSelect U path nil}}
      of _|T then {AdjoinAt U path {Reverse T}}
      else url(unit) end
   end
   fun {Dirname P} {ToString {DirnameURL P}} end
   fun {DirnameAtom P} {VSToAtom {Dirname P}} end

   proc {SplitExtension P U E}
      UU={ToURL P}
   in
      case {Reverse {CondSelect UU path nil}}
      of H|T then Ext Rest in
	 {TakeDropWhile {Reverse H} NotIsDot Ext Rest}
	 case Rest of &.|Rest then
	    U={AdjoinAt UU path {Reverse {Reverse Rest}|T}}
	    E={Reverse Ext}
	 else U=UU E=unit end
      else U=UU E=unit end
   end
   fun {Extension P} {SplitExtension P _ $} end
   fun {ExtensionAtom P} E={Extension P} in
      if E==unit then unit else {VSToAtom E} end
   end
   
   fun {DropExtensionURL  P} {SplitExtension P $ _} end
   fun {DropExtension     P} {ToString {DropExtensionURL P}} end
   fun {DropExtensionAtom P} {VSToAtom {DropExtension P}} end

   fun {AddExtensionURL P E} U={ToURL P} in
      case {Reverse {CondSelect U path nil}}
      of H|T then {AdjoinAt U path
		   {Reverse {Append H &.|{VSToString E}}|T}}
      end
   end
   fun {AddExtension P E} {ToString {AddExtensionURL P E}} end
   fun {AddExtensionAtom P E} {VSToAtom {AddExtension P E}} end

   fun {ReplaceExtensionURL P E}
      {AddExtensionURL {DropExtensionURL P} E}
   end
   fun {ReplaceExtension P E}
      {ToString {ReplaceExtensionURL P E}}
   end
   fun {ReplaceExtensionAtom P E}
      {VSToAtom {ReplaceExtension P E}}
   end

   PLATFORM = {Property.get 'platform'}

   fun {AddPlatformURL P} U={ToURL P} in
      case {Reverse {CondSelect U path nil}}
      of H|T then {AdjoinAt U path
		   {Reverse
		    {VSToString H#'-'#PLATFORM.name}|T}}
      else U end
   end
   fun {AddPlatform P} {VSToString {AddPlatformURL P}} end
   fun {AddPlatformAtom P} {VSToAtom {AddPlatform P}} end

   fun {MaybeAddPlatformURL P} U={ToURL P} in
      if {ExtensionAtom P}=='so' then
	 {AddPlatformURL U}
      else U end
   end
   fun {MaybeAddPlatform P}
      {VSToString {MaybeAddPlatformURL P}}
   end
   fun {MaybeAddPlatformAtom P}
      {VSToAtom {MaybeAddPlatform P}}
   end

   fun {ToCache P}
      {URL.toVirtualStringExtended {URL.make P} FULL_RAW_CACHE}
   end
   fun {ToCacheURL P} {ToURL {ToCache P}} end
   fun {ToCacheAtom P} {VSToAtom {ToCache P}} end

   fun {Stat P} {OS.stat {ToString P}} end
   fun {SafeStat P}
      try {Stat P} catch _ then NOT_FOUND end
   end

   fun {Dir P}
      {Filter {OS.getDir {ToString P}} NotIsDot}
   end

   fun {Exists P} {SafeStat P}.type\=unknown end
   fun {IsDir  P} {SafeStat P}.type==dir end
   fun {IsFile P} {SafeStat P}.type==reg end

   fun {Unresolve Base Path}
      B={Append {Expand {ToBase Base}} "/"}
      P={Expand Path}
   in
      if {IsPrefix B P} then {Append B $ P} else P end
   end
   fun {UnresolveURL Base Path}
      {ToURL {Unresolve Base Path}}
   end
   fun {UnresolveAtom Base Path}
      {VSToAtom {Unresolve Base Path}}
   end

   fun {IsAncestor Base Path}
      B={Append {Expand {ToBase Base}} "/"}
      P={Expand Path}
   in
      {IsPrefix B P} 
   end

   proc {MakeDirectory P}
      U = {ExpandURL {ToNonBaseURL P}}
   in
      case {SafeStat U}.type
      of 'unknown' then {OS.mkDir {ToString U}}
      [] 'dir'     then skip
      else
	 {Exception.raiseError path(mkdir({ToAtom U}))}
      end
   end

   proc {MakeDirectoryRecursive P}
      U = {ExpandURL {ToNonBaseURL P}}
   in
      case {CondSelect U path nil}
      of nil then skip
      [] [nil] then skip
      else
	 {MakeDirectoryRecursive {DirnameURL U}}
	 {MakeDirectory U}
      end
   end

   proc {Remove P}
      U = {ExpandURL {ToNonBaseURL P}}
   in
      if {IsFile U} then {OS.unlink {ToString U}}
      elseif {IsDir U} then
	 {Shell.executeCommand [rmdir {ToString U}]}
      else {Exception.raiseError path(rm({ToString U}))} end
   end

   proc {RemoveRecursive P}
      U = {ExpandURL {ToNonBaseURL P}}
   in
      if     {IsFile U} then {OS.unlink {ToString U}}
      elseif {IsDir  U} then
	 for E in {Dir U} do {RemoveRecursive {ResolveURL U E}} end
	 {Remove U}
      else {Exception.raiseError path(rm({ToString U}))} end
   end

   proc {Copy P1 P2}
      {Shell.executeCommand [cp {Expand P1} {Expand P2}]}
   end
end

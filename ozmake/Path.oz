functor
   %% This module provides an improved interface to pathnames.
export
   ToURL
   ToBaseURL		ToBase
   ToNonBaseURL		ToNonBase       ToNonBaseAtom
   DropInfoURL          DropInfo        DropInfoAtom
   ResolveURL		Resolve		ResolveAtom
   IsRelative IsAbsolute ToString ToAtom
   ExpandURL 		Expand
   BasenameURL		Basename	BasenameAtom
   DirnameURL		Dirname		DirnameAtom
   Extension		ExtensionAtom
   DropExtensionURL	DropExtension   DropExtensionAtom
   AddExtensionURL	AddExtension	AddExtensionAtom
   ReplaceExtensionURL	ReplaceExtension ReplaceExtensionAtom
   MaybeAddPlatform
   ToCacheURL ToCache Stat SafeStat Dir Ls
   Exists IsDir IsFile
   Unresolve		UnresolveURL   UnresolveAtom
   IsAncestor
   MakeDir MakeDirRec
   Remove RemoveDir RemoveRec
   CopyFile
   IsBasename
prepare
   %% Introduce some vars so we don't have to close over
   %% entire base modules

   VSToString    = VirtualString.toString
   TakeDropWhile = List.takeDropWhile
   IsPrefix      = List.isPrefix
   
   %% paths may contains characters which have special meanings
   %% for URIs and may be misinterpreted by the general Mozart
   %% URL module.  For this reason, these characters need to be
   %% protected by encoding, prior to being processed by module
   %% URL.
   
   fun {PROTECT S}
      case S
      of nil then nil
      [] &?|T then &%|&3|&f|{PROTECT T}
      [] &#|T then &%|&2|&3|{PROTECT T}
      [] &{|T then &%|&7|&b|{PROTECT T}
      []  H|T then        H|{PROTECT T}
      end
   end

   fun {NotIsDot S}
      {Not S=="." orelse S==".."}
   end

   fun {NotIsPeriod C} C\=&. end

   NOT_FOUND = notFound(type:unknown size:0)
   FULL_RAW = o(full:true raw:true)
   FULL_RAW_CACHE = o(cache:true full:true raw:true)
import
   URL     at 'x-oz://system/URL'
   RESOLVE at 'x-oz://system/Resolve.ozf'
   OS      at 'x-oz://system/OS.ozf'
   Shell   at 'Shell.ozf'
   Property
   Windows at 'Windows.ozf'
define
   %% turn the argument into a URL record, if it is not
   %% already one, protecting special characters

   fun {ToURL P}
      if {URL.is P} then P else
	 {URL.make {PROTECT {VSToString P}}}
      end
   end

   fun {ToBaseURL P} {URL.toBase {ToURL P}} end
   fun {ToBase P} {ToString {ToBaseURL P}} end

   fun {ToNonBaseURL P}
      U = {ToURL P}
   in
      case {Reverse {CondSelect U path nil}}
      of [nil] then U
      [] nil|T then {AdjoinAt U path {Reverse T}}
      else U end
   end
   fun {ToNonBase P} {ToString {ToNonBaseURL P}} end
   fun {ToNonBaseAtom P} {ToAtom {ToNonBaseURL P}} end
      
   fun {ResolveURL Base Rel}
      {URL.resolve {ToBase Base} {ToURL Rel}}
   end
   fun {Resolve Base Rel} {ToString {ResolveURL Base Rel}} end
   fun {ResolveAtom Base Rel} {StringToAtom {Resolve Base Rel}} end

   fun {IsRelative P} {URL.isRelative {ToURL P}} end
   fun {IsAbsolute P} {URL.isAbsolute {ToURL P}} end

   fun {DropInfoURL U} {AdjoinAt {ToURL U} info unit} end
   fun {DropInfo U} {ToString {DropInfoURL U}} end
   fun {DropInfoAtom U} {ToAtom {DropInfoURL U}} end

   %% On Win95/98 (aka old Windows), we need to use old style
   %% pathnames with \ instead of /.
   
   ToString
   if Windows.isOldWin then
      fun {DeS L}
	 case L of nil then nil
	 [] &/|L then &\\|{DeS L}
	 [] H|L then H|{DeS L} end
      end
   in
      fun {!ToString P}
	 U={ToURL P}
	 S={VSToString
	    {URL.toVirtualStringExtended U FULL_RAW}}
      in
	 if {CondSelect U scheme unit}==unit andthen
	    {CondSelect U authority unit}==unit
	 then {DeS S} else S end
      end
   else
      fun {!ToString P}
	 {VSToString
	  {URL.toVirtualStringExtended
	   {ToURL P} FULL_RAW}}
      end
   end

   fun {ToAtom P} {StringToAtom {ToString P}} end

   fun {ExpandURL P} {RESOLVE.expand {ToURL P}} end
   fun {Expand P} {ToString {ExpandURL P}} end

   fun {Basename P}
      case {Reverse {CondSelect {ToNonBaseURL P} path nil}}
      of H|_ then H
      else nil end
   end
   fun {BasenameURL P} {ToURL {Basename P}} end
   fun {BasenameAtom P} {StringToAtom {Basename P}} end

   fun {DirnameURL P}
      U = {ToNonBaseURL P}
   in
      case {Reverse {CondSelect U path nil}}
      of _|T then {AdjoinAt U path {Reverse T}}
      else {ToURL nil} end
   end
   fun {Dirname P} {ToString {DirnameURL P}} end
   fun {DirnameAtom P} {StringToAtom {Dirname P}} end

   proc {SplitExtension P U E}
      UU={ToURL P}
   in
      case {Reverse {CondSelect UU path nil}}
      of H|T then Ext Rest in
	 {TakeDropWhile {Reverse H} NotIsPeriod Ext Rest}
	 case Rest of &.|Rest then
	    U={AdjoinAt UU path {Reverse {Reverse Rest}|T}}
	    E={Reverse Ext}
	 else U=UU E=unit end
      else U=UU E=unit end
   end

   fun {Extension P} {SplitExtension P _ $} end
   fun {ExtensionAtom P}
      E={Extension P}
   in
      if E==unit then unit else {StringToAtom E} end
   end
   fun {DropExtensionURL  P} {SplitExtension P $ _} end
   fun {DropExtension     P} {ToString {DropExtensionURL P}} end
   fun {DropExtensionAtom P} {ToAtom   {DropExtensionURL P}} end

   fun {AddExtensionURL P E}
      U = {ToURL P}
   in
      case {Reverse {CondSelect U path nil}}
      of H|T then
	 {AdjoinAt U path
	  {Reverse {Append H &.|{VSToString E}}|T}}
      end
   end
   fun {AddExtension P E} {ToString {AddExtensionURL P E}} end
   fun {AddExtensionAtom P E} {StringToAtom {AddExtension P E}} end

   fun {ReplaceExtensionURL P E}
      {AddExtensionURL {DropExtensionURL P} E}
   end
   fun {ReplaceExtension P E}
      {ToString {ReplaceExtensionURL P E}}
   end
   fun {ReplaceExtensionAtom P E}
      {StringToAtom {ReplaceExtension P E}}
   end

   PLATFORM = {Property.get 'platform.name'}
   fun {MaybeAddPlatform P}
      if {ExtensionAtom P}=='so' then
	 {ToString {ToString P}#'-'#PLATFORM}
      else P end
   end

   fun {ToCacheRaw P}
      {URL.toVirtualStringExtended {ToURL P} FULL_RAW_CACHE}
   end
   fun {ToCacheURL P} {ToURL {ToCacheRaw P}} end
   fun {ToCache    P} {ToString {ToCacheRaw P}} end

   fun {Stat P} {OS.stat {ToString P}} end
   fun {SafeStat P} try {Stat P} catch _ then NOT_FOUND end end

   fun {Dir P}
      {Filter {OS.getDir {ToString P}} NotIsDot}
   end
   Ls=Dir

   fun {Exists P} {SafeStat P}.type\='unknown' end
   fun {IsDir  P} {SafeStat P}.type=='dir' end
   fun {IsFile P} {SafeStat P}.type=='reg' end

   fun {Unresolve Base Path}
      B={Expand {ToBase Base}}
      P={Expand Path}
   in
      if {IsPrefix B P} then {Append B $ P} else P end
   end
   fun {UnresolveURL Base Path}
      {ToURL {Unresolve Base Path}}
   end
   fun {UnresolveAtom Base Path}
      {StringToAtom {Unresolve Base Path}}
   end
   
   fun {IsAncestor Base Path}
      B={Expand {ToBase Base}}
      P={Expand Path}
   in
      {IsPrefix B P}
   end

   %% modifications to the file system

   proc {MakeDir P}
      U = {ExpandURL {ToNonBaseURL P}}
   in
      case {SafeStat U}.type
      of 'unknown' then {CreateDir {ToString U}}
      [] 'dir'     then skip
      else {Exception.raiseError path(mkdir({ToAtom U}))} end
   end

   proc {MakeDirRec P}
      U = {ExpandURL {ToNonBaseURL P}}
   in
      case {CondSelect U path nil}
      of nil   then skip
      [] [nil] then skip
      else {MakeDirRec {DirnameURL U}} {MakeDir U} end
   end

   proc {CreateDir P} {Shell.executeCommand [mkdir {Expand {ToNonBaseURL P}}]} end

   proc {Remove P}
      U = {ExpandURL {ToNonBaseURL P}}
   in
      if     {IsFile U} then {OS.unlink {ToString U}}
      elseif {IsDir  U} then {RemoveDir {ToString U}}
      else {Exception.raiseError path(remove({ToString U}))} end
   end

   proc {RemoveDir P}
      S = {Expand {ToNonBaseURL P}}
   in
      {Shell.executeCommand [rmdir S]}
   end

   proc {RemoveRec P}
      U = {ExpandURL {ToNonBaseURL P}}
   in
      if     {IsFile U} then {OS.unlink {ToString U}}
      elseif {IsDir  U} then
	 for E in {Dir U} do {RemoveRec {Resolve U E}} end
	 {RemoveDir {ToString U}}
      else {Exception.raiseError path(remove({ToString U}))} end
   end

   proc {CopyFile P1 P2}
      {Shell.executeCommand [cp {Expand P1} {Expand P2}]}
   end

   fun {IsBasename F}
      U={ToURL F}
   in
      {IsRelative U} andthen
      case {CondSelect U path unit}
      of [S] then S\=nil
      else false end
   end
end

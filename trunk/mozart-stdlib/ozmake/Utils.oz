functor
export
   SlurpFile
   CompileFile
   Union Diff
   DateParse DateLess DateToString DateCurrentToString
   DateCurrentToAtom DateCurrent DateToAtom DateToUserVS
   IsMogulID IsMogulRootID CleanMogulID
   NewStack NewStackFromList
   ListToVS
   ReadTextDB
   WriteTextDB
   HaveGNUC
   MogulToFilename MogulToPackagename MogulToRelative
   VersionToInts
   VersionFromInts
   IsVersion
   VersionCompare
   AuthorOK
   ToRecord
import
   Open Compiler OS URL
   Path at 'Path.ozf'
prepare
   MONTH = o('Jan' 'Feb' 'Mar' 'Apr' 'May' 'Jun' 'Jul' 'Aug' 'Sep' 'Oct' 'Nov' 'Dec')
   ZERO = "0"
   
   fun {DateParse VS}
      try
	 YEAR MONTH DAY HOUR MIN SEC
	 Year Month Day Hour Min Sec
      in
	 case {String.tokens {VirtualString.toString VS} &-}
	 of [Y M D H] then YEAR=Y MONTH=M DAY=D
	    case {String.tokens H &:}
	    of [H M S] then HOUR=H MIN=M SEC=S
	    [] [H M]   then HOUR=H MIN=M SEC=ZERO
	    end
	 [] [Y M D] then YEAR=Y MONTH=M DAY=D HOUR=MIN=SEC=ZERO
	 end
	 Year  = {StringToInt YEAR}
	 Month = {StringToInt MONTH}
	 Day   = {StringToInt DAY}
	 Hour  = {StringToInt HOUR}
	 Min   = {StringToInt MIN}
	 Sec   = {StringToInt SEC}
	 %% quick sanity check
	 if Year <0 orelse
	    Month<1 orelse Month>12 orelse
	    Day  <1 orelse Day  >31 orelse
	    Hour <0 orelse Hour >23 orelse
	    Min  <0 orelse Min  >59 orelse
	    Sec  <0 orelse Sec  >59
	 then raise unit end end
	 date(year  : Year
	      mon   : Month
	      mDay  : Day
	      hour  : Hour
	      min   : Min
	      sec   : Sec)
      catch _ then unit end
   end

   fun {DateLess D1 D2}
      D1.year < D2.year orelse
      (D1.year == D2.year andthen
       (D1.mon < D2.mon orelse
	(D1.mon == D2.mon andthen
	 (D1.mDay < D2.mDay orelse
	  (D1.mDay == D2.mDay andthen
	   (D1.hour < D2.hour orelse
	    (D1.hour == D2.hour andthen
	     (D1.min < D2.min orelse
	      (D1.min == D2.min andthen
	       (D1.sec < D2.sec))))))))))
   end

   fun {DateToString D}
      {VirtualString.toString
       D.year#'-'#D.mon#'-'#D.mDay#'-'#D.hour#':'#D.min#':'#D.sec}
   end

   fun {DateToAtom D} {StringToAtom {DateToString D}} end

   fun {DateToUserVS Date}
      YY = Date.year
      MM = {CondSelect MONTH Date.mon '???'}
      DD = Date.mDay
      H  = Date.hour
      M  = Date.min
      S  = Date.sec
      T  = if S==0 then
	      if M==0 andthen H==0 then nil
	      else ' ('#H#':'#M#')' end
	   else ' ('#H#':'#M#':'#S#')' end
   in
      DD#' '#MM#' '#YY#T
   end

   CTRL_A = 1
   

   VS2S = VirtualString.toString
   TOKS = String.tokens
   fun {VersionToInts S}
      {Map {TOKS {VS2S S} &.} StringToInt}
   end
   local
      fun {Conc VS I}
	 if VS==nil then I else VS#'.' end
      end
      fun {IsPos I} I>=0 end
      fun {Cmp L1 L2}
	 case L1
	 of nil then
	    case L2
	    of nil then eq
	    else lt end
	 [] I1|L1 then
	    case L2
	    of nil then gt
	    [] I2|L2 then
	       if I1==I2 then {Cmp L1 L2}
	       elseif I1>I2 then gt
	       else lt end
	    end
	 end
      end
   in
      fun {VersionFromInts L}
	 {VS2S {FoldL L Conc nil}}
      end
      fun {IsVersion S}
	 try L={VersionToInts S} in
	    L\=nil andthen {All L IsPos}
	 catch _ then false end
      end
      fun {VersionCompare S1 S2}
	 {Cmp
	  {VersionToInts S1}
	  {VersionToInts S2}}
      end
   end

define

   fun {SlurpFile F}
      O={New Open.file init(url:{Path.toURL F})}
   in
      try {ByteString.make {O read(list:$ size:all)}}
      finally
	 try {O close} catch _ then skip end
      end
   end

   proc {CompileFile F Debug R}
      E = {New Compiler.engine init}
      I = {New Compiler.interface init(E)}
   in
      {E enqueue(setSwitch(threadedqueries false))}
      {E enqueue(setSwitch(expression true))}
      {E enqueue(setSwitch(feedtoemulator true))}
      if Debug then
	 {E enqueue(setSwitch(controlflowinfo true))}
	 {E enqueue(setSwitch(staticvarnames true))}
      end
      {E enqueue(feedFile(F return(result:R)))}
      {I sync}
      if {I hasErrors($)} then
	 L={I getMessages($)}
	 LL={Map L fun {$ M}
		      case M of error(...) then message(M unit) else M end
		   end}
	 VS={I formatMessages(LL $)}
      in
	 raise ozmake(compiling:F VS) end
      end
   end

   fun {Union L1 L2}
      D={NewDictionary}
   in
      for F in L1 do A={Path.toAtom F} in D.A := true end
      for F in L2 do A={Path.toAtom F} in D.A := true end
      {Dictionary.keys D}
   end

   fun {Diff L1 L2}
      for X in L1 collect:Collect do
	 if {Not {Member X L2}} then {Collect X} end
      end
   end

   fun {DateCurrent}
      D={OS.localTime}
      Y=D.year+1900
      M=D.mon+1
   in
      date(
	 year : Y
	 mon  : M
	 mDay : D.mDay
	 hour : D.hour
	 min  : D.min
	 sec  : D.sec)
   end

   fun {DateCurrentToString}
      R = {DateCurrent}
   in
      {VirtualString.toString
       R.year#'-'#R.mon#'-'#R.mDay#'-'#R.hour#':'#R.min#':'#R.sec}
   end

   fun {DateCurrentToAtom} {StringToAtom {DateCurrentToString}} end

   fun {IsMogulID F}
      case {CondSelect {URL.make F} scheme unit}
      of "mogul" then true
      else false end
   end

   fun {IsMogulRootID F}
      U = {URL.make F}
   in
      case {CondSelect U scheme unit}#{CondSelect U path unit}
      of "mogul"#[_] then true
      else false end
   end

   fun {NewStack} {StackPack {NewCell nil}} end
   fun {NewStackFromList L} {StackPack {NewCell L}} end
   fun {StackPack C}
      proc {Push X} L in {Exchange C L X|L} end
      fun  {Pop} Old New in
	 {Exchange C Old New}
	 case Old of H|T then New=T H
	 else Old=New raise empty end end
      end
      fun {IsEmpty} {Access C}==nil end
      fun {ToList} {Access C} end
   in
      stack(push    : Push
	    pop     : Pop
	    isEmpty : IsEmpty
	    toList  : ToList)
   end

   fun {ListToVS L}
      {FoldL L fun {$ VS S}
		  if VS==nil then S
		  elseif S==nil then VS
		  else VS#' '#S end end nil}
   end

   class TextFile from Open.file Open.text end

   fun {ReadTextDB F}
      O = {New TextFile init(name:F)}
   in
      try {ReadNextEntry O}
      finally
	 try {O close} catch _ then skip end
      end
   end

   fun {ReadNextEntry O}
      C={O getC($)}
   in
      if C==false then nil
      else {Compiler.virtualStringToValue (C|{ReadEntry O})}|{ReadNextEntry O} end
   end

   fun {ReadEntry O}
      C={O getC($)}
   in
      if C==CTRL_A then nil else C|{ReadEntry O} end
   end

   proc {WriteTextDB L F}
      O = {New Open.file init(name:F flags:[write create truncate])}
   in
      try
	 for X in L do
	    {O write(vs:{Value.toVirtualString X 1000000 1000000}#[CTRL_A])}
	 end
      finally
	 try {O close} catch _ then skip end
      end
   end

   local
      fun {HasYes L}
	 case L
	 of &Y|&E|&S|_ then true
	 [] _|L then {HasYes L}
	 else false end
      end
   in
      fun {HaveGNUC GetTmpnam OZTOOL}
	 F={GetTmpnam}#'.h'
      in
	 try
	    O={New Open.file init(name:F flags:[write create truncate])}
	    {O write(vs:"#ifdef __GNUC__\nYES\n#endif")}
	    {O close}
	    P={New Open.pipe init(cmd:OZTOOL args:['c++' '-E' F])}
	    L={P read(list:$ size:all)}
	 in
	    {HasYes L}
	 catch _ then false
	 finally
	    try {OS.unlink F} catch _ then skip end
	 end
      end
   end

   local
      fun {SlashToDash L}
	 case L
	 of &/|L then &-|{SlashToDash L}
	 []  H|L then  H|{SlashToDash L}
	 []  nil then nil end
      end
   in
      fun {MogulToFilename MOG}
	 U1={Path.toURL MOG}
	 U2={Adjoin U1 url(scheme:unit device:unit absolute:false)}
      in
	 {SlashToDash {Path.toCache U2}}
      end
   end

   fun {MogulToPackagename MOG}
      {Append {MogulToFilename MOG} ".pkg"}
   end

   fun {MogulToRelative MOG}
      {Path.toString
       {Adjoin
	{Path.toURL MOG}
	url(scheme:unit device:unit absolute:false)}}
   end

   fun {AuthorOK S}
      {IsMogulID S} orelse
      local L={VS2S S} in
	 {Not {Member &: L} orelse {Member &/ L}}
      end
   end

   fun {ToRecord D}
      if {IsDictionary D} then
	 {Dictionary.toRecord o D}
      elseif {IsRecord D} then D
      else
	 raise ozmake(toRecord:D) end
      end
   end

   fun {CleanMogulID MOG}
      %% normalize MOGUL ID, i.e. no trailing slash
      {Path.toNonBaseAtom MOG}
   end
      
end
functor
export
   SlurpFile
   CompileFile
   Union
   DateParse DateLess DateToString DateCurrentToString
   DateCurrentToAtom DateCurrent DateToAtom DateToUserVS
   IsMogulID
   NewStack
   ListToVS
   ReadTextDB
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


   fun {DateCurrent}
      D={OS.localTime}
   in
      {AdjoinAt D year 1900+D.year}
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

   fun {NewStack}
      C={NewCell nil}
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
      {FoldL L fun {$ VS S} if VS==nil then S else VS#' '#S end end nil}
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

end
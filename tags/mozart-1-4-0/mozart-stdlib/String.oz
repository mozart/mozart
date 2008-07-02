functor
export
   Make Length
   ToInt ToAtom ToFloat
   Capitalize
   Split SplitAtMost Join
   ToLower ToUpper
   Lstrip Rstrip Strip
   Replace ReplaceAtMost
prepare
   CharToUpper = Char.toUpper
   CharToLower = Char.toLower
   CharIsSpace = Char.isSpace
   DropWhile = List.dropWhile

   Make = VirtualString.toString

   ToInt = StringToInt
   ToAtom = StringToAtom
   ToFloat = StringToFloat

   fun {Capitalize S}
      case S
      of H|T then {CharToUpper H}|T
      else S end
   end

   fun {SplitStart S Next Max}
      if S==nil then nil
      elseif Max==0 then [S]
      else Prefix Suffix in
	 {Next S Prefix Suffix}
	 Prefix|if Suffix==unit then nil else
		   {SplitMore Suffix Next Max-1}
		end
      end
   end

   fun {SplitMore S Next Max}
      if Max==0 orelse S==nil then [S]
      else Prefix Suffix in
	 {Next S Prefix Suffix}
	 Prefix|if Suffix==unit then nil else
		   {SplitMore Suffix Next Max-1}
		end
      end
   end

   fun {Split S Sep} {SplitStart S {NextSplitter Sep} ~1} end
   fun {SplitAtMost S Sep Max} {SplitStart S {NextSplitter Sep} Max} end

   proc {NextSplitWS S Prefix Suffix}
      case S
      of nil then Prefix=nil Suffix=unit
      [] H|T then
	 if {CharIsSpace H} then
	    Prefix=nil {DropWhile T CharIsSpace Suffix}
	 else Prefix2 in
	    Prefix=(H|Prefix2)
	    {NextSplitWS T Prefix2 Suffix}
	 end
      end
   end

   fun {WithPrefix SEP S}
      case SEP
      of nil then S
      [] H|SEP then
	 case S
	 of !H|S then {WithPrefix SEP S}
	 else unit end
      end
   end

   proc {NextSplitSEP S SEP Prefix Suffix}
      case S
      of nil then Prefix=nil Suffix=unit
      elsecase {WithPrefix SEP S}
      of unit then
	 case S
	 of H|T then Prefix2 in
	    Prefix=(H|Prefix2)
	    {NextSplitSEP T SEP Prefix2 Suffix}
	 end
      [] S then Prefix=nil Suffix=S
      end
   end

   proc {NextSplitNULL S Prefix Suffix}
      case S
      of nil then Prefix=nil Suffix=unit
      [] H|T then Prefix=[H] Suffix=T
      end
   end

   fun {NextSplitter Sep}
      case Sep
      of unit then NextSplitWS
      [] nil  then NextSplitNULL
      else
	 proc {$ S Prefix Suffix}
	    {NextSplitSEP S Sep Prefix Suffix}
	 end
      end
   end

   fun {Join L Sep}
      if L==nil then nil else
	 {FoldR L
	  fun {$ S Accu}
	     if Accu==unit then S else
		{Append S {Append Sep Accu}}
	     end
	  end unit}
      end
   end

   fun {ToUpper S} {Map S CharToUpper} end
   fun {ToLower S} {Map S CharToLower} end

   fun {Lstrip S Chars}
      {DropWhile S
       if Chars==unit then CharIsSpace else
	  fun {$ C} {Member C Chars} end
       end}
   end

   fun {Rstrip S Chars}
      {Reverse {Lstrip {Reverse S} Chars}}
   end

   fun {Strip S Chars}
      {Rstrip {Lstrip S Chars} Chars}
   end

   fun {Replace S Old New}
      {Join {Split S Old} New}
   end

   fun {ReplaceAtMost S Old New Max}
      {Join {SplitAtMost S Old Max} New}
   end
end
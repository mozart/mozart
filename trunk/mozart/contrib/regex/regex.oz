%%% -*-oz-*-
%%% Copyright © by Denys Duchier, Dec 1997, Universität des Saarlandes

local
   CFlagTable = table(extended:{BitString.make 4 [0]}
		      icase   :{BitString.make 4 [1]}
		      newline :{BitString.make 4 [2]}
		      nosub   :{BitString.make 4 [3]})
   CFlagNone  = {BitString.make 4 nil}
   fun {EncodeCFlag Spec}
      if {IsAtom Spec} then CFlagTable.Spec
      else {FoldL Spec
	    fun {$ B X} {BitString.disj B CFlagTable.X} end CFlagNone}
      end
   end
   EFlagTable = table(notbol:{BitString.make 2 [0]}
		      noteol:{BitString.make 2 [1]})
   EFlagNone  = {BitString.make 2 nil}
   fun {EncodeEFlag Spec}
      if {IsAtom Spec} then EFlagTable.Spec
      else {FoldL Spec
	    fun {$ B X} {BitString.disj B EFlagTable.X} end EFlagNone}
      end
   end
in
   functor
   import
      REGEX(compile	: COMPILE
	    execute	: EXECUTE
	    free	: FREE
	    flags	: FLAGS
	    is		: Is
	   ) at 'regex.so{native}'
      Finalize(register:Register)
   export
      CFlags EFlags
      Is Make Search Compile Execute Group Groups
      AllMatches ForAll FoldR FoldL Map Split
   define
      flag(EXTENDED ICASE NEWLINE NOSUB NOTBOL NOTEOL)={FLAGS}

      fun {CFlagToInt Spec}
	 B = {EncodeCFlag Spec}
      in
	 if {BitString.get B 0} then EXTENDED else 0 end +
	 if {BitString.get B 1} then ICASE    else 0 end +
	 if {BitString.get B 2} then NEWLINE  else 0 end +
	 if {BitString.get B 3} then NOSUB    else 0 end
      end

      CFlagsInt  = {NewCell EXTENDED+NEWLINE}
      CFlagsSpec = {NewCell [extended newline]}

      fun  {GetCFlags} {Access CFlagsSpec} end
      proc {SetCFlags Spec}
	 {Assign CFlagsInt  {CFlagToInt Spec}}
	 {Assign CFlagsSpec Spec}
      end

      CFlags = cflags(get:GetCFlags set:SetCFlags)

      fun {EFlagToInt Spec}
	 B = {EncodeEFlag Spec}
      in
	 if {BitString.get B 0} then NOTBOL else 0 end +
	 if {BitString.get B 1} then NOTEOL else 0 end
      end

      EFlagsInt  = {NewCell 0}
      EFlagsSpec = {NewCell nil}

      fun  {GetEFlags} {Access EFlagsSpec} end
      proc {SetEFlags Spec}
	 {Assign EFlagsInt {EFlagToInt Spec}}
	 {Assign EFlagsSpec Spec}
      end

      EFlags = eflags(get:GetEFlags set:SetEFlags)
   
      proc {Compile SRC FLAGS RE}
	 {COMPILE SRC {CFlagToInt FLAGS} RE}
	 {Register RE FREE}
      end
      proc {Make SRC RE}
	 {Compile SRC {Access CFlagsInt} RE}
      end
      fun {Coerce RE}
	 if {Is RE} then RE else {Make RE} end
      end
      fun {Execute RE TXT IDX FLAGS}
	 {EXECUTE {Coerce RE} TXT IDX {EFlagToInt FLAGS}}
      end
      fun {Search RE TXT}
	 {EXECUTE {Coerce RE} TXT 0 {Access EFlagsInt}}
      end
      %%
      fun {Group N Match Txt}
	 BEG#END = Match.N
	 TXT = {ByteString.make Txt}
      in
	 {ByteString.slice TXT BEG END}
      end
      %% omit group 0 which is the whole match
      fun {Groups Match Txt}
	 _|L = {Arity Match}
      in
	 {List.toTuple group
	  {List.map L fun {$ N} {Group N Match Txt} end}}
      end
      %%
      %% RE should not be anchored because we assume that EXECUTE will
      %% find the next match if there is one.
      %%
      fun {AllMatches Re TXT}
	 RE  = {Coerce Re}
	 END = {VirtualString.length TXT}
	 DEF = {Access EFlagsInt}
	 fun {Loop I}
	    if I>=END then nil else
	       M = {EXECUTE RE TXT I DEF}
	    in
	       if M==false then nil else J = M.0.2 in
		  %% ensure progress over 0-length matches
		  M|{Loop if I==J then I+1 else J end}
	       end
	    end
	 end
      in {Loop 0} end
      %%
      %% execute PROC for every non-overlapping match of RE in TXT
      %% proc takes 1 args the match record
      %%
      proc {ForAll RE TXT PROC}
	 {List.forAll {AllMatches RE TXT} PROC}
      end
      %%
      %% FUN takes the match record as input arg
      %%
      fun {FoldR RE TXT FUN INIT}
	 {List.foldR {AllMatches RE TXT} FUN INIT}
      end
      %%
      fun {FoldL RE TXT FUN INIT}
	 {List.foldL {AllMatches RE TXT} FUN INIT}
      end
      %%
      fun {Map RE TXT FUN}
	 {List.map {AllMatches RE TXT} FUN}
      end
      %%
      %% split input at every occurrence of a regex
      %% the regex represents the separator and the corresponding
      %% matching text is discarded.
      %%
      fun {Split Re Txt}
	 RE  = {Coerce Re}
	 DEF = {Access EFlagsInt}
	 TXT = {ByteString.make Txt}
	 END = {ByteString.length TXT}
	 fun {Loop I}
	    if I>=END then nil else
	       M = {EXECUTE RE TXT I DEF}
	    in
	       if M==false then [{ByteString.slice TXT I END}]
	       else K#J = M.0 in
		  {ByteString.slice TXT I K}
		  |{Loop if I==J then I+1 else J end}
	       end
	    end
	 end
      in {Loop 0} end
   end
end

functor
import
   Regex at 'x-oz://contrib/regex'
export
   Split Strip Split1 SplitWords ToLower ToUpper MakeBS Capitalize Join
   Capwords EmptyToUnit HtmlQuote
define
   %%
   %% split TEXT at all occurrences of PAT
   %%
   fun {Split TEXT PAT}
      {Regex.split PAT TEXT}
   end
   RE_WS = {Regex.make '[[:space:]]+'}
   fun {SplitWords TEXT}
      {Split TEXT RE_WS}
   end
   %%
   %% split TEXT at 1st occurrence of PAT
   %%
   fun {Split1 TEXT PAT}
      case {Regex.search PAT TEXT}
      of false then [TEXT]
      [] M then
	 BS = {MakeBS TEXT}
	 I#J = M.0
      in
	 [{ByteString.slice BS 0 I}
	  {ByteString.slice BS J {ByteString.width BS}}]
      end
   end
   %%
   fun {MakeBS S}
      if {ByteString.is S} then S else {ByteString.make S} end
   end
   %%
   local
      %% the following doesn't work on RedHat 8.0 when the TXT contains only spaces
      %% PAT = {Regex.compile '^[[:space:]]*(.*[^[:space:]])[[:space:]]*$' [extended]}
      %% in any case, it would not have stripped in that case
      PREFIX = {Regex.compile '^[[:space:]]+' [extended]}
      SUFFIX = {Regex.compile '[[:space:]]+$' [extended]}
   in
      fun {Strip TXT}
	 S = {MakeBS TXT}
	 S1 =
	 case {Regex.search PREFIX S}
	 of false then S
	 [] M then {ByteString.slice S M.0.2 {ByteString.width S}}
	 end
      in
	 case {Regex.search SUFFIX S1}
	 of false then S1
	 [] M then {ByteString.slice S1 0 M.0.1}
	 end
      end
   end
   %%
   fun {ToLower TXT}
      {Map {VirtualString.toString TXT} Char.toLower}
   end
   fun {ToUpper TXT}
      {Map {VirtualString.toString TXT} Char.toUpper}
   end
   %%
   fun {Join Words Sep}
      case Words
      of nil then nil
      [] [H] then H
      [] H|T then H#Sep#{Join T Sep}
      end
   end
   %%
   fun {Capitalize Word}
      case {ToLower Word}
      of nil then nil
      [] H|T then {Char.toUpper H}|T end
   end
   fun {Capwords S}
      {Join {Map {SplitWords S} Capitalize} ' '}
   end
   %%
   fun {EmptyToUnit S}
      if S==unit orelse {VirtualString.length S}==0 then unit
      else S end
   end
   %%
   local
      fun {Loop S}
	 case S of nil then nil
	 [] &&|T then &&|&a|&m|&p|&;|{Loop T}
	 [] &<|T then &&|&l|&t|&;   |{Loop T}
	 [] &>|T then &&|&g|&t|&;   |{Loop T}
	 []  H|T then  H            |{Loop T}
	 end
      end
   in
      fun {HtmlQuote S}
	 {Loop {VirtualString.toString S}}
      end
   end
end

%%% -*-oz-*-
%%%
%%% This file implements a compiler for xsl match patterns
%%%
functor
import
   Search(
      disj	: OR
      seq	: SEQ
      ) at 'XSL-SEARCH.ozf'
   Element(
      element	: ELEMENT
      ) at 'XSL-ELEMENT.ozf'
export
   PatternMatcher
define
   fun {TRUE Node Stack Info No} true end
   fun {FALSE} false end
   %%
   fun {PatternMatcher Pattern}
      Matcher = {MatchPattern Pattern}
   in
      fun {$ Node Stack Info}
	 {Matcher Node Stack Info TRUE FALSE}
      end
   end
   %%
   fun {MatchPattern Pat}
      case Pat of pattern(Alts) then
	 L = {Map Alts MatchAlt}
      in
	 {OR L}
      end
   end
   %%
   fun {MatchAlt A}
      case A of alt(L) then
	 {SEQ {Map {Reverse L} MatchOne}}
      end
   end
   %%
   fun {MatchOne X}
      case X
      of     down               then MatchDown
      elseof downMany           then MatchDownMany
      elseof element(Type Qual) then {ELEMENT Type Qual}
      elseof id(ID)             then {MatchId ID}
      elseof root               then MatchRoot
      else raise illegalInMatchPattern(X) end
      end
   end
   %%
   %% since we are matching, we are proceeding from the end of
   %% the pattern to the front; therefore `down' must actually
   %% go up the tree.
   %%
   fun {MatchDown Node Stack Info Yes No}
      case Stack of H|T then
	 {Yes H T Info No}
      else {No} end
   end
   %%
   fun {MatchDownMany Node Stack Info Yes No}
      case Stack of nil then {No}
      [] H|T then
	 {Yes H T Info
	  fun {$}
	     {MatchDownMany H T Info Yes No}
	  end}
      end
   end
   %%
   fun {MatchId ID}
      fun {$ Node Stack Info Yes No}
	 if {Info.getid Node}==ID then
	    {Yes Node Stack Info Yes No}
	 else {No} end
      end
   end
   %%
   fun {MatchRoot Node Stack Info Yes No}
      if Stack==nil then {Yes Node Stack Info No} else {No} end
   end
end

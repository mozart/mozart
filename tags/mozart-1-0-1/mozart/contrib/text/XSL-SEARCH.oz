%%% -*-oz-*-
%%%
%%% This file implements general search combinators and meta combinators
%%% for xsl match/select patterns.  A search combinator is a function
%%%
%%%	fun {$ Node Stack Info Yes No} ... end
%%%
%%% where Node is the current node at which the search is currently
%%% located, Stack is the stack of its ancestors, Info gives us access
%%% to stylesheet and document dependent information, Yes is the
%%% success continuation and No is the failure continuation.
%%%
%%% A success continuation is a function:
%%%
%%%	fun {$ Node Stack Info No} ... end
%%%
%%% i.e. almost like a search combinator, but without the success
%%% continuation argument.  A failure continuation is a function:
%%%
%%%	fun {$} ... end
%%%
functor
export
   Never Always Seq Conj Disj Not
define
   %%
   %% Never: search combinator that immediately fails
   %%
   fun {Never Node Stack Info Yes No} {No} end
   %%
   %% Always: search combinator that immediately succeeds
   %%
   fun {Always Node Stack Info Yes No} {Yes Node Stack Info No} end
   %%
   %% {Seq L} combines search combinators sequentially.  each
   %% ones is used as the success continuation of the previous one.
   %%
   fun {Seq L}
      case L of nil then Always
      elseof [S] then S
      elseof S1|T then S2={Seq T} in
	 fun {$ Node Stack Info Yes No}
	    {S1 Node Stack Info
	     fun {$ Node Stack Info No}
		{S2 Node Stack Info Yes No}
	     end No}
	 end
      end
   end
   %%
   %% {Conj L} combines search combinators conjunctively.  This
   %% differs from {Seq L} only in that here, each search combinator
   %% is applied to the same node.  With Seq, it is possible that
   %% one combinator passes on a different node to its continuation.
   %%
   fun {Conj L}
      case L of nil then Always
      elseof [S] then S
      elseof S1|T then S2={Conj T} in
	 fun {$ Node Stack Info Yes No}
	    {S1 Node Stack Info
	     fun {$ _ _ _ No}
		{S2 Node Stack Info Yes No}
	     end No}
	 end
      end
   end
   %%
   %% {Disj L} combines search combinators disjunctively.
   %%
   fun {Disj L}
      case L of nil then Never
      elseof [S] then S
      elseof S1|T then S2={Disj T} in
	 fun {$ Node Stack Info Yes No}
	    {S1 Node Stack Info Yes
	     fun {$}
		{S2 Node Stack Info Yes No}
	     end}
	 end
      end
   end
   %%
   %% {Not P} negates a search combinator
   %%
   fun {Not P}
      fun {$ Node Stack Info Yes No}
	 {P Node Stack Info
	  fun {$ _ _ _ _} {No} end
	  fun {$} {Yes Node Stack Info No} end}
      end
   end
end

%%% -*-oz-*-
%%%
%%% This file implements a compiler for xsl select patterns
%%%
functor
import
   Search(
      disj      : OR
      seq       : SEQ
      ) at 'XSL-SEARCH.ozf'
   Element(
      element   : ELEMENT
      withQualification : WITHQUALIFICATION
      ) at 'XSL-ELEMENT.ozf'
   Matching(
      patternMatcher : MATCHER
      ) at 'XSL-MATCHING.ozf'
export
   PatternSelector
define
   fun {PatternSelector Pattern}
      Selector = {SelectPattern Pattern}
   in
      fun {$ Node Stack Info}
         Accu = {NewCell nil}
         %%
         %% the ultimate success continuation accumulates
         %% the selected node and then backtracks to search
         %% for the next solution, etc...
         %%
         fun {MORE Node Stack Info Yes No}
            Selected
         in
            {Exchange Accu Selected Node|Selected}
            {No}
         end
         %%
         %% the ultimate failure continuation simply returns
         %% the accumulated selections.
         %%
         fun {NOMORE}
            {Reverse {Access Accu}}
         end
      in
         {Selector Node Stack Info MORE NOMORE}
      end
   end
   %%
   fun {SelectPattern Pat}
      case Pat of pattern(Alts) then
         L = {Map Alts SelectAlt}
      in
         {OR L}
      end
   end
   %%
   fun {SelectAlt A}
      case A of alt(L) then
         {SEQ {Map L SelectOne}}
      end
   end
   %%
   fun {SelectOne X}
      case X
      of     down               then SelectDown
      elseof downMany           then SelectDownMany
      elseof up(Quals)          then {SelectUp Quals}
      elseof element(Type Qual) then {ELEMENT Type Qual}
      elseof root               then SelectRoot
      elseof id(ID)             then {SelectId ID}
      elseof here(QUAL)         then {WITHQUALIFICATION QUAL}
      elseof ancestor(PAT)      then {SelectAncestor PAT}
      elseof attribute(ATTR)    then {SelectAttribute ATTR}
      end
   end
   %%
   fun {SelectDown Node Stack Info Yes No}
      {SelectDownLoop Node.content Node|Stack Info Yes No}
   end
   fun {SelectDownLoop Siblings Stack Info Yes No}
      case Siblings of nil then {No}
      [] H|T then
         {Yes H Stack Info Yes
          fun {$} {SelectDownLoop T Stack Info Yes No} end}
      end
   end
   %%
   fun {SelectDownMany Node Stack Info Yes No}
      {SelectDownManyLoop Node.content Node|Stack Info Yes No}
   end
   fun {SelectDownManyLoop Siblings Stack Info Yes No}
      case Siblings of nil then {No}
      [] H|T then
         %% try sibling H
         {Yes H Stack Info Yes
          fun {$}
             %% try desdendents of H
             {SelectDownMany H Stack Info Yes
              fun {$}
                 %% try other siblings of H
                 {SelectDownManyLoop T Stack Info Yes No}
              end}
          end}
      end
   end
   %%
   fun {SelectUp Quals}
      L = {Map Quals SelectUpOne}
   in {SEQ L} end
   %%
   fun {SelectUpOne Q}
      P = {WITHQUALIFICATION Q}
   in
      fun {$ Node Stack Info Yes No}
         case Stack of nil then {No}
         [] H|T then {P H T Info Yes No} end
      end
   end
   %%
   fun {SelectRoot Node Stack Info Yes No}
      {Yes if Stack==nil then Node
           else {List.last Stack} end
       nil Info Yes No}
   end
   %%
   fun {SelectId ID}
      fun {$ Node Stack Info Yes No}
         try
            {FindId ID Info
             if Stack==nil then Node
             else {List.last Stack} end nil}
            {No}
         catch idFound(Node Stack) then
            {Yes Node Stack Info No}
         end
      end
   end
   %%
   proc {FindId ID Info Node Stack}
      if {Info.getid Node}==ID then
         raise idFound(Node Stack) end
      else Stack2 = Node|Stack in
         {ForAll Node.content
          proc {$ N}
             {FindId ID Info N Stack2}
          end}
      end
   end
   %%
   fun {SelectAncestor PAT}
      M = {MATCHER PAT}
   in
      fun {$ Node Stack Info Yes No}
         {AncestorLoop Stack Info Yes No M}
      end
   end
   %%
   fun {AncestorLoop Stack Info Yes No M}
      case Stack of nil then {No}
      [] H|T then
         {M H T Info Yes
          fun {$} {AncestorLoop T Info Yes No M} end}
      end
   end
   %%
   fun {SelectAttribute ATTR}
      fun {$ Node Stack Info Yes No}
         A = {CondSelect {CondSelect Node attribute unit} ATTR unit}
      in
         if A==unit then {No} else {Yes A Node|Stack Info No} end
      end
   end
end

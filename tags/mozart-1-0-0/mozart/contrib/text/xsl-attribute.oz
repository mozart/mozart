%%% -*-oz-gump-*-
\switch +gump
functor
import
   GumpScanner('class':GS)
export
   'class' : XSL_Attribute_Parser
define
   \gumpscannerprefix 'XSL_Attribute_Scanner_'
   scanner XSL_Attribute_Scanner from GS
      meth scanAll($) X Y in
	 GS,getToken(X Y)
	 if X=='EOF' then nil
	 else X(Y)|XSL_Attribute_Scanner,scanAll($) end
      end
      lex id = <[a-zA-Z_][a-zA-Z0-9_.:-]*> end
      lex <[ \t\n]+> skip end
      lex <{id}> A in
	 GS,getAtom(A)
	 GS,putToken(ident A)
      end
      lex <\"[^\"]*\"|\'[^\']*\'> S in
	 GS,getString(_|S)
	 GS,putToken(string {List.take S {Length S}-1})
      end
      lex <".."|"."|"*"|"("|")"|"//"|"/"|"["|"]"|","|"="|"|"> A in
	 GS,getAtom(A)
	 GS,putToken(op A)
      end
      lex <.> raise xsl(scanFailed GS,getString($)) end end
      lex <<EOF>> GS,putToken1('EOF') end
   end
   class XSL_Attribute_Parser
      prop locking
      attr Scanner
      meth init Scanner <- {New XSL_Attribute_Scanner init} end
      meth parsePattern(VS $)
	 lock
	    {@Scanner scanVirtualString(VS)}
	    case {Pattern {@Scanner scanAll($)}} of P#nil then P end
	 end
      end
      meth parseStringExpression(VS $)
	 lock
	    {@Scanner scanVirtualString(VS)}
	    {StringExp {@Scanner scanAll($)}}
	 end
      end
   end
   %%
   fun {Pattern Tokens}
      Alts#Tokens2 = {OrPatterns Tokens}
   in pattern(Alts)#Tokens2 end
   fun {OrPatterns Tokens}
      A #Tokens2 = {Alt Tokens}
      As#Tokens3 =
      case Tokens2 of op('|')|Tokens then {OrPatterns Tokens}
      else nil#Tokens2 end
   in (A|As)#Tokens3 end
   fun {Alt Tokens}
      F#Tokens2 = {From Tokens}
   in case F
      of unit then L#Tokens = {NodePatterns Tokens2} in alt(L)#Tokens
      elseof root then
	 if case Tokens2 of H|_ then
	       case H of ident(_) then true
	       elseof op('*') then true
	       else false end
	    else false end
	 then L#Tokens = {NodePatterns Tokens2} in
	    alt(root|L)#Tokens
	 else alt([root])#Tokens2 end
      elsecase Tokens2 of op('/')|Tokens then
	 L#Tokens3 = {NodePatterns Tokens}
      in alt(F|down|L)#Tokens3
      elseof op('//')|Tokens then
	 L#Tokens3 = {NodePatterns Tokens}
      in alt(F|downMany|L)#Tokens3
      else alt([F])#Tokens2 end
   end
   fun {From Tokens}
      case Tokens
      of ident(id)|op('(')|ident(ID)|op(')')|Tokens then id(ID)#Tokens
      elseof ident(ancestor)|op('(')|Tokens then
	 P#(op(')')|Tokens2) = {Pattern Tokens}
	 %% we invert the pattern so that we can use it for
	 %% checking rather than for generation
      in ancestor({Invert P})#Tokens2
      elseof op('.')|Tokens then
	 L#Tokens2 = {Qualification Tokens}
      in here(L)#Tokens2
      elseof op('..')|Tokens then
	 Q #Tokens2 = {Qualification Tokens}
	 Qs#Tokens3 = {Gobble Tokens2}
      in up(Q|Qs)#Tokens3
      elseof op('/')|Tokens then root#Tokens
      else unit#Tokens end
   end
   fun {Gobble Tokens}
      case Tokens of op('/')|op('..')|Tokens then
	 Q #Tokens2 = {Qualification Tokens}
	 Qs#Tokens3 = {Gobble Tokens2}
      in (Q|Qs)#Tokens3 else nil#Tokens end
   end
   fun {NodePatterns Tokens}
      case Tokens
      of ident(attribute)|op('(')|ident(ID)|op(')')|Tokens then
	 [attribute(ID)]#Tokens
      else
	 Type#Tokens2 =
	 case Tokens of op('*')|L then any#L
	 elseof ident(ID)|L then  type(ID)#L end
	 Qual#Tokens3 = {Qualification Tokens2}
	 Path#Tokens4 = {MoreNodePatterns Tokens3}
      in
	 (element(Type Qual)|Path)#Tokens4
      end
   end
   fun {MoreNodePatterns Tokens}
      case Tokens of op('/')|Tokens then
	 L#Tokens2 = {NodePatterns Tokens}
      in (down|L)#Tokens2
      elseof op('//')|Tokens then
	 L#Tokens2 = {NodePatterns Tokens}
      in (downMany|L)#Tokens2
      else nil#Tokens end
   end
   fun {Qualification Tokens}
      case Tokens of op('[')|Tokens then
	 L#(op(']')|Tokens2) = {Qualifiers Tokens}
      in L#Tokens2 else nil#Tokens end
   end
   fun {Qualifiers Tokens}
      Q #Tokens2 =
      case Tokens
      of ident(attribute)|op('(')|ident(ID)|op(')')|Tokens then
	 case Tokens of op('=')|string(V)|Tokens then
	    attribute(ID equal(V))#Tokens
	 elseof ident(contains)|op('(')|string(V)|op(')')|Tokens then
	    attribute(ID contains(V))#Tokens
	 else
	    attribute(ID exists)#Tokens
	 end
      elseof ident(POS)|op('(')|op(')')|Tokens then
	 position(POS)#Tokens
      elseof ident(ID)|Tokens then
	 child(ID exists)#Tokens
      end
      Qs#Tokens3 =
      case Tokens2 of op(',')|Tokens then {Qualifiers Tokens}
      else nil#Tokens2 end
   in (Q|Qs)#Tokens3 end
   %%
   fun {Invert P}
      case P of pattern(L) then pattern({Map L InvertAlt}) end
   end
   fun {InvertAlt A}
      case A of alt(L) then alt({InvertPath L}) end
   end
   fun {InvertPath Path}
      %% optimization: all elements are below the root; there is
      %% no need to check that
      {Map {Reverse case Path of root|downMany|Path then Path else Path end}
       InvertOne}
   end
   fun {InvertOne X}
      case X
      of     down    then up
      elseof up      then down
      elseof down(L) then up({Reverse L})
      elseof up(L)   then down({Reverse L})
      else X end
   end
   %%
   fun {StringExp Tokens}
      case Tokens
      of [ident(constant) op('(') ident(ID) op(')')] then
	 strexp(constant(ID))
      elseof [ident(arg) op('(') ident(ID) op(')')] then
	 strexp(arg(ID))
      elsecase {Pattern Tokens} of P#nil then
	 strexp(P)
      end
   end
end

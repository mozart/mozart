%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1996-1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%   http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%%
%% The Gump Scanner Generator.
%%

local
   fun {CheckLegalFlexName X} S in
      S = {Atom.toString X}
      case S of C|Cr then
	 if C >= &A andthen C =< &Z
	    orelse C >= &a andthen C =< &z
	    orelse C == &_
	 then
	    {All Cr
	     fun {$ C}
		C >= &A andthen C =< &Z orelse
		C >= &a andthen C =< &z orelse
		C >= &0 andthen C =< &9 orelse
		C == &_ orelse C == &-
	     end}
	 else false
	 end
      else false
      end
   end

   proc {TransformScannerDescriptor D Globals Rep}
      case D of fLexicalAbbreviation(L R) then
	 {Globals
	  addGrammarSymbol({New LexicalAbbreviation init(L R)} Rep)}
      [] fLexicalRule(R IE) then
	 {Globals addLexicalRule({New LexicalRule init(R IE)})}
      [] fMode(M Ds) then Xs Ys Parents in
	 {List.partition Ds fun {$ D} {Label D} == fInheritedModes end ?Xs ?Ys}
	 Parents = {Flatten {Map Xs fun {$ fInheritedModes(Xs)} Xs end}}
	 {Globals addGrammarSymbol({New LexicalMode init(M Parents)} Rep)}
	 {ForAll Ys
	  proc {$ D} {TransformScannerDescriptor D Globals Rep} end}
	 {Globals popLexicalMode()}
      end
   end

   fun {CompileScanner Flex T Rep}
      case Flex of noLexer then noLexer
      else FlexFile = {MakeFileName T ".l"} in
	 {Rep logSubPhase('writing flex input file ...')}
	 {WriteVSFile Flex FlexFile}
	 {Rep logSubPhase('generating scanner tables ...')}
	 if {OS.system PLATFORMDIR#'/share/gump/flex -Cem '#FlexFile} \= 0
	 then
	    {Rep error(kind: 'system error'
		       msg: 'invocation of ozflex failed')}
	    stop
	 else
	    {Rep logSubPhase('compiling scanner ...')}
	    if {OS.system
		'g++ -fno-rtti -O3 '#
		'-I'#{Property.get 'oz.home'}#'/include -I'#INCLUDEDIR#
		' -c '#{MakeFileName T ".C"}#
		' -o '#{MakeFileName T ".o"}} \= 0
	    then
	       {Rep error(kind: 'system error'
			  msg: 'invocation of g++ failed')}
	       stop
	    elseif {OS.system 'ozdynld '#
		    {MakeFileName T ".o"}#' -o '#
		    {MakeFileName T ".dl"}#' -lc'} \= 0
	    then
	       {Rep error(kind: 'system error'
			  msg: 'invocation of ozdynld failed')}
	       stop
	    else continue
	    end
	 end
      end
   end

   %-----------------------------------------------------------------------
   % Global Symbol Table
   %-----------------------------------------------------------------------

   class ScannerSpecification from ClassDescriptors
      prop final
      attr
	 activeLexicalModes: nil
	 lexicalRules: nil nextLexicalRuleNumber: 0
	 grammarSymbols: unit grammarSymbolsNotAnalysed: nil
      meth init() INITIAL in
	 INITIAL = fVar('INITIAL' unit)
	 grammarSymbols <- [INITIAL#{New LexicalMode init(INITIAL nil)}]
      end
      meth popLexicalMode()
	 activeLexicalModes <- @activeLexicalModes.2
      end
      meth addLexicalRule(Entry)
	 case @activeLexicalModes of nil then
	    Entry.modeSymbol = fVar('INITIAL' unit)
	 [] Mode|_ then Entry.modeSymbol = Mode
	 end
	 Entry.ruleNumber = @nextLexicalRuleNumber
	 nextLexicalRuleNumber <- @nextLexicalRuleNumber + 1
	 lexicalRules <- Entry|@lexicalRules
      end
      meth addGrammarSymbol(Entry Rep) Symbol CurrEntry in
	 Symbol = {Entry getSymbol($)}
	 if {CheckLegalFlexName Symbol.1} then skip
	 else
	    {Rep error(coord: {CoordinatesOf Symbol}
		       kind: 'scanner generator error'
		       msg: 'illegal syntax for grammar symbol')}
	 end
	 CurrEntry = ScannerSpecification, getGrammarSymbol(Symbol $)
	 case CurrEntry of notFound then
	    case {Entry getEntryType($)} of lexicalMode then
	       {Entry addParents(@activeLexicalModes)}
	       activeLexicalModes <- Symbol|@activeLexicalModes
	    else skip
	    end
	    grammarSymbols <- (Symbol#Entry)|@grammarSymbols
	    grammarSymbolsNotAnalysed <- Entry|@grammarSymbolsNotAnalysed
	 elsecase {Entry getEntryType($)} of lexicalMode then
	    if {CurrEntry getEntryType($)} \= lexicalMode then
	       {Rep error(coord: {CoordinatesOf Symbol}
			  kind: 'scanner generator error'
			  msg: ('grammar symbol '#
				{SymbolToVirtualString Symbol}#
				' multiply defined'))}
	    else
	       {CurrEntry addParents({Entry getParents($)})}
	       {CurrEntry addParents(@activeLexicalModes)}
	    end
	 else
	    {Rep error(coord: {CoordinatesOf Symbol}
		       kind: 'scanner generator error'
		       msg: ('grammar symbol '#
			     {SymbolToVirtualString Symbol}#
			     ' multiply defined'))}
	 end
      end
      meth getGrammarSymbol(Symbol $)
	 ScannerSpecification, GetGrammarSymbol(@grammarSymbols Symbol $)
      end
      meth analyse(Rep)
	 {ForAll @lexicalRules proc {$ R} {R analyse(self Rep)} end}
	 ScannerSpecification, AnalyseGrammarSymbols(Rep)
      end
      meth generate(Globals OutFile ?Flex ?Locals ?Meth)
	 SCs Flex0 ICs CaseClauses X
      in
	 _#SCs#Locals =
	 {FoldL @grammarSymbols
	  fun {$ I#In#ASTs _#E}
	     case {E getEntryType($)} of lexicalAbbreviation then
		I#(In#{E generate($)})#ASTs
	     [] lexicalMode then AST in
		(I + 1)#(In#{E generate(I ?AST $)})#(AST|ASTs)
	     end
	  end 1#""#nil}
	 Flex = '%option c++ 8bit noyywrap nostack yymore nodefault'#
		{Globals getFlexOptions($)}#'\n'#
		'%option outfile="'#OutFile#
		'" prefix="'#{Globals getFlag(prefix $)}#'"\n'#
		SCs#'%%\n'#Flex0#'%%\n#include "lexer.h"\n'
	 _#Flex0#ICs =
	 {FoldR @lexicalRules
	  fun {$ R I#Flex#Cases} Flex0 Expr in
	     {R generate(self I ?Flex0 ?Expr)}
	     (I + 1)#(Flex#Flex0)#((I#Expr)|Cases)
	  end 1#""#[0#fApply(fSelf(unit)
			     [fRecord(fAtom('putToken1' unit)
				      [fAtom('EOF' unit)])] unit)]}
	 CaseClauses =
	 {FoldL ICs
	  fun {$ In I#C}
	     fCaseClause(fInt(I unit) C)|In
	  end nil}
	 X = fVar({Fresh} unit)
	 Meth = [fMeth(fRecord(fAtom('lexExecuteAction' unit)
			       [fMethArg(X fNoDefault)])
		       fCase(X [CaseClauses]
			     fNoElse(unit) unit) unit)]
      end

      meth GetGrammarSymbol(GrammarSymbols S $)
	 case GrammarSymbols of (T#Entry)|Rest then
	    if {SymbolEq S T} then Entry
	    else ScannerSpecification, GetGrammarSymbol(Rest S $)
	    end
	 [] nil then notFound
	 end
      end
      meth AnalyseGrammarSymbols(Rep)
	 case @grammarSymbolsNotAnalysed of Entries=_|_ then
	    grammarSymbolsNotAnalysed <- nil
	    {ForAll Entries proc {$ Entry} {Entry analyse(self Rep)} end}
	    ScannerSpecification, AnalyseGrammarSymbols(Rep)
	 else
	    if {Rep hasSeenError($)} then skip
	    else ModeDescendants in
	       ModeDescendants = {NewDictionary}
	       {ForAll @grammarSymbols
		proc {$ Symbol#Entry}
		   case {Entry getEntryType($)} of lexicalMode then
		      fVar(Y _) = Symbol in
		      {ForAll {Entry getParents($)}
		       proc {$ fVar(X _)}
			  {Dictionary.put ModeDescendants X
			   Y|{Dictionary.condGet ModeDescendants X nil}}
		       end}
		   else skip
		   end
		end}
	       {ForAll @grammarSymbols
		proc {$ _#Entry}
		   case {Entry getEntryType($)} of lexicalMode then
		      fVar(Y _) = {Entry getSymbol($)} in
		      {Entry setDescendants({Dictionary.condGet
					     ModeDescendants Y nil})}
		   else skip
		   end
		end}
	    end
	 end
      end
   end

   %--------------------------------------------------------------------
   % Lexical Rules
   %--------------------------------------------------------------------

   class LexicalRule
      prop final
      attr regularExpression: unit ozExpression: unit
      feat modeSymbol ruleNumber
      meth init(RegularExpression OzExpression)
	 regularExpression <- RegularExpression
	 ozExpression <- OzExpression
      end
      meth analyse(_ _)
	 skip
      end
      meth generate(Globals I ?Flex ?Expr) Flex0 in
	 case @regularExpression of '<<EOF>>' then
	    Flex0 = {SymbolToVirtualString self.modeSymbol}   %--** legal?
	 else Entry in
	    Entry = {Globals getGrammarSymbol(self.modeSymbol $)}
	    Flex0 = {FoldL {Entry getDescendants($)}
		     fun {$ In X} In#','#X end
		     {SymbolToVirtualString self.modeSymbol}}   %--** legal?
	 end
	 Flex = '<'#Flex0#'>'#@regularExpression#' return '#I#';\n'
	 Expr = @ozExpression
      end
   end

   %--------------------------------------------------------------------
   % Grammar Symbols
   %--------------------------------------------------------------------

   class GrammarSymbol
      attr symbol: unit
      meth init(Symbol)
	 symbol <- Symbol
      end
      meth getSymbol($)
	 @symbol
      end
   end

   class LexicalAbbreviation from GrammarSymbol
      prop final
      attr regularExpression: unit
      meth init(Symbol RegularExpression)
	 GrammarSymbol, init(Symbol)
	 regularExpression <- RegularExpression
      end
      meth getEntryType($)
	 lexicalAbbreviation
      end
      meth analyse(_ _)
	 skip
      end
      meth generate($)
	 {SymbolToVirtualString @symbol}#   %--** legal?
	 ' '#@regularExpression#'\n'
      end
   end

   class LexicalMode from GrammarSymbol
      prop final
      attr parents: unit descendants: unit
      meth init(Symbol Parents)
	 GrammarSymbol, init(Symbol)
	 parents <- Parents
      end
      meth getEntryType($)
	 lexicalMode
      end
      meth addParents(Symbols)
	 parents <- {Append Symbols @parents}
      end
      meth getParents($)
	 @parents
      end
      meth setDescendants(Descendants)
	 descendants <- Descendants
      end
      meth getDescendants($)
	 @descendants
      end
      meth analyse(Globals Rep) Parents = @parents in
	 {ForAll Parents
	  proc {$ S} GrammarSymbol in
	     GrammarSymbol = {Globals getGrammarSymbol(S $)}
	     if GrammarSymbol == notFound then
		{Rep error(coord: {CoordinatesOf S}
			   kind: 'scanner generator error'
			   msg: 'undefined mode '#{SymbolToVirtualString S})}
	     elseif {GrammarSymbol getEntryType($)} \= lexicalMode then
		{Rep error(coord: {CoordinatesOf S}
			   kind: 'scanner generator error'
			   msg: ({SymbolToVirtualString S}#
				 ' does not denote a lexical mode'))}
	     end
	  end}
	 parents <- nil
	 LexicalMode, MergeParents(Parents Globals Rep)
      end
      meth generate(I ?AST $)
	 case @symbol of fVar('INITIAL' _) then   % predefined by flex
	    AST = fEq(@symbol fInt(0 unit) unit)
	    ""
	 else
	    AST = fEq(@symbol fInt(I unit) unit)
	    '%x '#{SymbolToVirtualString @symbol}#'\n'   %--** legal?
	 end
      end

      meth MergeParents(Symbols Globals Rep)
	 case Symbols of Symbol|Rest then
	    if {SymbolEq Symbol @symbol} then
	       {Rep error(coord: {CoordinatesOf @symbol}
			  kind: 'scanner generator error'
			  msg: ('cyclic inheritance for lexical mode '#
				{SymbolToVirtualString @symbol}))}
	       LexicalMode, MergeParents(Rest Globals Rep)
	    elseif {Some @parents fun {$ S} {SymbolEq S Symbol} end} then
	       LexicalMode, MergeParents(Rest Globals Rep)
	    else
	       GrammarSymbol = {Globals getGrammarSymbol(Symbol $)} in
	       if GrammarSymbol \= notFound andthen
		  {GrammarSymbol getEntryType($)} == lexicalMode
	       then
		  Parents = {GrammarSymbol getParents($)} in
		  parents <- Symbol|@parents
		  LexicalMode, MergeParents({Append Parents Rest}
					    Globals Rep)
	       else   % errors only reported for direct parents (in `analyse')
		  LexicalMode, MergeParents(Rest Globals Rep)
	       end
	    end
	 [] nil then skip
	 end
      end
   end
in
   fun {TransformScanner T From Prop Attr Feat Ms Rules P Flags Rep}
      Globals
   in
      {Rep logPhase('processing scanner "'#{SymbolToVirtualString T}#'" ...')}
      Globals = {New ScannerSpecification init()}
      {Globals setFlags(Flags)}
      {Globals enterFrom(From)}
      {Globals enterProp(Prop)}
      {Globals enterAttr(Attr)}
      {Globals enterFeat(Feat)}
      {Globals enterMeth(Ms)}
      {ForAll Rules
       proc {$ Rule} {TransformScannerDescriptor Rule Globals Rep} end}
      if {Rep hasSeenError($)} then fSkip(unit)
      else
	 {Rep logSubPhase('analysing scanner ...')}
	 {Globals analyse(Rep)}
	 if {Rep hasSeenError($)} then fSkip(unit)
	 else Flex Local Locals LexMeth MakeLexer in
	    {Rep logSubPhase('extracting lexical rules ...')}
	    {Globals generate(Globals {MakeFileName T ".C"}
			      ?Flex ?Local|?Locals ?LexMeth)}
	    MakeLexer = {CompileScanner Flex T Rep}
	    case MakeLexer of stop then fSkip(unit)
	    else LexerLoad Locals2 Descrs Meths in
	       {Rep logSubPhase('building class definition ...')}
	       case MakeLexer of continue then
% {List.toRecord lexer
%  {List.zip
%   [create currentMode delete getAtom getLength getNextMatch getString
%    input setMode switchToBuffer unput]
%   {Record.toList
%    {Foreign.load {Class.getFeature T filenameprefix}#"MyScanner.dl"}}
%   fun {$ F X} F#X end}}
LexerO = fRecord(fAtom('#' unit)
		 [fApply(fOpApply('.' [fVar('Class' unit)
				       fAtom('getFeature' unit)] unit)
			 [T fAtom('filenameprefix' unit)] unit)
		  fAtom({MakeFileName T ".dl"} unit)])
	       in
LexerLoad =
fEq(fVar('`lexer`' unit)
fApply(fOpApply('.' [fVar('List' unit) fAtom('toRecord' unit)] unit)
       [fAtom(lexer unit)
	fApply(fOpApply('.' [fVar('List' unit) fAtom('zip' unit)] unit)
	       [fRecord(fAtom('|' unit) [fAtom(create unit)
		fRecord(fAtom('|' unit) [fAtom(currentMode unit)
		fRecord(fAtom('|' unit) [fAtom(delete unit)
		fRecord(fAtom('|' unit) [fAtom(getAtom unit)
		fRecord(fAtom('|' unit) [fAtom(getLength unit)
		fRecord(fAtom('|' unit) [fAtom(getNextMatch unit)
		fRecord(fAtom('|' unit) [fAtom(getString unit)
		fRecord(fAtom('|' unit) [fAtom(input unit)
		fRecord(fAtom('|' unit) [fAtom(setMode unit)
		fRecord(fAtom('|' unit) [fAtom(switchToBuffer unit)
		fRecord(fAtom('|' unit) [fAtom(unput unit)
		fAtom(nil unit)])])])])])])])])])])])
		fApply(fOpApply('.' [fVar('Record' unit) fAtom('toList' unit)] unit)
		       [fApply(fOpApply('.' [fVar('Foreign' unit) fAtom('load' unit)] unit)
			       [LexerO] unit)] unit)
		fFun(fDollar(unit) [fVar('F' unit) fVar('X' unit)]
		     fRecord(fAtom('#' unit) [fVar('F' unit) fVar('X' unit)])
		     nil unit)] unit)] unit) unit)
		  {Globals enterFeat([fAtom(lexer unit)#fVar('`lexer`' unit)])}
		  Locals2 = fVar('`lexer`' unit)|Locals
	       [] noLexer then
		  LexerLoad = fSkip(unit)
		  Locals2 = Locals
	       end
	       {Globals enterMeth(LexMeth)}
	       {Globals getDescrs(?Descrs)}
	       {Globals getMeth(?Meths)}
	       fLocal({FoldL Locals2 fun {$ In L} fAnd(In L) end Local}
		      fAnd(fClass(T Descrs Meths P) LexerLoad) unit)
	    end
	 end
      end
   end
end

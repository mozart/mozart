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
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
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

   local
      proc {GetCoord S ?Coord ?Msg} FileName Rest1 in
	 {List.takeDropWhile S fun {$ C} C \= &: end ?FileName ?Rest1}
	 case Rest1 of nil then
	    Coord = unit
	    Msg = S
	 elseof &:|Rest2 then Column in
	    {List.takeDropWhile Rest2 fun {$ C} C \= &: end ?Column &:|& |Msg}
	    Coord = pos({String.toAtom FileName} {String.toInt Column} ~1)
	 end
      end

      fun {OutputMessages S|Sr Rep}
	 case S of C|&:|R then
	    case C of &W then Coord Msg in
	       {GetCoord R ?Coord ?Msg}
	       {Rep warn(coord: Coord
			 kind: 'scanner generator warning'
			 msg: Msg)}
	       {OutputMessages Sr Rep}
	    [] &E then Coord Msg in
	       {GetCoord R ?Coord ?Msg}
	       {Rep error(coord: Coord
			  kind: 'scanner generator error'
			  msg: Msg)}
	       {OutputMessages Sr Rep}
	    [] &X then
	       Sr = nil
	       {String.toInt R}
	    end
	 else
	    {Rep warn(kind: 'scanner generator' msg: S)}
	    {OutputMessages Sr Rep}
	 end
      end

      class TextPipe from Open.text Open.pipe
	 prop final
	 meth getAll($)
	    case Open.text, getS($) of false then nil
	    elseof S then S2 in
	       S2 = {Filter S fun {$ C} C \= &\r end}
	       case S2 of &X|&:|_ then [S2]
	       else S2|TextPipe, getAll($)
	       end
	    end
	 end
      end

      fun {InvokeFlex FlexFile Rep} T RaiseOnBlock in
	 T = {Thread.this}
	 RaiseOnBlock = {Debug.getRaiseOnBlock T}
	 {Debug.setRaiseOnBlock T false}
	 try P Ss in
	    P = {New TextPipe init(cmd: {OZFLEX}
				   args: ['-Cem' FlexFile])}
	    {P getAll(?Ss)}
	    {P close()}
	    try
	       {OutputMessages Ss Rep}
	    catch _ then
	       {Rep error(kind: 'scanner generator error'
			  msg: 'unrecognized output from flex'
			  items: {Map Ss fun {$ S} line(S) end})}
	       1
	    end
	 catch E then
	    {Error.printException E}
	    {Rep error(kind: 'scanner generator'
		       msg: 'invocation of flex.exe failed')}
	    1
	 finally
	    {Debug.setRaiseOnBlock T RaiseOnBlock}
	 end
      end

      class TextFile from Open.file Open.text
	 prop final
	 meth readAll($)
	    case TextFile, getS($) of false then ""
	    elseof S then S#'\n'#TextFile, readAll($)
	    end
	 end
      end

      proc {ReadFile File ?VS} F in
	 F = {New TextFile init(name: File flags: [read])}
	 {F readAll(?VS)}
	 {F close()}
      end

      fun {GetTargetPlatform Rep} Tmp Cmd in
	 Tmp = {OS.tmpnam}
	 Cmd = {OZTOOL}#' platform -o '#Tmp
	 try Exit VS in
	    Exit = {OS.system Cmd}
	    VS = {ReadFile Tmp}
	    if Exit == 0 then
	       {Filter {VirtualString.toString VS}
		fun {$ C} {Not {Char.isSpace C}} end}
	    else
	       {Rep error(kind: 'scanner generator'
			  msg: 'invocation of oztool failed'
			  items: [hint(l: 'Command' m: Cmd)
				  hint(l: 'Exit code' m: Exit)])}
	       false
	    end
	 catch E then
	    {Error.printException E}
	    {Rep error(kind: 'scanner generator'
		       msg: 'invocation of oztool failed'
		       items: [hint(l: 'Command' m: Cmd)])}
	    false
	 finally
	    {OS.unlink Tmp}
	 end
      end
   in
      fun {CompileScanner Flex T Rep Dir}
	 case Flex of noLexer then noLexer
	 else FlexFile = {MakeFileName T ".l" Dir} in
	    {Rep startSubPhase('writing flex input file')}
	    {WriteVSFile Flex FlexFile}
	    {Rep startSubPhase('generating scanner tables')}
	    case {InvokeFlex FlexFile Rep} of 0 then
	       PLATFORM = {GetTargetPlatform Rep}
	       Cmd1 = ({OZTOOL}#' c++ '#{OZTOOLINC}#
		       ' -c '#{MakeFileName T ".C" Dir}#
		       ' -o '#{MakeFileName T ".o" Dir})
	       Cmd2 = ({OZTOOL}#' ld '#
		       ' -o '#{MakeFileName T ".so" Dir}#'-'#PLATFORM#' '#
		       {MakeFileName T ".o" Dir})
	       Exit1 Exit2
	    in
	       {Rep startSubPhase('compiling scanner')}
	       case PLATFORM of false then
		  stop
	       elseif (Exit1 = {OS.system Cmd1}) \= 0 then
		  {Rep error(kind: 'system error'
			     msg: 'invocation of oztool failed'
			     items: [hint(l: 'Command' m: Cmd1)
				     hint(l: 'Exit code' m: Exit1)])}
		  stop
	       elseif (Exit2 = {OS.system Cmd2}) \= 0 then
		  {Rep error(kind: 'system error'
			     msg: 'invocation of oztool failed'
			     items: [hint(l: 'Command' m: Cmd2)
				     hint(l: 'Exit code' m: Exit2)])}
		  stop
	       else continue
	       end
	    elseof I then
	       if {Rep hasSeenError($)} then skip
	       else
		  {Rep error(kind: 'system error'
			     msg: 'invocation of flex.exe failed'
			     items: [hint(l: 'Exit code' m: I)])}
	       end
	       stop
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
		       fCase(X CaseClauses fNoElse(unit) unit) unit)]
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
	    AST = fAnd(fEq(@symbol fInt(0 unit) unit)
		       fEq(@symbol @symbol unit))
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
   fun {TransformScanner T From Prop Attr Feat Ms Rules P Flags ImportFV Rep}
      Globals Dir FS Imports
   in
      {Rep startPhase('processing scanner "'#{SymbolToVirtualString T}#'"')}
      Globals = {New ScannerSpecification init()}
      {Globals setFlags(Flags)}
      {Globals enterFrom(From)}
      {Globals enterProp(Prop)}
      {Globals enterAttr(Attr)}
      {Globals enterFeat(Feat)}
      {Globals enterMeth(Ms)}
      Dir = {Globals getFlag(directory $)}
      {ForAll Rules
       proc {$ Rule} {TransformScannerDescriptor Rule Globals Rep} end}
      if {Rep hasSeenError($)} then
	 Imports = nil
	 fSkip(unit)
      else
	 {Rep startSubPhase('analysing scanner')}
	 {Globals analyse(Rep)}
	 if {Rep hasSeenError($)} then
	    Imports = nil
	    fSkip(unit)
	 else Flex Local Locals LexMeth MakeLexer in
	    {Rep startSubPhase('extracting lexical rules')}
	    {Globals generate(Globals {MakeFileName T ".C" Dir}
			      ?Flex ?Local|?Locals ?LexMeth)}
	    MakeLexer = {CompileScanner Flex T Rep
			 {Globals getFlag(directory $)}}
	    case MakeLexer of stop then
	       Imports = nil
	       fSkip(unit)
	    else LexerLoad Locals2 Descrs Meths in
	       {Rep startSubPhase('building class definition')}
	       case MakeLexer of continue then From in
		  From = {VirtualString.toAtom
			  {MakeFileName T ".so" unit}#'{native}'}
		  case ImportFV of unit then New Message in
		     Imports = nil
		     New = fApply(fVar('New' unit)
				  [fOpApply('.' [fVar('Module' unit)
						 fAtom(manager unit)] unit)
				   fAtom(init unit)] unit)
		     Message = fRecord(fAtom(link unit)
				       [fColon(fAtom(url unit)
					       fAtom(From unit))
					fVar('`lexer`' unit)])
		     LexerLoad = fLocal(fEq(fVar('M' unit) New unit)
					fApply(fVar('M' unit) [Message] unit)
					unit)
		  else Feature in
		     Imports = [Feature#From]
		     LexerLoad = fEq(fVar('`lexer`' unit)
				     fOpApply('.' [ImportFV
						   fAtom(Feature unit)] unit)
				     unit)
		  end
		  {Globals enterFeat([fAtom(lexer unit)#fVar('`lexer`' unit)])}
		  Locals2 = fVar('`lexer`' unit)|Locals
	       [] noLexer then
		  Imports = nil
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
      end = FS
      FS#Imports
   end
end

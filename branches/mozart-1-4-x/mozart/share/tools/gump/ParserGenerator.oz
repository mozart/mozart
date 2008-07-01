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
%% The Gump Parser Generator.
%%

local
   \insert Bison

   ParserGeneratorError = 'parser generator error'
   ParserGeneratorWarning = 'parser generator warning'

   %--------------------------------------------------------------------
   % Auxiliary Functions and Classes
   %--------------------------------------------------------------------

   fun {LookupProductionTemplate Templates Key ?NewVisibles}
      case Templates of Key0#ProductionTemplate|Rest then
	 if Key == Key0 then
	    NewVisibles = Rest
	    ProductionTemplate
	 else
	    {LookupProductionTemplate Rest Key ?NewVisibles}
	 end
      [] nil then notFound
      [] none then notFound
      end
   end

   %--------------------------------------------------------------------
   % Auxiliary Functions for Attributes

   fun {OutputAttr Attr}
      if {IsFree Attr} then ""
      else
	 case Attr of synthesized then ' /* synthesized */'
	 [] inherited then ' /* inherited */'
	 end
      end
   end

   fun {FindAttribute X As ?NewAs}
      case As of (Pair=Y#Attr)|Rest then
	 if X == Y then
	    NewAs = Rest
	    Attr
	 else NewAsRest in
	    NewAs = Pair|NewAsRest
	    {FindAttribute X Rest ?NewAsRest}
	 end
      [] nil then
	 % If the attribute is not contained in As, then it is not a
	 % local variable of the syntax rule and thus must be considered
	 % inherited.
	 NewAs = nil
	 inherited
      end
   end

   fun {AdjoinAttribute X As}
      if {Some As fun {$ Y#_} X == Y end} then As
      else (X#synthesized)|As
      end
   end

   fun {IntersectAttributes As Bs}
      {Filter As fun {$ X#_} {Some Bs fun {$ Y#_} X == Y end} end}
   end

   %--------------------------------------------------------------------
   % Auxiliary Functions on Oz Terms, Expressions and Variables

   local
      fun {Lookup X Assocs}
	 case Assocs of (Y#RHS)|Rest then
	    if X == Y then RHS
	    else {Lookup X Rest}
	    end
	 [] nil then notFound
	 end
      end
   in
      fun {RenameVariables OzExpr Renamings}
	 case OzExpr of fVar(X C) then
	    case {Lookup X Renamings} of notFound then OzExpr
	    elseof Y then fVar(Y C)
	    end
	 else
	    if {IsRecord OzExpr} then
	       {Record.map OzExpr fun {$ E} {RenameVariables E Renamings} end}
	    else OzExpr
	    end
	 end
      end

      fun {LookupSubst Symbol TemplI}
	 case Symbol of fVar(X _) then {Lookup X TemplI.2}
	 else notFound
	 end
      end
   end

   proc {FreeVariablesOf OzExpression VsHd VsTl}
      case OzExpression of fVar(_ _) then
	 VsHd = OzExpression|VsTl
      [] fClass(S Ts Ms _) then Vs VsInter1 VsInter2 PrivateClassMembers in
	 {FreeVariablesOf S Vs VsInter1}
	 {FoldL Ts fun {$ In T} {FreeVariablesOf T In $} end VsInter1 VsInter2}
	 {FoldL Ms fun {$ In M} {FreeVariablesOf M In $} end VsInter2 nil}
	 PrivateClassMembers =
	 {FoldR Ts
	  fun {$ T In}
	     case T of fAttr(As _) then {FoldR As PrivateAttrFeat In}
	     [] fFeat(Fs _) then {FoldR Fs PrivateAttrFeat In}
	     else In
	     end
	  end {FoldR Ms PrivateMeth nil}}
	 {VarListSub PrivateClassMembers Vs VsHd VsTl}
      [] fLocal(S T _) then Vs0 VsInter Vs1 in
	 {FreeVariablesOf S Vs0 VsInter}
	 {FreeVariablesOf T VsInter nil}
	 {GetPatternVariablesStatement S Vs1 nil}
	 {VarListSub Vs1 Vs0 VsHd VsTl}
      [] fCaseClause(S T) then Vs0 VsInter Vs1 in
	 {FreeVariablesOf S Vs0 VsInter}
	 {FreeVariablesOf T VsInter nil}
	 {GetPatternVariablesExpression S Vs1 nil}
	 {VarListSub Vs1 Vs0 VsHd VsTl}
      [] fClause(S T U) then Vs0 VsInter1 VsInter2 Vs1 in
	 {FreeVariablesOf S Vs0 VsInter1}
	 {FreeVariablesOf T VsInter1 VsInter2}
	 {FreeVariablesOf U VsInter2 nil}
	 {GetPatternVariablesStatement S Vs1 nil}
	 {VarListSub Vs1 Vs0 VsHd VsTl}
      else
	 if {IsRecord OzExpression} then
	    {Record.foldL OzExpression
	     fun {$ In T} {FreeVariablesOf T In $} end VsHd VsTl}
	 else
	    VsHd = VsTl
	 end
      end
   end

   %--------------------------------------------------------------------
   % Auxiliary Functions for Code Generation

   fun {SymbolToAtom Symbol}
      {VirtualString.toAtom {SymbolToVirtualString Symbol}}
   end

   fun {MakeSemanticValue Ts}
      case Ts of [T] then T else fRecord(fAtom('#' unit) Ts) end
   end

   fun {ShortenStackRest T}
      % transform `...|_|_|_|_' into `...|_':
      case T of fRecord(A [T Tr]) then NewTr in
	 NewTr = {ShortenStackRest Tr}
	 case T of fWildcard(_) then
	    case NewTr of fWildcard(_) then NewTr
	    else fRecord(A [T NewTr])
	    end
	 else fRecord(A [T NewTr])
	 end
      else T
      end
   end

   %-----------------------------------------------------------------------
   % Global Symbol Table
   %-----------------------------------------------------------------------

   class ParserSpecification from ClassDescriptors
      prop final
      attr
	 productionTemplates: unit
	 grammarSymbols: unit grammarSymbolsNotAnalysed: nil
	 lastStartToken: 0
      meth init(ProductionTemplates)
	 EOF = fAtom('EOF' unit)
	 EOFToken = {New Terminal init(EOF noRep)}
	 Error = fAtom('error' unit)
	 ErrorToken = {New Terminal init(Error noRep)}
	 Undefined = fAtom('$undefined.' unit)
	 UndefinedToken = {New Terminal init(Undefined noRep)}
      in
	 productionTemplates <- ProductionTemplates
	 grammarSymbols <- [EOF#EOFToken
			    Error#ErrorToken
			    Undefined#UndefinedToken]
      end
      meth addProductionTemplate(Key ProductionTemplate Rep)
	 {ProductionTemplate analyse(Rep)}
	 productionTemplates <- Key#ProductionTemplate|@productionTemplates
      end
      meth addGrammarSymbol(Entry Rep) Symbol CurrEntry in
	 {Entry getSymbol(?Symbol)}
	 ParserSpecification, getGrammarSymbol(Symbol ?CurrEntry)
	 case {Entry getEntryType($)} of terminal then
	    if CurrEntry == notFound then
	       grammarSymbols <- Symbol#Entry|@grammarSymbols
	    elseif {CurrEntry getEntryType($)} \= terminal then
	       {Rep error(coord: {CoordinatesOf Symbol}
			  kind: ParserGeneratorError
			  msg: ('grammar symbol '#
				{SymbolToVirtualString Symbol}#
				' multiply defined'))}
	    else
	       {CurrEntry setAssoc({Entry getAssoc($)} Rep)}
	    end
	 elsecase CurrEntry of notFound then
	    case {Entry getEntryType($)} of nonterminal then
	       {Entry provideDefaultProductionTemplates(@productionTemplates)}
	    else skip
	    end
	    grammarSymbols <- Symbol#Entry|@grammarSymbols
	    grammarSymbolsNotAnalysed <- Entry|@grammarSymbolsNotAnalysed
	 else
	    {Rep error(coord: {CoordinatesOf Symbol}
		       kind: ParserGeneratorError
		       msg: ('grammar symbol '#
			     {SymbolToVirtualString Symbol}#
			     ' multiply defined'))}
	 end
      end
      meth getGrammarSymbol(Symbol $) Res in
	 ParserSpecification, GetGrammarSymbol(@grammarSymbols Symbol ?Res)
	 case Res of notFound then
	    case Symbol of fAtom(X _) then
	       case {Atom.toString X} of [_] then Entry in
		  % single-character atoms correspond to literal character
		  % tokens:
		  Entry = {New Terminal init(Symbol noRep)}
		  grammarSymbols <- Symbol#Entry|@grammarSymbols
		  Entry
	       else notFound
	       end
	    else notFound
	    end
	 else Res
	 end
      end
      meth analyse(Rep)
	 case @grammarSymbolsNotAnalysed of Entry|Rest then
	    grammarSymbolsNotAnalysed <- Rest
	    {Entry analyse(self Rep)}
	    ParserSpecification, analyse(Rep)
	 else
	    if {Rep hasSeenError($)} then skip
	    else
	       {ForAll @grammarSymbols
		proc {$ _#Entry}
		   case {Entry getEntryType($)} of nonterminal then
		      {Entry classifyAttributes(self Rep)}
		   else skip
		   end
		end}
	       {ForAll @grammarSymbols
		proc {$ _#Entry}
		   case {Entry getEntryType($)} of nonterminal then
		      {Entry completeAttributes(Rep)}
		   else skip
		   end
		end}
	    end
	 end
      end
      meth output(T $)
	 {FoldL @grammarSymbols
	  fun {$ In _#Entry}
	     case {Entry getEntryType($)} of nonterminal then
		{Entry output($)}#NL#In
	     else In
	     end
	  end ""}
      end
      meth generate(?PTG)
	 PTG = {New ParseTableGenerator init()}
	 {ForAll @grammarSymbols
	  proc {$ _#Entry} {Entry generate(self PTG)} end}
      end
      meth generateStartToken(Coord $) I X Symbol in
	 % after this method has been executed, no further grammar symbols
	 % should be added to the grammar
	 I = @lastStartToken + 1
	 lastStartToken <- I
	 X = {VirtualString.toAtom 'startToken'#I}
	 Symbol = fAtom(X Coord)
	 if {Some @grammarSymbols fun {$ S#_} {SymbolEq S Symbol} end} then
	    ParserSpecification, generateStartToken(Coord $)
	 else Symbol
	 end
      end

      meth GetGrammarSymbol(GrammarSymbols S $)
	 case GrammarSymbols of G|Rest then T#Entry = G in
	    if {SymbolEq S T} then Entry
	    else ParserSpecification, GetGrammarSymbol(Rest S $)
	    end
	 [] nil then notFound
	 end
      end
   end

   %--------------------------------------------------------------------
   % Production Templates
   %--------------------------------------------------------------------

   class ProductionTemplate
      prop final
      attr variables: unit localRules: unit synExpression: unit
      meth init(Variables LocalRules SynExpression ReturnVariable)
	 variables <- {Map Variables
		       fun {$ V}
			  case V of fWildcard(C) then fVar({Fresh} C)
			  else V
			  end
		       end}
	 localRules <- LocalRules
	 case ReturnVariable of none then
	    synExpression <- SynExpression
	 [] fDollar(_) then
	    synExpression <- SynExpression
	 [] fVar(_ _) then Action in
	    Action = {New SynAction init(ReturnVariable)}
	    synExpression <- {New SynSequence
			      init([ReturnVariable] [SynExpression Action])}
	 end
      end
      meth getVariableNames($)
	 {Map @variables fun {$ fVar(X _)} X end}
      end
      meth getLocalRules($)
	 @localRules
      end
      meth getBody($)
	 @synExpression
      end
      meth analyse(Rep) LocalVariables in
	 LocalVariables =
	 {FoldR @localRules
	  fun {$ R Vs} Symbol in
	     {R getSymbol(?Symbol)}
	     case Symbol of fAtom(_ C) then
		{Rep error(coord: C kind: ParserGeneratorError
			   msg: ('Atom '#{SymbolToVirtualString Symbol}#
				 ' not allowed as local rule name'))}
		Vs
	     [] fVar(_ _) then
		Symbol|Vs
	     end
	  end @variables}
	 {ForAllTail LocalVariables
	  proc {$ V|Vr}
	     if {Some Vr fun {$ V0} {SymbolEq V V0} end} then
		{Rep error(coord: {CoordinatesOf V}
			   kind: ParserGeneratorError
			   msg: ('Symbol '#{SymbolToVirtualString V}#
				 ' multiply defined in production template'))}
	     end
	  end}
      end
   end

   %--------------------------------------------------------------------
   % Grammar Symbols
   %--------------------------------------------------------------------

   class GrammarSymbol
      attr symbol: unit
      meth init(Symbol)
	 case Symbol of fVar(_ _) then symbol <- Symbol
	 [] fAtom(_ _) then symbol <- Symbol
	 end
      end
      meth getSymbol($)
	 @symbol
      end
   end

   class Terminal from GrammarSymbol
      prop final
      attr assoc: none
      meth init(Symbol Rep)
	 case Symbol of fAtom(_ _) then
	    GrammarSymbol, init(Symbol)
	 elseof (Sym2=fAtom(_ _))#fRecord(fAtom(Assoc APos) [fInt(Prec IPos)])
	 then
	    GrammarSymbol, init(Sym2)
	    if Prec > 0 then
	       case Assoc of leftAssoc then assoc <- leftAssoc#Prec
	       [] rightAssoc then assoc <- rightAssoc#Prec
	       [] nonAssoc then assoc <- nonAssoc#Prec
	       else
		  {Rep error(coord: APos kind: ParserGeneratorError
			     msg: 'illegal associativity declaration')}
	       end
	    else
	       {Rep error(coord: IPos kind: ParserGeneratorError
			  msg: 'precedence must be > 0')}
	    end
	 elseof Sym2#fRecord(Label [fColon(fInt(1 _) I)]) then
	    % allow for e. g. `leftAssoc(1: 6)'
	    Terminal, init(Sym2#fRecord(Label [I]) Rep)
	 elseof fAtom(_ Pos)#_ then
	    {Rep error(coord: Pos kind: ParserGeneratorError
		       msg: 'illegal token declaration')}
	 end
      end
      meth getAssoc($)
	 @assoc
      end
      meth setAssoc(Assoc Rep)
	 case @assoc of none then assoc <- Assoc
	 elsecase Assoc of none then skip
	 else
	    {Rep error(coord: {CoordinatesOf @symbol}
		       kind: ParserGeneratorError
		       msg: ('multiple associativity declarations for '#
			     'terminal '#{SymbolToVirtualString @symbol}))}
	 end
      end
      meth getEntryType($)
	 terminal
      end
      meth generate(Globals PTG)
	 {PTG enterToken(@symbol @assoc)}
      end
   end

   class SynFormalParameterList
      prop final
      attr formalParameters: unit length: 0 dollarIndex: ~1
      meth init()
	 formalParameters <- {NewDictionary}
      end
      meth copy($ ?NewParameterList) Formals = @formalParameters in
	 % variables for attribute types are freshly allocated
	 NewParameterList = {New SynFormalParameterList init()}
	 {ForThread 0 @length - 1 1   % compute the renamings:
	  fun {$ Renamings I} Parameter#_#_ = {Dictionary.get Formals I} in
	     case Parameter of fDollar(_) then
		{NewParameterList addParameter(Parameter unit)}
		Renamings
	     [] fVar(X P) then NewVariable = {Fresh} in
		{NewParameterList addParameter(fVar(NewVariable P) unit)}
		(X#NewVariable)|Renamings
	     end
	  end nil}
      end
      meth addParameter(Parameter Rep)
	 case Parameter of fDollar(P) then
	    if @dollarIndex \= ~1 then
	       {Rep error(coord: P kind: ParserGeneratorError
			  msg: ('multiple nesting markers in formal '#
				'parameters'))}
	    else
	       {Dictionary.put @formalParameters @length
		Parameter#synthesized#P}
	       dollarIndex <- @length
	       length <- @length + 1
	    end
	 [] fVar(X P) then
	    if {Some {Dictionary.entries @formalParameters}
		fun {$ _#(P#_#_)}
		   case P of fVar(Y _) then X == Y else false end
		end}
	    then
	       {Rep error(coord: P kind: ParserGeneratorError
			  msg: ({SymbolToVirtualString Parameter}#
				' multiply contained in formal parameters'))}
	    else
	       {Dictionary.put @formalParameters @length Parameter#_#_}
	       length <- @length + 1
	    end
	 [] fWildcard(C) then
	    SynFormalParameterList, addParameter(fVar({Fresh} C) Rep)
	 end
      end
      meth hasDollar($)
	 @dollarIndex \= ~1
      end
      meth getDollarPos($)
	 {Dictionary.get @formalParameters @dollarIndex}.1.2
      end
      meth getLength($)
	 @length
      end
      meth getParameters($) Formals = @formalParameters in
	 {ForThread @length - 1 0 ~1
	  fun {$ Ps I} V#A#_ = {Dictionary.get Formals I} in (V#A)|Ps end nil}
      end
      meth output($)
	 if @length == 0 then ""
	 else Formals = @formalParameters Parameter AttrType in
	    Parameter#AttrType#_ = {Dictionary.get Formals 0}
	    {ForThread 1 @length - 1 1
	     fun {$ X I} Parameter#AttrType#_ = {Dictionary.get Formals I} in
		X#GL#{SymbolToVirtualString Parameter}#{OutputAttr AttrType}
	     end
	     '('#PU#{SymbolToVirtualString Parameter}#
	     {OutputAttr AttrType}}#PO#')'
	 end
      end
      meth setAttributeType(N X P Rep) Parameter AttrType P0 in
	 Parameter#AttrType#P0 = {Dictionary.get @formalParameters N}
	 if {IsFree P0} then P0 = P end
	 if {IsFree AttrType} orelse {IsFree X} then
	    AttrType = X
	 elseif AttrType \= X then Items in
	    Items = case AttrType of inherited then
		       [line('this is an inherited use') P0
			line('this is a synthesized use') P]
		    [] synthesized then
		       [line('this is a synthesized use') P0
			line('this is an inherited use') P]
		    end
	    {Rep error(coord: {CoordinatesOf Parameter}
		       kind: ParserGeneratorError
		       msg: ('conflicting attribute types of formal '#
			     'parameter '#{SymbolToVirtualString Parameter})
		       items: Items)}
	 end
      end
      meth completeAttributes(Rep) Formals = @formalParameters in
	 {For 0 @length - 1 1
	  proc {$ I} _#AttrType#_ = {Dictionary.get Formals I} in
	     if {IsFree AttrType} then AttrType = synthesized end
	  end}
      end
   end

   class SyntaxRule from GrammarSymbol
      prop final
      attr formalParameterList: unit synExpression: unit templI: none#nil#nil
      % TemplI stores the information about production templates in the form
      %   '#'(VisibleTemplates ActiveSubstitutions ImplicitParameters)

      meth init(Symbol FormalParameterList SynExpression)
	 GrammarSymbol, init(Symbol)
	 formalParameterList <- FormalParameterList
	 synExpression <- SynExpression
      end
      meth copy(TemplI ?Res)
	 Symbol Implicits FormalParameterList SynExpression Renamings in
	 {LookupSubst @symbol TemplI} = symbol(Symbol Implicits)
	 FormalParameterList = {@formalParameterList copy(?Renamings $)}
	 {ForAll Implicits
	  proc {$ V}
	     {FormalParameterList addParameter(V unit)}
	  end}
	 SynExpression = {@synExpression copy(Renamings $)}
	 Res = {New SyntaxRule init(Symbol FormalParameterList SynExpression)}
	 {Res setTemplI(TemplI)}
      end
      meth getEntryType($)
	 nonterminal
      end
      meth getParameterListLength($)
	 {@formalParameterList getLength($)}
      end
      meth setTemplI(TemplI)
	 templI <- TemplI
      end
      meth provideDefaultProductionTemplates(ProductionTemplates)
	 case @templI of none#Subst#Implicits then
	    templI <- ProductionTemplates#Subst#Implicits
	 else skip
	 end
      end
      meth analyse(Globals Rep) Expected Vs TemplI SeqEx NewEx in
	 Expected = if {@formalParameterList hasDollar($)} then term
		    else expr
		    end
	 Vs = {FoldL {@formalParameterList getParameters($)}
	       fun {$ Vs P}
		  case P of (V=fVar(_ _))#_ then V|Vs else Vs end
	       end nil}
	 TemplI = @templI.1#@templI.2#{Append Vs @templI.3}
	 SeqEx = {New SynSequence init(nil [@synExpression])}
	 NewEx = {SeqEx simplify(Globals TemplI Rep Expected $)}
	 SyntaxRule, MakeAlternative(Globals Rep Expected NewEx)
      end
      meth output($)
	 'syn '#{SymbolToVirtualString @symbol}#
	 {@formalParameterList output($)}#IN#NL#
	 {@synExpression output($)}#EX#NL#'end'
      end
      meth classifyAttributes(Globals Rep) As in
	 As = {FoldL {@formalParameterList getParameters($)}
	       fun {$ In P#Attr}
		  case P of fVar(X _) then
		     if {IsFree Attr} orelse Attr == synthesized then
			(X#Attr)|In
		     else In
		     end
		  [] fDollar(_) then In
		  end
	       end nil}
	 {@synExpression classifyAttributes(Globals Rep As _)}
      end
      meth setAttributeType(N X P Rep)
	 {@formalParameterList setAttributeType(N X P Rep)}
      end
      meth completeAttributes(Rep)
	 {@formalParameterList completeAttributes(Rep)}
      end
      meth getParameters($)
	 {@formalParameterList getParameters($)}
      end
      meth generate(Globals PTG) Is Ss Ps in
	 Is#Ss#Ps#_ = {FoldL {@formalParameterList getParameters($)}
		       fun {$ Is#Ss#Ps#I Variable#Attr}
			  case Attr of inherited then
			     (Variable|Is)#Ss#((I#inherited)|Ps)#(I + 1)
			  [] synthesized then
			     Is#(Variable|Ss)#((I#synthesized)|Ps)#(I + 1)
			  end
		       end nil#nil#nil#1}
	 {@synExpression generate(@symbol Globals Is Ss PTG)}
	 case @symbol of fAtom(_ _) then
	    {PTG declareStartSymbol(Globals @symbol Ps)}
	 [] fVar(_ _) then skip
	 end
      end

      meth MakeAlternative(Globals Rep Expected NewEx)
	 case {NewEx getType($)} of synAlternative then
	    synExpression <- NewEx
	 else
	    {NewEx normalize(Globals Rep Expected)}
	    synExpression <- {New SynAlternative init([NewEx])}
	 end
      end
   end

   %-----------------------------------------------------------------------
   % EBNF Expressions
   %-----------------------------------------------------------------------

   class SynExpression
      meth synCompareResult(Result Expected Rep OzTerm)
	 if Result == Expected then skip
	 else Coord in
	    Coord = case OzTerm of unit then unit
		    [] pos(_ _ _) then OzTerm
		    else {CoordinatesOf OzTerm}
		    end
	    case Expected of term then
	       {Rep error(coord: Coord kind: ParserGeneratorError
			  msg: 'statement at expression position')}
	    [] expr then
	       {Rep error(coord: Coord kind: ParserGeneratorError
			  msg: 'expression at statement position')}
	    end
	 end
      end
      meth getPatternVariables($)
	 nil
      end
   end

   class SynApplication from SynExpression
      prop final
      attr symbol: unit actualParameterList: unit
      meth init(Symbol ActualParameterList)
	 symbol <- Symbol
	 actualParameterList <- ActualParameterList
      end
      meth coordinates($)
	 {CoordinatesOf @symbol}
      end
      meth copy(Renamings $) NewParameterList in
	 NewParameterList = {@actualParameterList copy(Renamings $)}
	 {New SynApplication init(@symbol NewParameterList)}
      end
      meth getType($)
	 synApplication
      end
      meth getSymbol($)
	 @symbol
      end
      meth getParameterListLength($)
	 {@actualParameterList getLength($)}
      end
      meth getPatternVariables($)
	 {@actualParameterList getPatternVariables($)}
      end
      meth assignTo(OzTerm Rep) DollarIndex in
	 DollarIndex = {@actualParameterList getDollarIndex($)}
	 if DollarIndex > ~1 then Parameter in
	    Parameter = {New SynActualParameter init(OzTerm)}
	    {@actualParameterList replaceParameter(DollarIndex Parameter)}
	 else
	    {Rep error(coord: {CoordinatesOf @symbol}
		       kind: ParserGeneratorError
		       msg: ('missing nesting marker in application of '#
			     {SymbolToVirtualString @symbol}))}
	 end
      end
      meth simplify(Globals TemplI Rep Expected $)
	 case @symbol of fAtom('prec' _) then   % precedence token
	    case {@actualParameterList getLength($)} of 1 then Parameter in
	       Parameter = {{@actualParameterList getParameter(0 $)}
			    getValue($)}
	       case Parameter of fAtom(_ _) then Entry in
		  Entry = {Globals getGrammarSymbol(Parameter $)}
		  case Entry of notFound then
		     {Rep error(coord: {CoordinatesOf Parameter}
				kind: ParserGeneratorError
				msg: ('unknown grammar symbol '#
				      {SymbolToVirtualString Parameter}))}
		  else
		     if {Entry getEntryType($)} \= terminal then
			{Rep error(coord: {CoordinatesOf Parameter}
				   kind: ParserGeneratorError
				   msg: ('precedence token '#
					 {SymbolToVirtualString Parameter}#
					 ' must be a terminal'))}
		     end
		  end
	       else
		  {Rep error(coord: {CoordinatesOf Parameter}
			     kind: ParserGeneratorError
			     msg: 'illegal precedence specification')}
	       end
	    else
	       {Rep error(coord: {CoordinatesOf @symbol}
			  kind: ParserGeneratorError
			  msg: 'illegal precedence specification')}
	    end
	    SynExpression, synCompareResult(expr Expected Rep @symbol)
	    self
	 elsecase {LookupSubst @symbol TemplI} of symbol(Symbol Implicits) then
	    Actuals = @actualParameterList in
	    symbol <- Symbol
	    {ForAll Implicits proc {$ V} {Actuals addParameter(V Rep)} end}
	    SynApplication, Simplify(Globals Rep Expected $)
	 [] synExpression(Ex ExTemplI) then
	    case {@actualParameterList getLength($)} of 0 then
	       SynExpression, synCompareResult(expr Expected Rep @symbol)
	       {{Ex copy(nil $)} simplify(Globals ExTemplI Rep expr $)}
	    [] 1 then Parameter NewEx in
	       Parameter = {@actualParameterList getParameter(0 $)}
	       NewEx =
	       {{Ex copy(nil $)} simplify(Globals ExTemplI Rep term $)}
	       if {Parameter isDollar($)} then
		  SynExpression, synCompareResult(term Expected Rep @symbol)
	       else Value in
		  SynExpression, synCompareResult(expr Expected Rep @symbol)
		  Value = case {Parameter getValue($)} of V=fVar(_ P) then
			     fEscape(V P)
			  elseof V then V
			  end
		  {NewEx assignTo(Value Rep)}
	       end
	       NewEx
	    else
	       {Rep error(coord: {CoordinatesOf @symbol}
			  kind: ParserGeneratorError
			  msg: ('wrong number of parameters '#
				'in application of '#
				{SymbolToVirtualString @symbol}))}
	       self
	    end
	 [] notFound then
	    SynApplication, Simplify(Globals Rep Expected $)
	 end
      end
      meth getUsedLocals(Vs $) Actuals = @actualParameterList in
	 {SymbolSetIntersection Vs
	  {ForThread 0 {Actuals getLength($)} - 1 1
	   fun {$ Ws I}
	      {SymbolSetUnion Ws
	       {FreeVariablesOf {{Actuals getParameter(I $)} getValue($)}
		$ nil}}
	   end nil}}
      end
      meth output($)
	 {SymbolToVirtualString @symbol}#{@actualParameterList output($)}
      end
      meth classifyAttributes(Globals Rep As $) Entry in
	 Entry = {Globals getGrammarSymbol(@symbol $)}
	 case @symbol of fAtom('prec' _) then As
	 elsecase {Entry getEntryType($)} of terminal then
	    case {@actualParameterList getLength($)} of 1 then
	       CheckInherited Parameter in
	       proc {CheckInherited Attr P}
		  if {IsFree Attr} then
		     Attr = synthesized
		  else
		     case Attr of inherited then
			{Rep error(coord: P kind: ParserGeneratorError
				   msg: ('inherited attribute illegal for '#
					 'terminal'#
					 {SymbolToVirtualString @symbol}))}
		     [] synthesized then skip
		     end
		  end
	       end
	       Parameter = {@actualParameterList getParameter(0 $)}
	       case {Parameter getValue($)} of fVar(X P) then Attr NewAs in
		  Attr = {FindAttribute X As ?NewAs}
		  {CheckInherited Attr P}
		  NewAs
	       [] fEscape(V _) then fVar(X P) = V Attr NewAs in
		  Attr = {FindAttribute X As ?NewAs}
		  {CheckInherited Attr P}
		  NewAs
	       [] fDollar(_) then As
	       end
	    else As
	    end
	 [] nonterminal then Synthesized Actuals SymbolPos = @symbol.2 in
	    % Variables with attribute type `synthesized' may only appear
	    % at most once in an application and then only as a parameter.
	    Synthesized =
	    {FoldL As
	     fun {$ Xs X#Attr}
		if {IsFree Attr} orelse Attr == inherited then Xs
		else X|Xs
		end
	     end nil}
	    Actuals = @actualParameterList
	    {ForThread 0 {Entry getParameterListLength($)} - 1 1
	     fun {$ As#Illegal I} Parameter Value in
		Parameter = {Actuals getParameter(I $)}
		case {Parameter getValue($)} of fEscape(V _) then Value = V
		elseof V then Value = V
		end
		case Value of fVar(X P) then
		   if {Member X Illegal} then
		      {Rep error(coord: P kind: ParserGeneratorError
				 msg: 'variable synthesized more than once')}
		   else Attr NewAs in
		      Attr = {FindAttribute X As ?NewAs}
		      if {IsFree Attr} then
			 {Entry setAttributeType(I Attr P Rep)}
			 if {IsFree Attr} then As#Illegal
			 else NewAs#Illegal
			 end
		      else
			 case Attr of synthesized then
			    {Entry setAttributeType(I synthesized P Rep)}
			    NewAs#(X|Illegal)
			 [] inherited then
			    {Entry setAttributeType(I inherited P Rep)}
			    NewAs#Illegal
			 end
		      end
		   end
		[] fDollar(P) then
		   {Entry setAttributeType(I synthesized P Rep)}
		   As#Illegal
		else
		   % complex term is an inherited attribute computation
		   {ForAll {FreeVariablesOf Value $ nil}
		    proc {$ fVar(X P)}
		       if {Member X Synthesized} then
			  {Rep error(coord: P kind: ParserGeneratorError
				     msg: ('illegal use of non-allocated '#
					   'variable'))}
		       end
		    end}
		   {Entry setAttributeType(I inherited SymbolPos Rep)}
		   As#Illegal
		end
	     end As#nil}.1
	 end
      end
      meth generate(Globals DataFlow PTG) Actuals DollarIndex Entry in
	 Actuals = @actualParameterList
	 DollarIndex = {Actuals getDollarIndex($)}
	 if DollarIndex > ~1 then Variable Parameter in
	    % return parameter with nesting marker:
	    Variable = fVar({Fresh} unit)
	    Parameter = {New SynActualParameter init(Variable)}
	    {Actuals replaceParameter(DollarIndex Parameter)}
	    {DataFlow setAction(Variable)}
	 end
	 Entry = {Globals getGrammarSymbol(@symbol $)}
	 case @symbol of fAtom('prec' _) then
	    {DataFlow setRulePrecedence({{Actuals getParameter(0 $)}
					 getValue($)})}
	 elsecase {Entry getEntryType($)} of nonterminal then Ts Vs in
	    _#Ts#Vs = {FoldL {Entry getParameters($)}
		       fun {$ I#Ts#Vs _#Attr} Actual in
			  case {{Actuals getParameter(I $)} getValue($)}
			  of fEscape(V _) then Actual = V
			  elseof T then Actual = T
			  end
			  case Attr of inherited then
			     (I + 1)#(Actual|Ts)#Vs
			  [] synthesized then
			     (I + 1)#Ts#(Actual|Vs)
			  end
		       end 0#nil#nil}
	    case Ts of _|_ then NewSymbol in
	       NewSymbol = fVar({Fresh} unit)
	       {{DataFlow makeLocalDataFlow(Ts $)}
		makeRule(PTG NewSymbol @symbol.2)}
	       {DataFlow appendSymbol(NewSymbol [fWildcard(unit)])}
	    [] nil then skip
	    end
	    {DataFlow appendSymbol(@symbol Vs)}
	 [] terminal then Vs in
	    Vs = case {Actuals getLength($)} of 0 then [fWildcard(unit)]
		 [] 1 then Actual in
		    {{Actuals getParameter(0 $)} getValue(Actual)}
		    case Actual of fEscape(V _) then [V]
		    else [Actual]
		    end
		 end
	    {DataFlow appendSymbol(@symbol Vs)}
	 end
      end

      meth Simplify(Globals Rep Expected $) Entry in
	 Entry = {Globals getGrammarSymbol(@symbol $)}
	 case Entry of notFound then
	    {Rep error(coord: {CoordinatesOf @symbol}
		       kind: ParserGeneratorError
		       msg: ('undefined grammar symbol '#
			     {SymbolToVirtualString @symbol}))}
	    self
	 elsecase {Entry getEntryType($)} of terminal then
	    SynApplication, SimplifyTerminalAppl(Globals Rep Expected $)
	 [] nonterminal then
	    SynApplication, SimplifyNonterminalAppl(Entry Globals Rep
						    Expected $)
	 else
	    {Rep error(coord: {CoordinatesOf @symbol}
		       kind: ParserGeneratorError
		       msg: ({SymbolToVirtualString @symbol}#
			     ' is not a grammar symbol'))}
	    self
	 end
      end
      meth SimplifyTerminalAppl(Globals Rep Expected $)
	 case {@actualParameterList getLength($)} of 0 then
	    SynExpression, synCompareResult(expr Expected Rep @symbol)
	 [] 1 then Parameter in
	    Parameter = {@actualParameterList getParameter(0 $)}
	    if {Parameter isDollar($)} then
	       SynExpression, synCompareResult(term Expected Rep @symbol)
	    elseif {Parameter isVariable($)} then
	       SynExpression, synCompareResult(expr Expected Rep @symbol)
	    else
	       {Rep error(coord: {CoordinatesOf {Parameter getValue($)}}
			  kind: ParserGeneratorError
			  msg: ('expression at position of '#
				'synthesized attribute'))}
	    end
	 else
	    {Rep error(coord: {CoordinatesOf @symbol}
		       kind: ParserGeneratorError
		       msg: ('too many arguments in application of terminal '#
			     {SymbolToVirtualString @symbol}))}
	 end
	 self
      end
      meth SimplifyNonterminalAppl(Entry Globals Rep Expected $)
	 Length DollarIndex in
	 Length = {@actualParameterList getLength($)}
	 if Length \= {Entry getParameterListLength($)} then
	    {Rep error(coord: {CoordinatesOf @symbol}
		       kind: ParserGeneratorError
		       msg: ('wrong number of parameters in application of '#
			     {SymbolToVirtualString @symbol}))}
	 end
	 DollarIndex = {@actualParameterList getDollarIndex($)}
	 if DollarIndex \= ~1 then
	    SynExpression, synCompareResult(term Expected Rep @symbol)
	 else
	    SynExpression, synCompareResult(expr Expected Rep @symbol)
	 end
	 self
      end
   end

   class SynAction from SynExpression
      attr ozExpression: unit
      meth init(OzExpression)
	 ozExpression <- OzExpression
      end
      meth coordinates($)
	 {CoordinatesOf @ozExpression}
      end
      meth copy(Renamings $) NewExpression in
	 NewExpression = {RenameVariables @ozExpression Renamings}
	 {New SynAction init(NewExpression)}
      end
      meth getType($)
	 synAction
      end
      meth getExpression($)
	 @ozExpression
      end
      meth prependExpression(Expression)
	 ozExpression <- fAnd(Expression @ozExpression)
      end
      meth assignTo(OzTerm Rep)
	 ozExpression <- fEq(OzTerm @ozExpression unit)
      end
      meth simplify(Globals TemplI Rep Expected $)
	 self
      end
      meth getUsedLocals(Vs $)
	 {SymbolSetIntersection Vs {FreeVariablesOf @ozExpression $ nil}}
      end
      meth output($)
	 NL#'=> '#IN#{OutputOz @ozExpression}#EX
      end
      meth classifyAttributes(Globals Rep As $)
	 As
      end
      meth generate(Globals DataFlow PTG)
	 {DataFlow setAction(@ozExpression)}
      end
   end

   class SynSequence from SynExpression
      prop final
      attr localVariables: unit synExpressions: unit coord: unit visibles: unit
      meth init(LocalVariables SynExpressions Coord <= unit)
	 localVariables <- LocalVariables
	 synExpressions <- SynExpressions
	 coord <- Coord
      end
      meth coordinates($)
	 case @synExpressions of E|_ then {E coordinates($)}
	 else unit
	 end
      end
      meth copy(Renamings $) Freshs LocalRen Expressions in
	 Freshs = {Map @localVariables fun {$ fVar(_ P)} fVar({Fresh} P) end}
	 LocalRen = {Append {List.zip @localVariables Freshs
			     fun {$ V1 V2} V1.1#V2.1 end} Renamings}
	 Expressions = {Map @synExpressions fun {$ E} {E copy(LocalRen $)} end}
	 {New SynSequence init(Freshs Expressions)}
      end
      meth getType($)
	 synSequence
      end
      meth getSynExpressions($)
	 @synExpressions
      end
      meth enterLocals(LocalVariables)
	 localVariables <- {SymbolSetUnion LocalVariables @localVariables}
      end
      meth getLocals($)
	 @localVariables
      end
      meth assignTo(OzTerm Rep)
	 case @synExpressions of Expressions=_|_ then
	    {{List.last Expressions} assignTo(OzTerm Rep)}
	 [] nil then
	    {Rep error(coord: {CoordinatesOf OzTerm}
		       kind: ParserGeneratorError
		       msg: 'epsilon sequence may not be assigned to')}
	 end
      end
      meth simplify(Globals TemplI Rep Expected $)
	 case @synExpressions of nil then
	    SynExpression, synCompareResult(expr Expected Rep @coord)
	    self
	 [] Exs=_|_ then Vs NewTemplI Results Alt in
	    SynSequence, EnterDeclarations(Exs)
	    Vs = {SymbolSetUnion @localVariables TemplI.3}
	    NewTemplI = TemplI.1#TemplI.2#Vs
	    Results = SynSequence, Simplify(Exs Globals NewTemplI
					    Rep Expected $)
	    Alt = case Results of [Ex] then {Ex getType($)} == synAlternative
		  else false
		  end
	    if Alt then Ex = Results.1 in
	       {Ex enterLocals(@localVariables)}
	       Ex
	    else
	       synExpressions <- Results
	       visibles <- {SymbolSetUnion @localVariables TemplI.3}
	       self
	    end
	 end
      end
      meth normalize(Globals Rep Expected) Merged in
	 Merged = SynSequence, MergeActions(@synExpressions $)
	 synExpressions <-
	 SynSequence, Normalize(Globals Merged Rep @visibles Expected $)
      end
      meth getUsedLocals(Vs $)
	 {FoldL @synExpressions
	  fun {$ Ws Ex} {SymbolSetUnion Ws {Ex getUsedLocals(Vs $)}} end nil}
      end
      meth output($)
	 case @localVariables of Vs=_|_ then
	    {LI {Map Vs SymbolToVirtualString} GL}#GL#'in'#NL
	 else ""
	 end#
	 case @synExpressions of Exs=Ex|_ then
	    case {Ex getType($)} of synAction then 'skip' else "" end#
	    {LI {Map Exs fun {$ Ex} {Ex output($)} end} GL}
	 [] nil then 'skip'
	 end
      end
      meth classifyAttributes(Globals Rep As $)
	 {FoldL @synExpressions
	  fun {$ As Ex} {Ex classifyAttributes(Globals Rep As $)} end
	  {FoldL @localVariables
	   fun {$ As fVar(X _)} {AdjoinAttribute X As} end As}}
      end
      meth generate(Symbol Globals Is Ss PTG) Ls DataFlow in
	 Ls = {Append {Filter Ss fun {$ X} {Label X} == fVar end}
	       @localVariables}
	 DataFlow = {New SynDataFlowClass init(Is Ss Ls)}
	 {PTG setStacks(DataFlow)}
	 {ForAll @synExpressions
	  proc {$ Ex} {Ex generate(Globals DataFlow PTG)} end}
	 {DataFlow makeRule(PTG Symbol Symbol.2)}
      end

      meth EnterDeclarations(Exs)   % make declarations explicit
	 case Exs of Ex|Exr then
	    SynSequence, enterLocals({Ex getPatternVariables($)})
	    SynSequence, EnterDeclarations(Exr)
	 [] nil then skip
	 end
      end
      meth Simplify(Expressions Globals TemplI Rep Expected $)
	 case Expressions of Ex|ExRest then NewEx NewExs in
	    case ExRest of _|_ then
	       NewEx = {Ex simplify(Globals TemplI Rep expr $)}
	    [] nil then
	       NewEx = {Ex simplify(Globals TemplI Rep Expected $)}
	    end
	    NewExs = SynSequence, Simplify(ExRest Globals TemplI
					   Rep Expected $)
	    case {NewEx getType($)} of synSequence then
	       Clashes Freshs LocalRen in
	       % Incorporate the nested sequence.  Before doing this,
	       % rename its local variables to avoid name clashes.
	       Clashes = {NewEx getLocals($)}
	       Freshs = {Map Clashes fun {$ fVar(_ P)} fVar({Fresh} P) end}
	       LocalRen = {List.zip Clashes Freshs fun {$ V1 V2} V1.1#V2.1 end}
	       SynSequence, enterLocals(Freshs)
	       {FoldR {NewEx getSynExpressions($)}
		fun {$ Ex In} {Ex copy(LocalRen $)}|In end NewExs}
	    else NewEx|NewExs
	    end
	 [] nil then nil
	 end
      end
      meth MergeActions(Sequence $)
	 case Sequence of Ex1|(SequenceRest=Ex2|_) then
	    if {Ex1 getType($)} == synAction
	       andthen {Ex2 getType($)} == synAction
	    then
	       {Ex2 prependExpression({Ex1 getExpression($)})}
	       SynSequence, MergeActions(SequenceRest $)
	    else Ex1|(SynSequence, MergeActions(SequenceRest $))
	    end
	 else Sequence
	 end
      end
      meth Normalize(Globals Sequence Rep Vs Expected $)
	 case Sequence of Ex|ExRest then ExType = {Ex getType($)} in
	    if ExType == synAction andthen ExRest \= nil
	       orelse ExType == synAlternative
	    then Implicits in
	       Implicits = {Ex getUsedLocals(Vs $)}
	       case ExRest of _|_ then
		  SynSequence, Externalize(Globals expr Implicits Ex Rep $)
	       else
		  SynSequence, Externalize(Globals Expected Implicits Ex
					   Rep $)
	       end
	    else Ex
	    end|(SynSequence, Normalize(Globals ExRest Rep Vs Expected $))
	 [] nil then nil
	 end
      end
      meth Externalize(Globals Expected Implicits Ex Rep $)
	 Formals Actuals Symbol NewRule in
	 Formals = {New SynFormalParameterList init()}
	 Actuals = {New SynActualParameterList init()}
	 {ForAll Implicits
	  proc {$ V}
	     {Formals addParameter(V Rep)}
	     {Actuals addParameter(V Rep)}
	  end}
	 case Expected of term then P = fDollar(unit) in
	    {Formals addParameter(P Rep)}
	    {Actuals addParameter(P Rep)}
	 [] expr then skip
	 end
	 Symbol = fVar({Fresh} {@synExpressions.1 coordinates($)})
	 NewRule = {New SyntaxRule init(Symbol Formals Ex)}
	 {Globals addGrammarSymbol(NewRule Rep)}
	 {New SynApplication init(Symbol Actuals)}
      end
   end

   class SynAlternative from SynExpression
      prop final
      attr synExpressions: unit
      meth init(SynExpressions)
	 synExpressions <- SynExpressions
      end
      meth coordinates($)
	 case @synExpressions of E|_ then {E coordinates($)}
	 else unit
	 end
      end
      meth copy(Renamings $) Expressions in
	 Expressions = {Map @synExpressions
			fun {$ E} {E copy(Renamings $)} end}
	 {New SynAlternative init(Expressions)}
      end
      meth getType($)
	 synAlternative
      end
      meth getSynExpressions($)
	 @synExpressions
      end
      meth enterLocals(LocalVariables)
	 {ForAll @synExpressions proc {$ E} {E enterLocals(LocalVariables)} end}
      end
      meth assignTo(OzTerm Rep)
	 {ForAll @synExpressions proc {$ E} {E assignTo(OzTerm Rep)} end}
      end
      meth simplify(Globals TemplI Rep Expected $)
	 case @synExpressions of nil then
	    {New SynSequence init(nil nil)}
	 [] Exs=_|_ then NewExs in
	    NewExs = SynAlternative, Simplify(Exs Globals TemplI Rep
					      Expected $)
	    case NewExs of [Ex] then Ex
	    else
	       {ForAll NewExs
		proc {$ Ex} {Ex normalize(Globals Rep Expected)} end}
	       synExpressions <- NewExs
	       self
	    end
	 end
      end
      meth getUsedLocals(Vs $)
	 {FoldL @synExpressions
	  fun {$ Ws Ex} {SymbolSetUnion Ws {Ex getUsedLocals(Vs $)}} end nil}
      end
      meth output($)
	 '(  '#PU#
	 {LI {Map @synExpressions fun {$ Ex} {Ex output($)} end}
	  EX#NL#'[] '#IN}#PO#NL#')'
      end
      meth classifyAttributes(Globals Rep As $)
	 {FoldL @synExpressions
	  fun {$ In E}
	     {IntersectAttributes In
	      {E classifyAttributes(Globals Rep As $)}}
	  end As}
      end
      meth generate(Symbol Globals Is Ss PTG)
	 {ForAll @synExpressions
	  proc {$ Ex} {Ex generate(Symbol Globals Is Ss PTG)} end}
      end

      meth Simplify(Expressions Globals TemplI Rep Expected $)
	 case Expressions of Ex|Exs then SeqEx NewEx NewExs in
	    SeqEx = {New SynSequence init(nil [Ex])}
	    NewEx = {SeqEx simplify(Globals TemplI Rep Expected $)}
	    NewExs = SynAlternative, Simplify(Exs Globals TemplI Rep
					      Expected $)
	    case {NewEx getType($)} of synAlternative then
	       {Append {NewEx getSynExpressions($)} NewExs}
	    [] synSequence then NewEx|NewExs
	    end
	 [] nil then nil
	 end
      end
   end

   class SynAssignment from SynExpression
      prop final
      attr ozTerm: unit synExpression: unit
      meth init(OzTerm AssignedExpression)
	 ozTerm <- OzTerm
	 synExpression <- AssignedExpression
      end
      meth coordinates($)
	 {CoordinatesOf @ozTerm}
      end
      meth copy(Renamings $) OzTerm Expression in
	 OzTerm = {RenameVariables @ozTerm Renamings}
	 Expression = {@synExpression copy(Renamings $)}
	 {New SynAssignment init(OzTerm Expression)}
      end
      meth getType($)
	 synAssignment
      end
      meth getPatternVariables($)
	 case @ozTerm of fVar(_ _) then [@ozTerm]
	 else nil
	 end
      end
      meth simplify(Globals TemplI Rep Expected $) SeqEx NewEx OzTerm in
	 SynExpression, synCompareResult(expr Expected Rep @ozTerm)
	 SeqEx = {New SynSequence init(nil [@synExpression])}
	 NewEx = {SeqEx simplify(Globals TemplI Rep term $)}
	 OzTerm = case @ozTerm of fVar(_ P) then fEscape(@ozTerm P)
		  else @ozTerm
		  end
	 {NewEx assignTo(OzTerm Rep)}
	 NewEx
      end
      meth getUsedLocals(Vs $)
	 {SymbolSetUnion {SymbolSetIntersection Vs
			  {FreeVariablesOf @ozTerm $ nil}}
	  {@synExpression getUsedLocals(Vs $)}}
      end
   end

   class SynTemplateInstantiation from SynExpression
      prop final
      attr key: unit actualExpressions: unit pos: unit
      meth init(Key ActualExpressions Pos)
	 key <- Key
	 actualExpressions <- ActualExpressions
	 pos <- Pos
      end
      meth coordinates($)
	 case @actualExpressions of E|_ then {E coordinates($)}
	 else unit
	 end
      end
      meth copy(Renamings $) Exs in
	 Exs = {Map @actualExpressions fun {$ E} {E copy(Renamings $)} end}
	 {New SynTemplateInstantiation init(@key Exs @pos)}
      end
      meth getType($)
	 synTemplateInstantiation
      end
      meth simplify(Globals TemplI Rep Expected $) Template NewVisibles in
	 case Expected of term then key <- @key.1#(&=|@key.2)
	 [] expr then skip
	 end
	 Template = {LookupProductionTemplate TemplI.1 @key ?NewVisibles}
	 case Template of notFound then Key in
	    case @key of none#S then S
	    elseof fAtom(X _)#S then X#':'#S
	    end = Key
	    {Rep error(coord: @pos kind: ParserGeneratorError
		       msg: 'unknown production template with key `'#Key#'\'')}
	    {New SynSequence init(nil nil)}
	 else VariableNames LocalRules Vs NewSubst NewTemplI in
	    VariableNames = {Template getVariableNames($)}
	    LocalRules = {Template getLocalRules($)}
	    Vs = SynTemplateInstantiation, getUsedLocals(TemplI.3 $)
	    NewSubst = {Append
			{List.zip VariableNames @actualExpressions
			 fun {$ V A}
			    V#synExpression({New SynSequence init(nil [A])}
					    TemplI)
			 end}
			{Map LocalRules
			 fun {$ R} fVar(X P) = {R getSymbol($)} in
			    X#symbol(fVar({Fresh} P) Vs)
			 end}}
	    NewTemplI = NewVisibles#NewSubst#nil
	    {ForAll LocalRules
	     proc {$ Rule}
		{Globals addGrammarSymbol({Rule copy(NewTemplI $)} Rep)}
	     end}
	    {{New SynSequence init(nil [{{Template getBody($)} copy(nil $)}])}
	     simplify(Globals NewVisibles#NewSubst#TemplI.3 Rep Expected $)}
	 end
      end
      meth getUsedLocals(Vs $)
	 {FoldL @actualExpressions
	  fun {$ Ws Ex} {SymbolSetUnion Ws {Ex getUsedLocals(Vs $)}} end nil}
      end
   end

   %--------------------------------------------------------------------
   % Actual Parameter Lists for Terminal and Nonterminal Applications
   %--------------------------------------------------------------------

   class SynActualParameter
      prop final
      attr value
      meth init(Value)
	 value <- case Value of fWildcard(P) then fVar({Fresh} P)
		  else Value
		  end
      end
      meth isDollar($)
	 {Label @value} == fDollar
      end
      meth isVariable($)
	 {Label @value} == fVar orelse {Label @value} == fEscape
      end
      meth isPatternVariable($)
	 case @value of fVar(_ _) then true
	 else false
	 end
      end
      meth getValue($)
	 @value
      end
   end

   class SynActualParameterList
      prop final
      attr actualParameters: unit length: 0 dollarIndex: ~1
      meth init()
	 actualParameters <- {NewDictionary}
      end
      meth copy(Renamings ?NewParameterList) Actuals in
	 NewParameterList = {New SynActualParameterList init()}
	 Actuals = @actualParameters
	 {For 0 @length - 1 1
	  proc {$ I} OldValue NewValue in
	     {{Dictionary.get Actuals I} getValue(?OldValue)}
	     NewValue = {RenameVariables OldValue Renamings}
	     {NewParameterList addParameter(NewValue unit)}
	  end}
      end
      meth addParameter(OzTerm Rep) ActualParameter in
	 ActualParameter = {New SynActualParameter init(OzTerm)}
	 if {ActualParameter isDollar($)} then
	    if @dollarIndex == ~1 then
	       {Dictionary.put @actualParameters @length ActualParameter}
	       dollarIndex <- @length
	       length <- @length + 1
	    else
	       {Rep error(coord: {CoordinatesOf {ActualParameter getValue($)}}
			  kind: ParserGeneratorError
			  msg: ('multiple nesting markers in '#
				'actual parameters'))}
	    end
	 else
	    {Dictionary.put @actualParameters @length ActualParameter}
	    length <- @length + 1
	 end
      end
      meth getLength($)
	 @length
      end
      meth getParameter(Index $)
	 {Dictionary.get @actualParameters Index}
      end
      meth replaceParameter(Index NewActualParameter)
	 case @dollarIndex == Index of true then
	    {Dictionary.put @actualParameters @dollarIndex NewActualParameter}
	    dollarIndex <- ~1
	 elsecase Index >= 0 andthen Index < @length of true then
	    {Dictionary.put @actualParameters Index NewActualParameter}
	 end
      end
      meth getDollarIndex($)
	 @dollarIndex
      end
      meth getPatternVariables($) Actuals = @actualParameters in
	 {ForThread 0 @length - 1 1
	  fun {$ In I} ActualParameter = {Dictionary.get Actuals I} in
	     if {ActualParameter isPatternVariable($)} then
		{ActualParameter getValue($)}|In
	     else In
	     end
	  end nil}
      end
      meth output($)
	 if @length == 0 then ""
	 else Actuals = @actualParameters in
	    '('#PU#
	    {ForThread 1 @length - 1 1
	     fun {$ X I} P = {Dictionary.get Actuals I} in
		X#GL#{OutputOz {P getValue($)}}
	     end {OutputOz {{Dictionary.get Actuals 0} getValue($)}}}#
	    PO#')'
	 end
      end
   end

   %--------------------------------------------------------------------
   % The Parse Table Generator and Data Flow Classes
   %--------------------------------------------------------------------

   class ParseTableGenerator
      prop final
      attr
	 start: unit startSymbols: nil symbols: nil tokens: nil
	 rules rulesTail: unit ruleNumber: 1 actions: unit
	 stackX: unit newStackX: unit assocs: nil
      meth init()
	 start <- fVar({Fresh} unit)
	 rulesTail <- @rules
	 actions <- {NewDictionary}
	 stackX <- fVar({Fresh} unit)
	 newStackX <- fVar({Fresh} unit)
      end
      meth setStacks(DataFlow)
	 {DataFlow setStacks(@stackX @newStackX)}
      end
      meth enterToken(Symbol AssocPrec)
	 case Symbol of fAtom(_ _) then
	    X = ParseTableGenerator, ConvertSymbol(Symbol $)
	 in
	    tokens <- X|@tokens
	    case AssocPrec of none then skip
	    [] Assoc#Prec then assocs <- (X#Assoc#Prec)|@assocs
	    end
	 end
      end
      meth declareStartSymbol(Globals Symbol Ps)
	 Feat Decl TSymbol Token X Action in
	 Feat = Symbol.1
	 Decl = {AdjoinList Feat Ps}
	 TSymbol = {Globals generateStartToken(unit $)}
	 Token = ParseTableGenerator, ConvertSymbol(TSymbol $)
	 tokens <- Token|@tokens
	 startSymbols <- (Feat#((TSymbol.1)#Decl))|@startSymbols
	 X = fVar({Fresh} unit)
	 Action = fLocal(fEq(fRecord(fAtom('|' unit)
				     [X fEscape(@newStackX unit)])
			     @stackX unit)
			 X unit)
	 ParseTableGenerator, enterRule(@start [TSymbol Symbol] Action
					Symbol.2 '_')
      end
      meth enterRule(Symbol RHS Action Position RulePrecedence)
	 X Rule NewTail
      in
	 X = ParseTableGenerator, ConvertSymbol(Symbol $)
	 Rule = case RulePrecedence of '_' then
		   Position#X#(ParseTableGenerator, ConvertSymbols(RHS $))
		else
		   Position#X#(ParseTableGenerator, ConvertSymbols(RHS $))#
		   RulePrecedence
		end
	 @rulesTail = Rule|NewTail
	 rulesTail <- NewTail
	 {Dictionary.put @actions @ruleNumber Action}
	 ruleNumber <- @ruleNumber + 1
      end
      meth hasStartSymbols($)
	 @startSymbols \= nil
      end
      meth generateTables(Globals VerboseFile P Rep ?Meth ?Tables)
	 case @startSymbols of nil then
	    Meth = nil
	    Tables = '#'()
	 else ADict CaseClauses X Dollar Grammar Tables0 StartSymbols in
	    {Rep startSubPhase('generating parse tables')}
	    @rulesTail = nil
	    ADict = @actions
	    CaseClauses =
	    {ForThread @ruleNumber - 1 1 ~1
	     fun {$ In I}
		fCaseClause(fInt(I unit) {Dictionary.get ADict I})|In
	     end nil}
	    X = fVar({Fresh} unit)
	    Dollar = fDollar(unit)
	    Meth = [fMeth(fRecord(fAtom('synExecuteAction' unit)
				  [fMethArg(X fNoDefault)
				   fMethArg(@stackX fNoDefault)
				   fMethArg(@newStackX fNoDefault)
				   fMethArg(Dollar fNoDefault)])
			  fCase(X CaseClauses fNoElse(unit) unit) unit)]
	    Grammar = (ParseTableGenerator, ConvertSymbol(@start $)#@tokens#
		       @assocs#@rules)
	    try
	       Tables0 = {Bison {Length @symbols} Grammar VerboseFile Rep}
	    catch ozbison(VS) then
	       {Rep error(kind: ParserGeneratorError coord: P
			  msg: 'parse table generator exited abnormally'
			  items: [hint(l: 'Fatal error' m: VS)])}
	       Tables0 = ozbisonTables
	    end
	    case {CondSelect Tables0 conflicts unit} of unit then skip
	    elseof SR#RR then
	       if SR \= {Globals getFlag(expect $)} orelse RR \= 0 then
		  {Rep warn(kind: ParserGeneratorWarning coord: P
			    msg: 'parser specification contains conflicts'
			    items: [hint(l: 'Shift/reduce' m: SR)
				    hint(l: 'Reduce/reduce' m: RR)])}
	       end
	    end
	    case {CondSelect Tables0 useless unit} of unit then skip
	    elseof R#N then
	       {Rep warn(kind: ParserGeneratorWarning coord: P
			 msg: 'parser specification contains useless items'
			 items: [hint(l: 'Useless rules' m: R)
				 hint(l: 'Useless nonterminals' m: N)])}
	    end
	    StartSymbols = {AdjoinList synStartSymbols @startSymbols}
	    Tables = {AdjoinAt Tables0 synStartSymbols StartSymbols}
	 end
      end

      meth ConvertSymbol(Symbol ?X)
	 X = {SymbolToAtom Symbol}
	 if {Member X @symbols} then skip
	 else symbols <- X|@symbols
	 end
      end
      meth ConvertSymbols(Symbols $)
	 case Symbols of S|Sr then
	    ParseTableGenerator, ConvertSymbol(S $)|
	    ParseTableGenerator, ConvertSymbols(Sr $)
	 [] nil then nil
	 end
      end
   end

   class SynDataFlowClass
      prop final
      attr
	 stack: unit newStack: unit returnValues: unit notAllocated: unit
	 rhs rhsTail: unit action: none
	 stackX: unit newStackX: unit
	 rulePrecedence: '_'   % meaning none specified
      meth init(InhVariables ReturnValues LocalVariables)
	 stack <- case InhVariables of nil then nil else [InhVariables] end
	 newStack <- @stack
	 returnValues <- ReturnValues
	 notAllocated <- {Map LocalVariables fun {$ fVar(X _)} X end}
	 rhsTail <- @rhs
      end
      meth makeLocalDataFlow(ReturnValues $) DF in
	 DF = {New SynDataFlowClass Set(@stack ReturnValues @notAllocated)}
	 {DF setStacks(@stackX @newStackX)}
	 DF
      end
      meth setStacks(StackX NewStackX)
	 stackX <- StackX
	 newStackX <- NewStackX
      end
      meth appendSymbol(Symbol Ds) NewTail in
	 stack <- Ds|@stack
	 @rhsTail = Symbol|NewTail
	 rhsTail <- NewTail
	 notAllocated <- {Filter @notAllocated
			  fun {$ X}
			     {All Ds fun {$ D}
					case D of fDollar(_) then true
					[] fVar(Y _) then X \= Y
					[] fEscape(fVar(Y _) _) then X \= Y
					[] fWildcard(_) then true
					end
				     end}
			  end}
      end
      meth setAction(Action)
	 action <- Action
      end
      meth setRulePrecedence(PrecSymbol)
	 rulePrecedence <- {SymbolToAtom PrecSymbol}
      end
      meth makeRule(PTG Symbol Position)
	 ActionVs NewSt NewStackX Extract Ns Ls Action in
	 ActionVs = {UniqueVariables
		     {FreeVariablesOf @action|@returnValues $ nil}}
	 NewSt = @newStack
	 NewStackX = @newStackX
	 Extract = fEq({FoldRTail @stack
			fun {$ Dss In} TmpDs NewDs ExtractRest in
			   {Map Dss.1   % replace all unneeded variables by `_'
			    fun {$ D}
			       case D of fVar(_ _) then
				  if {Some ActionVs
				      fun {$ V} {SymbolEq D V} end}
				  then D
				  else fWildcard(unit)
				  end
			       [] fWildcard(_) then D
			       end
			    end ?TmpDs}
			   NewDs = if {All TmpDs   % replace `_#_#...#_' by `_'
				       fun {$ D} {Label D} == fWildcard end}
				   then fWildcard(unit)
				   else {MakeSemanticValue TmpDs}
				   end
			   ExtractRest = fRecord(fAtom('|' unit) [NewDs In])
			   if Dss == NewSt then
			      fEq(fEscape(NewStackX unit)
				  {ShortenStackRest ExtractRest} unit)
			   else ExtractRest
			   end
			end
			case NewSt of nil then fEscape(NewStackX unit)
			[] _|_ then fWildcard(unit)
			end}
		       @stackX unit)
	 Ns = @notAllocated
	 Ls = {FoldL ActionVs
	       fun {$ Ls V=fVar(X _)}
		  if {Member X Ns} then fAnd(V Ls) else Ls end
	       end Extract}
	 case @action of none then
	    Action = fLocal(Ls {MakeSemanticValue @returnValues} unit)
	 else ReturnValues NewLs NewAction in
	    ReturnValues#NewLs#NewAction =
	    {FoldR @returnValues
	     fun {$ V Rs#Ls#Action}
		case V of fVar(_ _) then (V|Rs)#Ls#Action
		[] fDollar(_) then NewVar in
		   NewVar = fVar({Fresh} unit)
		   (NewVar|Rs)#fAnd(NewVar Ls)#fEq(NewVar Action unit)
		end
	     end nil#Ls#@action}
	    Action =
	    fLocal(NewLs fAnd(NewAction {MakeSemanticValue ReturnValues}) unit)
	 end
	 @rhsTail = nil
	 {PTG enterRule(Symbol @rhs Action Position @rulePrecedence)}
      end

      meth Set(Stack ReturnValues NotAllocated)
	 stack <- Stack
	 newStack <- Stack
	 returnValues <- ReturnValues
	 notAllocated <- NotAllocated
	 rhsTail <- @rhs
	 action <- none
	 rulePrecedence <- '_'   % meaning none specified
      end
   end

   class ProductionTemplatesClass
      prop final
      attr productionTemplates: nil
      meth init()
	 skip
      end
      meth add(Ts Rep) OldPs in
	 OldPs = @productionTemplates
	 ProductionTemplatesClass, Add(Ts Rep)
	 if {Rep hasSeenError($)} then
	    productionTemplates <- OldPs
	 end
      end
      meth Add(Ts Rep)
	 case Ts of T|Tr then Key Vs Rs A ReturnVariable RsO AO PT in
	    T = fProductionTemplate(Key Vs Rs A ReturnVariable)
	    RsO = {Map Rs fun {$ R} {TransformSyn R Rep} end}
	    AO = {TransformSyn A Rep}
	    PT = {New ProductionTemplate init(Vs RsO AO ReturnVariable)}
	    {PT analyse(Rep)}
	    productionTemplates <- (Key#PT)|@productionTemplates
	    ProductionTemplatesClass, Add(Tr Rep)
	 [] nil then skip
	 end
      end
      meth get($)
	 @productionTemplates
      end
   end

   %--------------------------------------------------------------------
   % Transformation from Tuples into Oject Abstract Syntax

   local
      fun {TransformSyns As Rep}
	 case As of A|Ar then
	    {TransformSyn A Rep}|{TransformSyns Ar Rep}
	 [] nil then nil
	 end
      end
   in
      fun {TransformSyn E Rep}
	 case E of fSyntaxRule(S Ps A) then Formals in
	    Formals = {New SynFormalParameterList init()}
	    {ForAll Ps proc {$ P} {Formals addParameter(P Rep)} end}
	    {New SyntaxRule init(S Formals {TransformSyn A Rep})}
	 [] fSynApplication(F Es) then Actuals in
	    Actuals = {New SynActualParameterList init()}
	    {ForAll Es proc {$ E} {Actuals addParameter(E Rep)} end}
	    {New SynApplication init(F Actuals)}
	 [] fSynAction(E) then
	    {New SynAction init(E)}
	 [] fSynSequence(Vs As C) then
	    {New SynSequence init(Vs {TransformSyns As Rep} C)}
	 [] fSynAlternative(As) then
	    {New SynAlternative init({TransformSyns As Rep})}
	 [] fSynAssignment(X A) then
	    {New SynAssignment init(X {TransformSyn A Rep})}
	 [] fSynTemplateInstantiation(Key As C) then
	    {New SynTemplateInstantiation init(Key {TransformSyns As Rep} C)}
	 end
      end
   end
in
   %--------------------------------------------------------------------
   % Expansion of Parser Specifications

   fun {MakeProductionTemplates}
      {New ProductionTemplatesClass init()}
   end

   fun {TransformParser T From Prop Attr Feat Ms Tokens Rules P Flags
	ProdTempl Rep}
      Globals
   in
      {Rep startPhase('processing parser "'#{SymbolToVirtualString T}#'"')}
      Globals = {New ParserSpecification init(ProdTempl)}
      {Globals setFlags(Flags)}
      {Globals enterFrom(From)}
      {Globals enterProp(Prop)}
      {Globals enterAttr(Attr)}
      {Globals enterFeat(Feat)}
      {Globals enterMeth(Ms)}
      case Tokens of fToken(Ss) then
	 {ForAll Ss
	  proc {$ S}
	     {Globals addGrammarSymbol({New Terminal init(S Rep)} Rep)}
	  end}
      end
      {ForAll Rules
       proc {$ Rule}
	  case Rule of fProductionTemplate(Key Vs Rs A ReturnVariable) then
	     RsO = {Map Rs fun {$ R} {TransformSyn R Rep} end}
	     AO = {TransformSyn A Rep}
	     PT = {New ProductionTemplate init(Vs RsO AO ReturnVariable)} in
	     {Globals addProductionTemplate(Key PT Rep)}
	  [] fSyntaxRule(_ _ _) then
	     {Globals addGrammarSymbol({TransformSyn Rule Rep} Rep)}
	  end
       end}
      {Rep startSubPhase('analysing and expanding grammar')}
      {Globals analyse(Rep)}
      if {Rep hasSeenError($)} then fSkip(unit)
      else PTG F SynMeth Tables in
	 {Rep startSubPhase('extracting BNF')}
	 {Globals generate(?PTG)}
	 if {PTG hasStartSymbols($)} then skip
	 else
	    {Rep error(coord: P kind: ParserGeneratorError
		       msg: 'grammar has no start symbol')}
	 end
	 if {Globals getFlag(outputSimplified $)} then
	    {WriteVSFile
	     {FormatStringToVirtualString {Globals output(T $)}}
	     {MakeFileName T ".simplified" {Globals getFlag(directory $)}}}
	 end
	 F = if {Globals getFlag(verbose $)} then
		{MakeFileName T ".output" {Globals getFlag(directory $)}}
	     else ''
	     end
	 {PTG generateTables(Globals F P Rep ?SynMeth ?Tables)}
	 if {Rep hasSeenError($)} then fSkip(unit)
	 else Descrs Meths in
	    {Rep startSubPhase('building class definition')}
	    {Globals
	     enterFeat({Record.foldRInd Tables
			fun {$ F V In}
			   ({ValueToAST F}#{ValueToAST V})|In
			end nil})}
	    {Globals enterMeth(SynMeth)}
	    {Globals getDescrs(?Descrs)}
	    {Globals getMeth(?Meths)}
	    fClass(T Descrs Meths P)
	 end
      end
   end
end

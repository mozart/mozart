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
%%%   $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%   $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%%
%% Functions to output Oz abstract syntax to textual form.
%%

local
   fun {ButLast Xs}
      case Xs of [_] then nil
      elseof X|Xr then X|{ButLast Xr}
      end
   end

   IN = format(indent)
   EX = format(exdent)
   NL = format(break)
   PU = format(push)
   PO = format(pop)
   GL = format(glue(" "))
   fun {LI Xs Sep} format(list(Xs Sep)) end

   fun {OzBlock Ex}
      case Ex of fLocal(S T _) then PU#{Oz S 0}#EX#NL#'in'#IN#NL#{Oz T 0}#PO
      else {Oz Ex 0}
      end
   end

   fun {DoOutput VS P Q}
      % P: required precedence level
      % Q: precedence level of term in VS
      case Q > P then VS else '('#PU#VS#PO#')' end
   end

   fun {Oz Ex P}
      % any term output by this function must have a precedence of at least P
      case Ex of _|_ then {LI {Map Ex OutputOz} NL}#NL
      [] nil then ""
      [] dirHalt then '\\halt'
      [] dirHelp then '\\help'
      [] dirSwitch(Switches) then
	 {FoldL Switches
	  fun {$ In Switch}
	     In#case Switch of on(X _) then ' +'#X
		[] off(X _) then ' -'#X
		end
	  end '\\switch'}
      [] dirShowSwitches then '\\showSwitches'
      [] dirFeed(Filename) then '\\feed \''#Filename#'\''
      [] dirThreadedFeed(Filename) then '\\threadedfeed \''#Filename#'\''
      [] dirCore(Filename) then '\\core \''#Filename#'\''
      [] dirMachine(Filename) then '\\machine \''#Filename#'\''
      [] dirExpect(_) then ""
      [] fAnd(S T) then
	 case S of fSkip(_) then {Oz T P}
	 elsecase T of fSkip(_) then {Oz S P}
	 else {DoOutput {Oz S 1}#NL#{Oz T 0} P 1}
	 end
      [] fDeclare(S T _) then
	 'declare'#NL#{OutputOz S}#NL#
	 case T of fSkip(_) then ""
	 else 'in'#NL#{OutputOz T}
	 end
      [] fSkip(_) then 'skip'
      [] fFail(_) then 'fail'
      [] fAssign(S T _) then {DoOutput {Oz S 201}#' <- '#{Oz T 200} P 200}
      [] fAtom(X _) then {System.valueToVirtualString X 0 0}
      [] fVar(X _) then {PrintNameToVirtualString X}
      [] fEscape(V _) then '!'#{OutputOz V}
      [] fWildcard(_) then '_'
      [] fRecord(L Ts) then
	 case L of fAtom(X _) then
	    case X == '#' andthen {Length Ts} > 1 then
	       {DoOutput {LI {Map Ts fun {$ T} {Oz T 801} end} "#"} P 800}
	    elsecase X == '|' andthen {Length Ts} == 2 then
	       {Oz Ts.1 701}#'|'#{Oz Ts.2.1 700}
	    else
	       {OutputOz L}#'('#PU#{LI {Map Ts OutputOz} GL}#PO#')'
	    end
	 else
	    {OutputOz L}#'('#PU#{LI {Map Ts OutputOz} GL}#PO#')'
	 end
      [] fOpenRecord(L Ts) then
	 {OutputOz L}#'('#PU#{LI {Map Ts OutputOz} GL}#GL#PO#'...)'
      [] fObjPattern(L Ts) then
	 {OutputOz L}#'<<'#PU#{LI {Map Ts OutputOz} GL}#GL#PO#'>>'
      [] fColon(S T) then {Oz S P}#': '#{Oz T P}
      [] fInt(X _) then
	 case X < 0 then '~'#~X
	 else X
	 end
      [] fFloat(X _) then
	 case X < 0.0 then '~'#~X
	 else X
	 end
      [] fApply(S Ts _) then '{'#PU#{LI {Map S|Ts OutputOz} GL}#PO#'}'
      [] fObjApply(S T _) then {DoOutput {Oz S 1101}#', '#{Oz T 1100} P 1100}
      [] fProc(T Fs E ProcFlags _) then
	 PU#'proc '#IN#{LI ProcFlags OutputOz}#
	 '{'#PU#{LI {Map T|Fs OutputOz} GL}#PO#'}'#NL#
	 {OzBlock E}#EX#NL#PO#'end'
      [] fFun(T Fs E ProcFlags _) then
	 PU#'fun '#IN#{LI ProcFlags OutputOz}#
	 '{'#PU#{LI {Map T|Fs OutputOz} GL}#PO#'}'#IN#NL#
	 {OzBlock E}#EX#NL#PO#'end'
      [] fFunctor(T Ds S1 S2 _) then
	 'functor '#{OutputOz T}#NL#{LI {Map Ds OutputOz} NL}#NL#
	 'body'#IN#NL#{OutputOz S1}#EX#NL#'in'#IN#NL#{OutputOz S2}#EX#NL#'end'
      [] fImport(Is _) then 'import'#IN#NL#{LI {Map Is OutputOz} NL}#EX
      [] fImportItem(V Fs OptFrom) then
	 {OutputOz V}#
	 case Fs of _|_ then '.{'#{LI {Map Fs OutputOz} GL}#'}'
	 [] nil then ""
	 end#
	 case OptFrom of fFrom(A) then ' from '#{OutputOz A}
	 [] fNoFrom() then ""
	 end
      [] fExport(Es _) then 'export'#IN#NL#{LI {Map Es OutputOz} NL}#EX
      [] fExportItem(V) then {OutputOz V}
      [] fOpApply(O Ts _) then
	 case O of '~' then [T] = Ts in
	    {DoOutput '~'#{Oz T 1201} P 1200}
	 else [T1 T2] = Ts in
	    case O of '==' then {DoOutput {Oz T1 501}#' == '#{Oz T2 501} P 500}
	    [] '<' then {DoOutput {Oz T1 501}#' < '#{Oz T2 501} P 500}
	    [] '>' then {DoOutput {Oz T1 501}#' > '#{Oz T2 501} P 500}
	    [] '=<' then {DoOutput {Oz T1 501}#' =< '#{Oz T2 501} P 500}
	    [] '>=' then {DoOutput {Oz T1 501}#' >= '#{Oz T2 501} P 500}
	    [] '\\=' then {DoOutput {Oz T1 501}#' \\= '#{Oz T2 501} P 500}
	    [] '+' then {DoOutput {Oz T1 900}#' + '#{Oz T2 901} P 900}
	    [] '-' then {DoOutput {Oz T1 900}#' - '#{Oz T2 901} P 900}
	    [] '*' then {DoOutput {Oz T1 1000}#' * '#{Oz T2 1001} P 1000}
	    [] '/' then {DoOutput {Oz T1 1000}#' / '#{Oz T2 1001} P 1000}
	    [] 'div' then {DoOutput {Oz T1 1000}#' div '#{Oz T2 1001} P 1000}
	    [] 'mod' then {DoOutput {Oz T1 1000}#' mod '#{Oz T2 1001} P 1000}
	    [] '.' then {DoOutput {Oz T1 1300}#'.'#{Oz T2 1301} P 1300}
	    [] '^' then {DoOutput {Oz T1 1300}#'^'#{Oz T2 1301} P 1300}
	    end
	 end
      [] fFdCompare(O S T _) then
	 {DoOutput {Oz S 501}#' '#O#' '#{Oz T 501} P 500}
      [] fFdIn(O S T _) then
	 {DoOutput {Oz S 600}#' '#O#' '#{Oz T 601} P 600}
      [] fAt(T _) then {DoOutput '@'#{Oz T 1401} P 1400}
      [] fAndThen(S T _) then {DoOutput {Oz S 401}#' andthen '#{Oz T 400} P 400}
      [] fOrElse(S T _) then {DoOutput {Oz S 301}#' orelse '#{Oz T 300} P 300}
      [] fClass(T Ds Ms _) then
	 'class '#{OutputOz T}#IN#NL#{LI {Map Ds OutputOz} NL}#NL#
	 {LI {Map Ms OutputOz} NL}#EX#NL#'end'
      [] fFrom(Ts _) then 'from '#IN#{LI {Map Ts OutputOz} GL}#EX
      [] fProp(Ts _) then 'prop '#IN#NL#{LI {Map Ts OutputOz} NL}#EX
      [] fAttr(Ts _) then 'attr '#IN#NL#{LI {Map Ts OutputOz} NL}#EX
      [] fFeat(Ts _) then 'feat '#IN#NL#{LI {Map Ts OutputOz} NL}#EX
      [] S#T then {Oz S P}#': '#{Oz T P}
      [] fMeth(H T _) then 'meth '#{OutputOz H}#IN#NL#{OzBlock T}#EX#NL#'end'
      [] fMethArg(T D) then {Oz T P}#{Oz D P}
      [] fMethColonArg(F T D) then {Oz F P}#': '#{Oz T P}#{Oz D P}
      [] fDefault(T _) then ' <= '#{Oz T P}
      [] fNoDefault then ""
      [] fDollar(_) then '$'
      [] fSelf(_) then 'self'
      [] fEq(S T _) then {DoOutput {Oz S 101}#' = '#{Oz T 100} P 100}
      [] fIf(Clauses IfElse _) then
	 PU#'if '#{LI {Map Clauses OutputOz} NL#'[] '}#
	 case IfElse of fNoElse(_) then ""
	 else NL#'else'#IN#NL#{OzBlock IfElse}#EX
	 end#NL#PO#'end'
      [] fClause(R S T) then
	 {OutputOz R}#' in '#{OutputOz S}#
	 case T of fNoThen(_) then "" else ' then'#IN#NL#{OzBlock T}#EX end
      [] fCondis(Tss _) then
	 PU#'condis '#IN#
	 {LI {Map Tss fun {$ Ts} {LI {Map Ts OutputOz} GL} end} EX#NL#'[] '#IN}#
	 EX#NL#PO#'end'
      [] fOr(Clauses Type _) then
	 PU#case Type of fchoice then 'choice '
	    [] fdis then 'dis '
	    [] for then 'or '
	    end#{LI {Map Clauses OutputOz} NL#'[] '}#NL#PO#'end'
      [] fBoolCase(E T1 T2 _) then
	 PU#'case '#{OutputOz E}#' then'#IN#NL#{OzBlock T1}#EX#
	 case T2 of fNoElse(_) then ""
	 else NL#'else'#IN#NL#{OzBlock T2}#EX
	 end#NL#PO#'end'
      [] fCase(T Css E _) then
	 PU#'case '#{OutputOz T}#' of '#
	 {LI {Map Css fun {$ Cs} {LI {Map Cs OutputOz} NL#'[] '} end}
	  NL#'elseof '}#
	 case E of fNoElse(_) then ""
	 else NL#'else'#IN#NL#{OzBlock E}#EX
	 end#NL#PO#'end'
      [] fCaseClause(S T) then
	 {OutputOz S}#' then'#IN#NL#{OzBlock T}#EX
      [] fLocal(S T _) then
	 case S of fSkip(_) then {Oz T P}
	 else
	    PU#'local'#IN#NL#{OutputOz S}#EX#NL#'in'#IN#NL#
	    {OutputOz T}#EX#NL#PO#'end'
	 end
      [] fLockThen(S T _) then
	 'lock '#{OutputOz S}#' then'#IN#NL#{OzBlock T}#EX#NL#'end'
      [] fLock(S _) then
	 'lock'#IN#NL#{OzBlock S}#EX#NL#'end'
      [] fThread(T _) then
	 'thread'#IN#NL#{OzBlock T}#EX#NL#'end'
      [] fNot(T _) then
	 'not'#IN#NL#{OzBlock T}#EX#NL#'end'
      [] fTry(R S T _) then
	 'try'#IN#NL#{OzBlock R}#EX#NL#
	 case S of fNoCatch then ""
	 [] fCatch(Ss _) then 'catch '#{LI {Map Ss OutputOz} NL#'[] '}#EX#NL
	 end#
	 case T of fNoFinally then ""
	 else 'finally'#IN#NL#{OzBlock T}#EX#NL
	 end#'end'
      [] fRaise(S _) then
	 'raise'#IN#NL#{OzBlock S}#EX#NL#'end'
      [] fRaiseWith(S T _) then
	 'raise'#IN#NL#{OzBlock S}#EX#NL#'with'#IN#NL#{OzBlock T}#EX#NL#'end'
      [] fGrammar(T Obj _) then
	 {Obj output(T $)}
      end
   end
in
   fun {OutputOz Ex}
      {Oz Ex 0}
   end
end

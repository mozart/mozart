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
   fun {LI Xs Sep} list(Xs Sep) end

   fun {OzBlock Ex}
      case Ex of fLocal(S T _) then PU#{Oz S 0}#EX#NL#'in'#IN#NL#{Oz T 0}#PO
      else {Oz Ex 0}
      end
   end

   fun {DoOutput VS P Q}
      % P: required precedence level
      % Q: precedence level of term in VS
      if Q > P then VS else '('#PU#VS#PO#')' end
   end

   fun {Oz Ex P}
      % any term output by this function must have a precedence of at least P
      case Ex of _|_ then {LI {Map Ex OutputOz} NL}#NL
      [] nil then ""
      [] dirSwitch(Switches) then
	 {FoldL Switches
	  fun {$ In Switch}
	     In#case Switch of on(X _) then ' +'#X
		[] off(X _) then ' -'#X
		end
	  end '\\switch'}
      [] dirLocalSwitches then '\\localSwitches'
      [] dirPushSwitches then '\\pushSwitches'
      [] dirPopSwitches then '\\popSwitches'
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
      [] fAtom(X _) then oz(X)
      [] fVar(X _) then pn(X)
      [] fEscape(V _) then '!'#{OutputOz V}
      [] fWildcard(_) then '_'
      [] fRecord(L Ts) then
	 case L of fAtom(X _) then
	    if X == '#' andthen {Length Ts} > 1 then
	       {DoOutput {LI {Map Ts fun {$ T} {Oz T 801} end} "#"} P 800}
	    elseif X == '|' andthen {Length Ts} == 2 then
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
	 if X < 0 then '~'#~X else X end
      [] fFloat(X _) then
	 if X < 0.0 then '~'#~X else X end
      [] fApply(S Ts _) then '{'#PU#{LI {Map S|Ts OutputOz} GL}#PO#'}'
      [] fObjApply(S T _) then {DoOutput {Oz S 1101}#', '#{Oz T 1100} P 1100}
      [] fProc(T Fs E ProcFlags _) then
	 PU#'proc '#IN#{LI {Map ProcFlags OutputOz} GL}#EX#
	 '{'#PU#{LI {Map T|Fs OutputOz} GL}#PO#'}'#NL#
	 {OzBlock E}#EX#NL#PO#'end'
      [] fFun(T Fs E ProcFlags _) then
	 PU#'fun '#IN#{LI {Map ProcFlags OutputOz} GL}#EX#
	 '{'#PU#{LI {Map T|Fs OutputOz} GL}#PO#'}'#IN#NL#
	 {OzBlock E}#EX#NL#PO#'end'
      [] fFunctor(T Ds _) then
	 'functor '#{OutputOz T}#NL#
	 case Ds of _|_ then {LI {Map Ds OutputOz} NL}#NL
	 [] nil then ""
	 end#'end'
      [] fRequire(Rs _) then 'require'#IN#NL#{LI {Map Rs OutputOz} NL}#EX
      [] fPrepare(S1 S2 _) then
	 'prepare'#IN#NL#{OutputOz S1}#EX#NL#'in'#IN#NL#{OutputOz S2}#EX
      [] fImport(Is _) then 'import'#IN#NL#{LI {Map Is OutputOz} NL}#EX
      [] fImportItem(V Fs OptImportAt) then
	 {OutputOz V}#
	 case Fs of _|_ then
	    '('#{LI {Map Fs
		     fun {$ X}
			case X of V#F then {OutputOz F}#': '#{OutputOz V}
			else {OutputOz X}
			end
		     end} GL}#')'
	 [] nil then ""
	 end#
	 case OptImportAt of fImportAt(A) then ' at '#{OutputOz A}
	 [] fNoImportAt then ""
	 end
      [] fExport(Es _) then 'export'#IN#NL#{LI {Map Es OutputOz} NL}#EX
      [] fExportItem(V) then {OutputOz V}
      [] fDefine(S1 S2 _) then
	 'define'#IN#NL#{OutputOz S1}#EX#NL#'in'#IN#NL#{OutputOz S2}#EX
      [] fOpApply(O Ts _) then
	 case O of '~' then [T] = Ts in
	    {DoOutput '~'#{Oz T 1201} P 1200}
	 elsecase Ts of [T1 T2] then
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
	 else
	    '{'#PU#'`'#O#'`'#
	    case Ts of _|_ then GL#{LI {Map Ts OutputOz} GL}
	    [] nil then ""
	    end#'}'#PO
	 end
      [] fOpApplyStatement(O Ts _) then
	 '{'#PU#'`'#O#'`'#
	 case Ts of _|_ then GL#{LI {Map Ts OutputOz} GL}
	 [] nil then ""
	 end#'}'#PO
      [] fFdCompare(O S T _) then
	 {DoOutput {Oz S 501}#' '#O#' '#{Oz T 501} P 500}
      [] fFdIn(O S T _) then
	 {DoOutput {Oz S 600}#' '#O#' '#{Oz T 601} P 600}
      [] fAt(T _) then {DoOutput '@'#{Oz T 1401} P 1400}
      [] fAndThen(S T _) then
	 {DoOutput {Oz S 401}#' andthen '#{Oz T 400} P 400}
      [] fOrElse(S T _) then
	 {DoOutput {Oz S 301}#' orelse '#{Oz T 300} P 300}
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
      [] fCond(Clauses IfElse _) then
	 PU#'cond '#{LI {Map Clauses OutputOz} NL#'[] '}#
	 case IfElse of fNoElse(_) then ""
	 else NL#'else'#IN#NL#{OzBlock IfElse}#EX
	 end#NL#PO#'end'
      [] fClause(R S T) then
	 {OutputOz R}#' in '#{OutputOz S}#
	 case T of fNoThen(_) then "" else ' then'#IN#NL#{OzBlock T}#EX end
      [] fOr(Clauses Type _) then
	 PU#case Type of fchoice then 'choice '
	    [] fdis then 'dis '
	    [] 'for' then 'or '
	    end#{LI {Map Clauses OutputOz} NL#'[] '}#NL#PO#'end'
      [] fBoolCase(E T1 T2 _) then
	 PU#'if '#{OutputOz E}#' then'#IN#NL#{OzBlock T1}#EX#
	 case T2 of fNoElse(_) then ""
	 else NL#'else'#IN#NL#{OzBlock T2}#EX
	 end#NL#PO#'end'
      [] fCase(T Cs E _) then
	 PU#'case '#{OutputOz T}#' of '#
	 {LI {Map Cs OutputOz} NL#'[] '}#
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
      [] fGrammar(T Obj _) then
	 {Obj output(T $)}
      [] fFOR(Ds B _) then
	 'for '#{LI {Map Ds OutputOz} ' '}#' do '#{OutputOz B}#' end'
      [] forPattern(P G) then {OutputOz P}#' in '#{OutputOz G}
      [] forFeature(F E) then {OutputOz F}#':'#{OutputOz E}
      [] forGeneratorList(E) then {OutputOz E}
      [] forGeneratorInt(E1 E2 E3) then
	 {OutputOz E1}#'..'#{OutputOz E2}#
	 if E3==unit then nil else ';'#{OutputOz E3} end
      [] forGeneratorC(E1 E2 E3) then
	 '('#{OutputOz E1}#';'#{OutputOz E2}#
	 if E3==unit then nil else ';'#{OutputOz E3} end#')'
      [] forFrom(P G) then
	 {OutputOz P}#' from '#{OutputOz G}
      end
   end
in
   fun {OutputOz Ex}
      {Oz Ex 0}
   end
end

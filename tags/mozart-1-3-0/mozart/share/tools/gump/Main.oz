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

local
   local
      OZHOME      = {Property.get 'oz.home'}
   in
      %% include dirs for testing in bootstrap mode:
      %%	mozart/platform/emulator mozart/platform/tools/gump
      %% {OZTOOL} returns a vs naming the oztool executable
      fun {OZTOOL}
	 case {Property.condGet 'oz.exe.oztool' unit} of unit
	 then case {OS.getEnv 'OZTOOL'} of false then 'oztool'
	      elseof X then X end
	 elseof X then X end
      end
      %% {OZTOOLINC} returns a vs consisting of -Idir elements
      fun {OZTOOLINC}
	 case {Property.condGet 'oz.inc.oztool' unit} of unit
	 then case {OS.getEnv 'OZTOOL_INCLUDES'} of false then ''
	      elseof X then X end
	 elseof X then X end
      end
      %% {OZFLEX} returns a vs naming the flex.exe executable
      fun {OZFLEX}
	 case {Property.condGet 'oz.exe.flex' unit} of unit
	 then case {OS.getEnv 'OZFLEX'} of false
	      then
		 OZHOME#'/platform/'#{Property.get 'platform.name'}#'/flex.exe'
	      elseof X then X end
	 elseof X then X end
      end
   end

   %--------------------------------------------------------------------
   % Auxiliary Functions and Classes
   %--------------------------------------------------------------------

   local
      fun {MakeFileNameSub S Ext}
	 case S of &/|Cr then &%|{MakeFileNameSub Cr Ext}
	 elseof &%|Cr then &%|&%|{MakeFileNameSub Cr Ext}
	 elseof C|Cr then C|{MakeFileNameSub Cr Ext}
	 [] nil then Ext
	 end
      end
   in
      fun {MakeFileName fVar(X _) Ext Dir} FN in
	 FN = {MakeFileNameSub {Atom.toString X} Ext}
	 case Dir of unit then {String.toAtom FN}
	 else S in
	    S = {VirtualString.toString Dir}
	    case {Reverse S} of &/|_ then {VirtualString.toAtom Dir#FN}
	    [] &\\|_ then {VirtualString.toAtom Dir#FN}
	    else {VirtualString.toAtom Dir#'/'#FN}
	    end
	 end
      end
   end

   %--------------------------------------------------------------------
   % Auxiliary Functions for Output

   \insert ../../lib/compiler/FormatStrings

   fun {SymbolToVirtualString S}
      {FormatStringToVirtualString
       case S of fVar(X _) then pn(X)
       [] fAtom(X _) then oz(X)
       [] fDollar(_) then '$'
       [] fWildcard(_) then '_'
       end}
   end

   \insert Output

   IN = format(indent)
   EX = format(exdent)
   NL = format(break)
   PU = format(push)
   PO = format(pop)
   GL = format(glue(" "))
   fun {LI Xs Sep} list(Xs Sep) end

   proc {WriteVSFile VS Name}
      T = {Thread.this}
      RaiseOnBlock = {Debug.getRaiseOnBlock T}
      {Debug.setRaiseOnBlock T false}
      File = {New Open.file init(name: Name flags: [write create truncate])}
   in
      {File write(vs: VS)}
      {File close()}
      {Debug.setRaiseOnBlock T RaiseOnBlock}
   end

   %--------------------------------------------------------------------
   % Auxiliary Functions on Oz Terms, Expressions and Variables

   \insert ../../lib/compiler/TupleSyntax

   fun {SymbolEq S1 S2}
      case S1 of fVar(X _) then
	 case S2 of fVar(!X _) then true else false end
      [] fAtom(X _) then
	 case S2 of fAtom(!X _) then true else false end
      end
   end

   fun {SymbolSetUnion Xs Ys}
      {FoldL Xs
       fun {$ Zs S}
	  if {Some Zs fun {$ T} {SymbolEq S T} end} then Zs
	  else S|Zs
	  end
       end Ys}
   end

   fun {SymbolSetIntersection Xs Ys}
      {Filter Xs fun {$ S} {Some Ys fun {$ T} {SymbolEq S T} end} end}
   end

   fun {GetMultipleSymbols Ss}
      case Ss of S|Sr then Ts Fs in
	 {List.partition Sr fun {$ T} {SymbolEq S T} end ?Ts ?Fs}
	 case Ts of _|_ then S|{GetMultipleSymbols Fs}
	 [] nil then {GetMultipleSymbols Fs}
	 end
      [] nil then nil
      end
   end

   local
      class VariableGeneratorClass
	 prop final
	 attr counter: ~1
	 meth init()
	    skip
	 end
	 meth generate($)
	    counter <- @counter + 1
	    {VirtualString.toAtom 'Syn'#@counter}
	 end
      end

      VariableGenerator = {New VariableGeneratorClass init()}
   in
      fun {Fresh}
	 {VariableGenerator generate($)}
      end
   end

   %--------------------------------------------------------------------
   % Auxiliary Functions for Code Generation

   fun {ValueToAST X}
      if {IsLiteral X} then fAtom(X unit)
      elseif {IsInt X} then fInt(X unit)
      elseif {IsTuple X} then
	 fRecord({ValueToAST {Label X}}
		 {Record.foldR X fun {$ V In} {ValueToAST V}|In end nil})
      elseif {IsRecord X} then
	 fRecord({ValueToAST {Label X}}
		 {Record.foldRInd X
		  fun {$ F V In} fColon({ValueToAST F} {ValueToAST V})|In end
		  nil})
      else
	 {Exception.raiseError gump(internal valueToAST)} unit
      end
   end

   %-----------------------------------------------------------------------
   % Management of Class Descriptors in Gump Specifications
   %-----------------------------------------------------------------------

   class ClassDescriptors
      attr
	 From: nil
	 Prop: nil
	 Attr: nil
	 Feat: nil
	 Meth: nil
	 Flags: unit
      meth setFlags(Rec)
	 Flags <- Rec
      end
      meth getFlag(X $)
	 @Flags.X
      end
      meth getFlexOptions($) S = @Flags in
	 if S.caseless then ' caseless' else "" end#
	 if S.bestfit then ' subset-sort' else "" end#
	 if S.backup then ' backup' else "" end#
	 if S.perfreport then ' perf-report' else "" end#
	 if S.statistics then ' verbose' else "" end#
	 if S.nowarn then ' nowarn' else "" end
      end
      meth enterFrom(Fs)
	 From <- {Append @From Fs}
      end
      meth enterProp(Ps)
	 Prop <- {Append @Prop Ps}
      end
      meth enterAttr(As)
	 Attr <- {Append @Attr As}
      end
      meth enterFeat(Fs)
	 Feat <- {Append @Feat Fs}
      end
      meth getDescrs($)
	 {Flatten
	  case @From of Fs=_|_ then [fFrom(Fs unit)] [] nil then nil end|
	  case @Prop of Ps=_|_ then [fProp(Ps unit)] [] nil then nil end|
	  case @Attr of As=_|_ then [fAttr(As unit)] [] nil then nil end|
	  case @Feat of Fs=_|_ then [fFeat(Fs unit)] [] nil then nil end|
	  nil}
      end
      meth enterMeth(Ms)
	 Meth <- {Append Ms @Meth}
      end
      meth getMeth($)
	 @Meth
      end
   end
in
   \insert ScannerGenerator
   \insert ParserGenerator
end

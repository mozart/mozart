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

local
   local
      OZHOME = {System.get home}
      X#Y = {System.get platform}
   in
      PLATFORMDIR = OZHOME#'/platform/'#X#'-'#Y
      INCLUDEDIR = OZHOME#'/include'
   end

   \insert ../compiler/Misc
   \insert ../compiler/FormatStrings

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
      fun {MakeFileName fVar(X _) Ext}
	 {String.toAtom {MakeFileNameSub {Atom.toString X} Ext}}
      end
   end

   %--------------------------------------------------------------------
   % Auxiliary Functions for Output

   \insert Output

   IN = format(indent)
   EX = format(exdent)
   NL = format(break)
   PU = format(push)
   PO = format(pop)
   GL = format(glue(" "))
   fun {LI Xs Sep} format(list(Xs Sep)) end

   proc {WriteVSFile VS Name}
      T = {Thread.this}
      RaiseOnBlock = {Thread.getRaiseOnBlock T}
      {Thread.setRaiseOnBlock T false}
      File = {New Open.file init(name: Name flags: [write create truncate])}
   in
      {File write(vs: VS)}
      {File close()}
      {Thread.setRaiseOnBlock T RaiseOnBlock}
   end

   %--------------------------------------------------------------------
   % Auxiliary Functions on Oz Terms, Expressions and Variables

   \insert ../compiler/TupleSyntax

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
	  case {Some Zs fun {$ T} {SymbolEq S T} end} then Zs
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
	    {ConcatenateAtomAndInt 'Syn' @counter}
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
      case {IsAtom X} then fAtom(X unit)
      elsecase {IsInt X} then fInt(X unit)
      elsecase {IsTuple X} then
	 fRecord({ValueToAST {Label X}}
		 {Record.foldR X fun {$ V In} {ValueToAST V}|In end nil})
      elsecase {IsRecord X} then
	 fRecord({ValueToAST {Label X}}
		 {Record.foldRInd X
		  fun {$ F V In} fColon({ValueToAST F} {ValueToAST V})|In end
		  nil})
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
	 case S.caseless then ' caseless' else "" end#
	 case S.bestfit then ' subset-sort' else "" end#
	 case S.backup then ' backup' else "" end#
	 case S.perfreport then ' perf-report' else "" end#
	 case S.statistics then ' verbose' else "" end#
	 case S.nowarn then ' nowarn' else "" end
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

   \insert ScannerGenerator
   \insert ParserGenerator
in
   Gump = gump(makeProductionTemplates: MakeProductionTemplates
	       transformScanner: TransformScanner
	       transformParser: TransformParser)
end

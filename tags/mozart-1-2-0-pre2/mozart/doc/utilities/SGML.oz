%%%
%%% Authors:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Denys Duchier <duchier@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
%%%   Denys Duchier, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   Property(get)
   Parser at 'x-oz://contrib/doc/sgml/Parser'
export
   Parse
   namePI: PI
   GetSubtree
   IsOfClass
define
   ParseError = 'sgml parse error'
   CrossRefWarning = 'sgml cross-reference warning'

   PI = {NewName}

   proc {OutputParseErrors S Reporter} Line Lines in
      {List.takeDropWhile S fun {$ C} C \= &\n end ?Line ?Lines}
      try R1 FileName R2 LineNumber R3 ColumnNumber R4 Meth Kind Msg in
	 Line = &n|&s|&g|&m|&l|&s|&:|R1
	 {List.takeDropWhile R1 fun {$ C} C \= &: end ?FileName &:|?R2}
	 {List.takeDropWhile R2 fun {$ C} C \= &: end ?LineNumber &:|?R3}
	 {List.takeDropWhile R3 fun {$ C} C \= &: end ?ColumnNumber &:|?R4}
	 Meth#Kind#Msg = case R4 of &E|&:|& |R then error#ParseError#R
			 elseof &X|&:|& |R then warn#CrossRefWarning#R
			 elseof & |R then error#ParseError#R
			 end
	 {Reporter Meth(coord: pos({String.toAtom FileName}
				   {String.toInt LineNumber}
				   {String.toInt ColumnNumber})
			kind: Kind
			msg: Msg)}
      catch _ then
	 {Reporter error(kind: ParseError msg: Line)}
      end
      case Lines of [&\n] then skip
      elseof &\n|Rest then {OutputParseErrors Rest Reporter}
      elseof nil then skip
      elseof X then {OutputParseErrors X Reporter}
      end
   end

   fun {Parse File Reporter} Res Errors in
      {Parser.object
       process([File] ?Res
	       catalog:{Property.get 'ozdoc.catalog'}
	       casefold:lower
	       include:{Property.get 'ozdoc.include'}
	       error:Errors)}
      if Errors \= nil then
	 {OutputParseErrors Errors Reporter}
      end
      if {Reporter hasSeenError($)} then unit
      else {Transform Res.docElem}
      end
   end

   fun {GetSubtree M L ?Mr}
      if {IsTuple M} then
	 case {Record.toList M} of (X=L(...))|Xr then
	    Mr = {List.toTuple {Label M} Xr}
	    X
	 else
	    Mr = M
	    unit
	 end
      else
	 {GetSubtree
	  {List.toRecord {Label M}
	   {Filter {Record.toListInd M} fun {$ X#_} {IsInt X} end}} L ?Mr}
      end
   end

   fun {IsOfClass M C}
      {Member C {CondSelect M 'class' nil}}
   end

   fun {Transform E}
      case E
      of element(tag:T attributes:A linkAttributes:L children:C) then
	 {List.toRecord T
	  {Append {List.mapInd C fun {$ I C} I#{Transform C} end}
	   {FoldR L TransformAttribute
	    {FoldR A TransformAttribute nil}}}}
      [] pi(Name) then PI(Name)
      [] data(Bytes) then {ByteString.toString Bytes}
      end
   end

   fun {TransformAttribute A L}
      Prop = A.name Kind = A.kind Value = A.value
   in
      if Kind=='IMPLIED' then L
      else
	 (Prop#if Prop=='class' then Value
	       else
		  case Value of [V] then
		     if {ByteString.is V}
		     then {ByteString.toString V}
		     else V
		     end
		  end
	       end)|L
      end
   end
end

%%%
%%% Author:
%%%   Thorsten Brunklaus <bruni@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Thorsten Brunklaus, 2001
%%%
%%% Last Change:
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
functor $
import
   OS(getCWD)
   Pickle(save)
   Word at 'x-oz://boot/Word.ozf'
export
   'createFuncs'   : CreateFuncs
   'cresateFields' : CreateFields
define
   fun {ToS V}
      {ByteString.make {VirtualString.toString V}}
   end

   fun {MakeCType Type}
      case Type
      of type(Name Ptrs) then 'TYPE'({ToS Name} {ToS Ptrs})
      end
   end

   local
      fun {Search V Is}
	 case Is
	 of item(Name Val)|Ir then
	    if {ToS V} == {ToS Name}
	    then Val else {Search V Ir} end
	 [] nil               then ({Pow 2 32} - 1)
	 end
      end
   in
      fun {ComputeVal V Items}
	 if {IsInt V}
	 then V
	 elseif {IsAtom V}
	 then {ComputeVal {Search V Items} Items}
	 elsecase V
	 of 'inclor(expr, expr)'(...) then
	    Vs = {Record.toList V}
	 in
	    {Word.toInt
	     {FoldL Vs
	      fun {$ E X}
		 {Word.orb E {Word.make 32 {ComputeVal X Items}}}
	      end {Word.make 32 0}}}
	 [] 'un_op cast_exp'('-' A) then
	    if A == '1' then ({Pow 2 32} - 1) else 1 end
	 end
      end
   end

   fun {MakeValue Value Items}
      Val = {ComputeVal Value Items}
   in
      if {IsInt Value} then 'SOME'(Val) else 'NONE' end
   end
   
   fun {MakeArgs Args}
      {Map Args fun {$ A}
		   case A
		   of arg(Type Name) then
		      'ARG'({MakeCType Type} {ToS Name})
		   end
		end}
   end

   fun {MakeStructItems Items}
      {Map Items fun {$ I}
		    case I
		    of item(text(Name) Ptrs Alias) then
		       '#'(name:{ToS Name} ptrs:{ToS Ptrs} alias:{ToS Alias})
		    end
		 end}
   end

   fun {MakeEnumItems Items}
      {Map Items
       fun {$ I}
	  case I
	  of item(Name Value) then
	     '#'(field:{ToS Name} value: {MakeValue Value Items})
	  end
       end}
   end
   
   local
      fun {ConvertFunction Key Value}
	 case Value
	 of function(Name RetType Args) then
	    FuncData = '#'(name:{ToS Name}
			   ret:{MakeCType RetType}
			   args: {MakeArgs Args})
	 in
	    'FUNCTION'({ToS Name} FuncData)
	 [] _ then unit
	 end
      end
      fun {ConvertStruct Key Value}
	 case Value
	 of struct(Items) then
	    'STRUCTURE'({ToS Key} {MakeStructItems Items}) 
	 [] _ then unit
	 end
      end
      fun {ConvertEnum Key Value}
	 case Value
	 of enum(Items) then
	    'ENUM'({ToS Key} {MakeEnumItems Items})
	 [] _ then unit
	 end
      end
      fun {ConvertAlias Key Value}
	 case Value
	 of alias(A B) then 'ALIAS'({ToS A} {ToS B})
	 [] _          then unit
	 end
      end
   in
      fun {Convert Item}
	 case Item
	 of Key#Value then
	    case Value
	    of function(...) then {ConvertFunction Key Value}
	    [] struct(...)   then {ConvertStruct Key Value}
	    [] enum(...)     then {ConvertEnum Key Value}
	    [] alias(...)    then {ConvertAlias Key Value}
	    [] _             then unit
	    end
	 end
      end
   end
   
   proc {CreateFuncs AllTypes}
      AliceValues = {FoldL {Dictionary.items AllTypes}
		     fun {$ E X}
			case {Convert X}
			of unit then E
			[] V    then V|E
			end
		     end nil}
      AliceFile   = {VirtualString.toString {OS.getCWD}#"/"#"Tree.ozp"} 
   in
      {Pickle.save AliceValues AliceFile}
   end
   proc {CreateFields _}
      skip
   end
end

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
   System(show)
export
   'collect' : Collect
define
   Types = {Dictionary.new}

   fun {ToString X}
      {VirtualString.toString X}
   end
   fun {IsTypeDef D}
      {IsTuple D} andthen {Label D} == 'stor_spec/decl_spec'
      andthen D.1 == 'typedef'
   end

   local
      fun {CheckFunDecl Is}
	 case Is
	 of 'pointer decl'(_ Is) then {CheckFunDecl Is}
	 [] 'decla(pars)'(...)   then true
	 [] 'decla()'(...)       then true
	 [] 'decla(ids)'(...)    then true
	 else false
	 end
      end
   in
      fun {IsFunTypeDef Is}
	 {IsTuple Is} andthen {CheckFunDecl Is}
      end
   end

   local
      fun {CheckStruct T}
	 case {Label T}
	 of 'named struct/union' then true
	 else false
	 end
      end
   in
      fun {IsStructAliasing T}
	 {IsTuple T} andthen {CheckStruct T}
      end
   end

   local
      fun {CheckEnum T}
	 case {Label T}
	 of 'enum{decls}' then true
	 else false
	 end
      end
   in
      fun {IsEnumDef T}
	 {IsTuple T} andthen {CheckEnum T}
      end
   end

   local
      fun {CheckStruct T}
	 case {Label T}
	 of 'struct{decls}'       then true
	 [] 'named struct{decls}' then true
	 else false
	 end
      end
   in
      fun {IsStructDef T}
	 {IsTuple T} andthen {CheckStruct T}
      end
   end
   
   fun {CollectName N}
      case N
      of 'type_spec/decl_spec'(A B) then {CollectName A}#' '#{CollectName B}
      [] 'stor_spec/decl_spec'(A B) then {CollectName A}#' '#{CollectName B}
      [] 'func_spec/decl_spec'(A B) then {CollectName A}#' '#{CollectName B}
      [] 'named struct/union'(A B)  then A#' '#B
      [] Id                         then Id
      end
   end
   
   local
      fun {CollectStars S}
	 case S
	 of '*'    then '*'
	 [] '*'(S) then '*'#{CollectStars S}
	 end
      end
   in
      fun {CollectPtrs Is}
	 case Is
	 of 'pointer decl'(S SB) then {CollectStars S}#{CollectPtrs SB}
	 [] 'decla[ass]'(...)    then '*'
	 [] '[ass]'(...)         then '*'
	 [] '*'                  then Is
	 [] _                    then ''
	 end
      end
   end

   fun {FindAlias T}
      case T
      of 'pointer decl'(_ Id)     then Id
      [] 'decla[qual_list]'(Id _) then Id
      [] Id                       then Id
      end
   end

   local
      local
	 fun {ToInt X}
	    {String.toInt {Atom.toString X}}
	 end
	 fun {DoMul X Y}
	    if Y == 0 then X else {DoMul (X * 2) (Y - 1)} end
	 end
	 fun {TryToInt X}
	    try {ToInt X} catch _ then X end
	 end
      in
	 fun {ComputeExp E}
	    case E
	    of '<<(expr, expr)'(X Y) then {DoMul {ToInt X} {ToInt Y}}
	    [] E                     then
	       if {IsAtom E} then {TryToInt E} else E end
	    end
	 end
      end

      fun {PrepareItems I Xs}
	 case Xs
	 of nil  then nil
	 [] X|Xr then
	    Item = case X
		   of 'enumerator(list, expr)'(Id Exp) then
		      item({ToString Id} {ComputeExp Exp})
		   [] Id                               then
		      item({ToString Id} I)
		   end
	 in
	    Item|{PrepareItems (I + 1) Xr}
	 end
      end
   in
      proc {RegisterEnumDef S Id}
	 Ls = case S.1
	      of 'enumerator(list, expr)'(_ _) then [S.1]
	      [] 'enumerator_list'(...)        then {Record.toList S.1}
	      [] Id                            then [Id]
	      end
      in
	 {Dictionary.put Types Id enum({PrepareItems 0 Ls})}
      end
   end

   local
      fun {CollectFunName Is}
	 case Is
	 of 'pointer decl'(_ Is) then {CollectFunName Is}
	 [] 'decla(pars)'(Id _)  then Id
	 [] 'decla()'(Id)        then Id
	 end
      end

      fun {CollectArgDecl Is}
	 case Is
	 of 'decl_spec decl'(D Is) then
	    ArgType = type({ToString {CollectName D}}
			   {ToString {CollectPtrs Is}})
	 in
	    arg(ArgType {FindAlias Is})
	 [] 'decl_spec absdecl'(D Is) then
	    ArgType = type({ToString {CollectName D}}
			   {ToString {CollectPtrs Is}})
	 in
	    arg(ArgType {FindAlias Is})
	 [] 'type_spec/decl_spec'(...) then
	    arg(type({ToString {CollectName Is}}
		     {ToString {CollectPtrs Is}}) '')
	 [] Id then
	    arg(type({ToString Id} nil) '')
	 end
      end

      fun {CollectFunArgs Is}
	 case Is
	 of 'pointer decl'(_ Is) then {CollectFunArgs Is}
	 [] 'decla(pars)'(_ Is)  then {CollectFunArgs Is}
	 [] 'decla()'(_)         then nil
	 [] 'pars decl'(...)     then {Map {Record.toList Is} CollectArgDecl}
	 [] '...'(Is)            then {Append {CollectFunArgs Is}
				       [arg(type("..." nil) _)]}
	 [] Is                   then [{CollectArgDecl Is}]
	 end
      end
   in
      proc {RegisterFunction D Is}
	 RetName = {CollectName D}
	 RetPtrs = {CollectPtrs Is}
	 FunName = {CollectFunName Is}
	 Args    = {CollectFunArgs Is}
	 RetType = type({ToString RetName} {ToString RetPtrs})
      in
	 {Dictionary.put Types FunName
	  function({ToString FunName} RetType Args)}
      end
   end
   
   proc {RegisterStructAlias S Id}
      Name = {CollectName S.2}
      Ptrs = {CollectPtrs Id}
   in
      {Dictionary.put Types {FindAlias Id} alias(Name Ptrs)}
   end
   
   proc {RegisterPlainType Ns Is}
      Name = {CollectName Ns}
      Ptrs = {CollectPtrs Is}
      Type = type({ToString Name} {ToString Ptrs})
   in
      {Dictionary.put Types {FindAlias Is} Type}
   end

   local
      fun {CheckFunc F}
	 case F
	 of 'pointer decl'(_ F) then {CheckFunc F}
	 [] 'decla(pars)'(...)  then true
	 [] 'decla()'(...)      then true
	 else false
	 end
      end
   in
      fun {IsFunctionDef Is}
	 {IsTuple Is} andthen {CheckFunc Is}
      end
   end
   
   fun {IsExternRef D}
      {IsTuple D} andthen {Label D} == 'stor_spec/decl_spec'
      andthen D.1 == 'extern'
   end

   fun {IsExternGtkType D Is}
      D == 'stor_spec/decl_spec'('extern' 'GtkType') andthen {IsAtom Is}
   end

   proc {RegisterConstant Is}
      Cs = {Dictionary.condGet Types gtk_const_kit nil}
   in
      {Dictionary.put Types gtk_const_kit Is|Cs}
   end
   
   RegisterStructDef
   
   proc {CollectDecl D Is}
      if {IsTypeDef D}
      then
	 if {IsFunTypeDef Is}
	 then skip
	 elseif {IsEnumDef D.2}
	 then {RegisterEnumDef D.2 Is}
	 elseif {IsStructDef D.2}
	 then {RegisterStructDef D.2 Is}
	 elseif {IsStructAliasing D.2}
	 then {RegisterStructAlias D.2 Is}
	 else {RegisterPlainType D.2 Is}
	 end
      elseif {IsExternGtkType D Is}
      then {RegisterConstant Is}
      elseif {IsExternRef D}
      then skip
      elseif {IsFunctionDef Is}
      then {RegisterFunction D Is}
      else raise error(collectDecl D Is) end
      end
   end

   local
      fun {FoldSpecs Ss I N}
	 if I =< N
	 then
	    Space = if I == N then '' else ' ' end
	 in
	    Ss.I#Space#{FoldSpecs Ss (I + 1) N}
	 else ''
	 end
      end

      PrepareItems

      fun {HandleStructItem T}
	 Items = {PrepareItems {Record.toList T.2}}
	 Name  = {NewName}
      in
	 {Dictionary.put Types Name
	  case T.1 of 'union' then union(Items) else 'struct'(Items) end}
	 Name
      end
      
      fun {CollectSpecs Ss}
	 case Ss
	 of 'specifier_list'(...)     then {FoldSpecs Ss 1 {Width Ss}}
	 [] 'named struct/union'(T I) then T#' '#I
	 [] 'struct{decls}'(...)      then {HandleStructItem Ss}
	 [] Id                        then Id
	 end
      end

      fun {!PrepareItems Is}
	 case Is
	 of nil  then nil
	 [] I|Ir then
	    case I
	    of 'struct_item'(Ss Is) then
	       Name = {CollectSpecs Ss}
	       Ptrs = {CollectPtrs Is}
	    in
	       item(if {IsName Name} then name(Name)
		    else text({ToString Name}) end
		    {ToString Ptrs}
		    {FindAlias Is})
	    end|{PrepareItems Ir}
	 end
      end

      fun {ExplodeItems Is}
	 case Is
	 of I|Ir then
	    case I
	    of item(A B struct_decl_list(...)) then
	       {Append {Map {Record.toList I.3} fun {$ N} item(A B N) end}
		{ExplodeItems Ir}}
	    [] _ then I|{ExplodeItems Ir}
	    end
	 [] nil then nil
	 end
      end
   in
      proc {CollectNamedStruct T}
	 ItemTup  = T.3
	 RawItems = case {Label ItemTup}
		    of 'struct_decls' then
		       {PrepareItems {Record.toList ItemTup}}
		    [] 'struct_item'  then {PrepareItems [ItemTup]}
		    end
	 Items    = {ExplodeItems RawItems}  
      in
	 {Dictionary.put Types T.2
	  case T.1 of 'union' then union(Items) else 'struct'(Items) end}
      end
      proc {!RegisterStructDef S Id}
	 ItemTup = if {Label S}  == 'named struct{decls}' then S.3 else S.2 end
	 RawItems = case {Label ItemTup}
		   of 'struct_decls' then
		      {PrepareItems {Record.toList ItemTup}}
		   [] 'struct_item'  then {PrepareItems [ItemTup]}
		    end
	 Items = {ExplodeItems RawItems}
      in
	 {Dictionary.put Types Id
	  case S.1 of 'union' then union(Items) else struct(Items) end}
      end
   end
   
   fun {Collect Ts}
      case Ts
      of nil  then Types
      [] T|Tr then
	 case T
	 of'decl_spec/init_decls'(D Is) then {CollectDecl D Is}
	 [] 'named struct{decls}'(...)  then {CollectNamedStruct T}
	 [] 'named struct/union'(...)   then skip
	 [] 'enum{decls}'(...)          then {RegisterEnumDef T {NewName}}
	 [] 'function s'(RT Args _)     then {CollectDecl RT Args}
	 [] T                           then raise error(collect T) end
	 end
	 {Collect Tr}
      end
   end
end

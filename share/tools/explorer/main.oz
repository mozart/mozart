%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local

   local
      MoveProc = {Tk.getId}
   in

      {Tk.send
       v('proc '#MoveProc#' {c b is} { ' #
         '   foreach i $is { ' #
         '      $c move $i $b 0' #
         '   } ' #
         '}\n')}
			   
      proc {MoveTree C ByX Is}
	 {Tk.send o(MoveProc C ByX q(b(Is)))}
      end
   end
   
   \insert misc.oz
   
   \insert configure.oz

   \insert manager.oz

   NoLabel       = {NewName}
   ManagerClosed = {NewName}
   
   ActionKinds   = [information compare statistics]
   ActionTypes   = [root space procedure]
   ActionArities = a(information: [2 3]
		     compare:     [4 5]
		     statistics:  [2 3])

   fun {SpaceToProcedure S}
      fun {$}
	 {Space.merge {Space.clone S}}
      end
   end

   fun {SpaceToSpace S} S end

   SpaceToRoot = Space.merge
   
in

   class ExplorerClass
      prop locking final

      feat
	 Options
      attr
	 Stacked:   nil
         MyManager: unit

      meth init
	 self.Options = {Record.map DefOptions Record.toDictionary}
      end
      
      meth Init()
	 case @MyManager\=unit then skip else
	    MyManager <- {New Manager init(self self.Options)}
	    %% Include the standard actions
               \insert default-actions.oz
	    {ForAll {Reverse @Stacked} self}
	    Stacked <- nil
	 end
      end

      meth !ManagerClosed()
	 lock
	    MyManager <- unit
	 end
      end
      
      meth script(Script Order <=false)
	 lock
	    ExplorerClass,Init
	    {@MyManager query(proc {$ X} {Script X} end Order)}
	 end
      end

      meth one(Script Order <=false)
	 lock
	    ExplorerClass,script(Script Order)
	    {@MyManager.menu.search.next tk(invoke)}
	 end
      end

      meth all(Script Order <=false)
	 lock
	    ExplorerClass,script(Script Order)
	    {@MyManager.menu.search.all tk(invoke)}
	 end
      end


      meth add(Kind What
	       label: Label <=NoLabel
	       type:  Type  <=root) = Add
	 lock
	    case @MyManager==unit then Stacked <- Add|@Stacked
	    elsecase
	       case {Member Kind ActionKinds} then
		  ActionMenu = @MyManager.case Kind
					  of information then infoAction
					  [] compare then cmpAction
					  [] statistics then statAction
					  end
	       in
		  case What==separator then
		     {ActionMenu addSeparator} true
		  elsecase
		     {IsProcedure What} andthen
		     {Member {ProcedureArity What} ActionArities.Kind} andthen
		     {Member Type ActionTypes} andthen
		     (Label==NoLabel orelse {IsVirtualString Label}) 
		  then
		     MenuLabel   = case Label==NoLabel then
				      {System.printName What}
				   else Label
				   end
		     SpaceToType = case Type
				   of root then SpaceToRoot
				   [] procedure then SpaceToProcedure
				   [] space then SpaceToSpace
				   end
		  in
		     {ActionMenu add(label: MenuLabel
				     value: What
				     type:  SpaceToType)}
		     true
		  else false
		  end
	       else false
	       end
	    then skip
	    else
	       {`RaiseError` explorer(actionAdd Add)}
	    end
	 end
      end

      meth delete(Kind What) = Del
	 lock
	    case @MyManager==unit then Stacked <- Del|@Stacked
	    elsecase
	       case {Member Kind ActionKinds} then
		  ActionMenu = @MyManager.case Kind
					  of information then infoAction
					  [] compare then cmpAction
					  [] statistics then statAction
					  end
	       in
		  case What
		  of all then {ActionMenu deleteAll} true
		  elsecase {IsProcedure What} then {ActionMenu delete(What)} true
		  else false
		  end
	       else false
	       end
	    then skip
	    else
	       {`RaiseError` explorer(actionDel Del)}
	    end
	 end
      end

      meth option(What ...) = OM
	 lock
	    case
	       case
		  What==postscript andthen
		  {List.sub {Arity OM} [1 color orientation size]}
	       then O=self.Options.postscript in
		  case {HasFeature OM size} then
		     case {CheckSize {VirtualString.toString OM.size}}
		     of false then false
		     elseof S then
			{Dictionary.put O width  S.width}
			{Dictionary.put O height S.height}
			{Dictionary.put O size   OM.size}
			true
		     end
		  else true end
		  andthen
		  case {HasFeature OM color} then
		     case OM.color
		     of full      then {Dictionary.put O color color} true
		     [] grayscale then {Dictionary.put O color gray}  true
		     [] bw        then {Dictionary.put O color mono}  true
		     else false
		     end
		  else true end
		  andthen
		  case {HasFeature OM orientation} then
		     case OM.orientation
		     of portrait  then {Dictionary.put O orientation false} true
		     [] landscape then {Dictionary.put O orientation true} true
		     else false
		     end
		  else true end
	       elsecase
		  What==search andthen
		  {List.sub {Arity OM} [1 failed information search]}
	       then O=self.Options.search in
		  case {HasFeature OM search} then S=OM.search in
		     case S
		     of none then {Dictionary.put O search 1} true
		     [] full then {Dictionary.put O search ~1} true
		     elsecase {IsInt S} then {Dictionary.put O search S} true
		     else false
		     end
		  else true end
		  andthen
		  case {HasFeature OM information} then I=OM.information in
		     case I
		     of none then {Dictionary.put O information 1} true
		     [] full then {Dictionary.put O information ~1} true
		     elsecase {IsInt I} then {Dictionary.put O information I} true
		     else false
		     end
		  else true end
		  andthen
		  case {HasFeature OM failed} then F=OM.failed in
		     case {IsBool F} then {Dictionary.put O failed F} true
		     else false
		     end
		  else true
		  end
	       elsecase
		  What==drawing andthen
		  {List.sub {Arity OM} [1 hide scale update]}
	       then O=self.Options.drawing in
		  case {HasFeature OM hide} then H=OM.hide in
		     case {IsBool H} then {Dictionary.put O hide H} true
		     else false
		     end
		  else true end
		  andthen
		  case {HasFeature OM scale} then S=OM.scale in
		     case {IsBool S} then {Dictionary.put O scale S} true
		     else false
		     end
		  else true end
		  andthen
		  case {HasFeature OM update} then U=OM.update in
		     case {IsInt U} andthen {IsNat U} then
			{Dictionary.put O update U} true
		     else false
		     end
		  else true end
	       else false
	       end
	    then
	       case @MyManager of unit then skip elseof M then
		  {M updateAfterOption}
	       end
	    else
	       {`RaiseError` explorer(option OM)}
	    end
	 end
      end

      meth close
	 lock
	    case @MyManager of unit then skip
	    elseof M then
	       MyManager <- unit
	       {M closeByMain}
	    end
	 end
      end
      
   end
   
end

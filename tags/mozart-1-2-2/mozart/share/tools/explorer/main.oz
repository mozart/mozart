%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
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
   
   \insert 'configure-dynamic.oz'

   \insert 'manager.oz'

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
	 if @MyManager==unit then
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
      
      meth Exec(Script Order Action)
	 lock
	    ExplorerClass,Init
	    {@MyManager script(proc {$ X} {Script X} end Order Action)}
	 end
      end

      meth script(Script Order <=false)
	 ExplorerClass,Exec(Script Order 'skip')
      end

      meth one(Script Order <=false)
	 ExplorerClass,Exec(Script Order 'next')
      end

      meth all(Script Order <=false)
	 ExplorerClass,Exec(Script Order 'all')
      end

\define CS_SPECIAL

\ifdef CS_SPECIAL
      meth sYnc(Script Order <= false)
	 lock
	    {@MyManager script(proc {$ X} {Script X} end Order 'next')}
	    {Wait {Tk.return update(idletasks)}}
	 end
      end
\endif
      
      meth add(Kind What
	       label: Label <=NoLabel
	       type:  Type  <=root) = Add
	 lock
	    if @MyManager==unit then Stacked <- Add|@Stacked
	    elseif
	       if {Member Kind ActionKinds} then
		  ActionMenu = @MyManager.case Kind
					  of information then infoAction
					  [] compare then cmpAction
					  [] statistics then statAction
					  end
	       in
		  if What==separator then
		     {ActionMenu addSeparator} true
		  elseif
		     {IsProcedure What} andthen
		     {Member {ProcedureArity What} ActionArities.Kind} andthen
		     {Member Type ActionTypes} andthen
		     (Label==NoLabel orelse {IsVirtualString Label}) 
		  then
		     MenuLabel   = if Label==NoLabel then
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
	       {Exception.raiseError explorer(actionAdd Add)}
	    end
	 end
      end

      meth delete(Kind What) = Del
	 lock
	    if @MyManager==unit then Stacked <- Del|@Stacked
	    elseif
	       if {Member Kind ActionKinds} then
		  ActionMenu = @MyManager.case Kind
					  of information then infoAction
					  [] compare then cmpAction
					  [] statistics then statAction
					  end
	       in
		  case What
		  of all then {ActionMenu deleteAll} true
		  else
		     if {IsProcedure What} then {ActionMenu delete(What)} true
		     else false
		     end
		  end
	       else false
	       end
	    then skip
	    else
	       {Exception.raiseError explorer(actionDel Del)}
	    end
	 end
      end

      meth option(What ...) = OM
	 lock
	    if
	       if
		  What==postscript andthen
		  {List.sub {Arity OM} [1 color orientation size]}
	       then O=self.Options.postscript in
		  if {HasFeature OM size} then
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
		  if {HasFeature OM color} then
		     case OM.color
		     of full      then {Dictionary.put O color color} true
		     [] grayscale then {Dictionary.put O color gray}  true
		     [] bw        then {Dictionary.put O color mono}  true
		     else false
		     end
		  else true end
		  andthen
		  if {HasFeature OM orientation} then
		     case OM.orientation
		     of portrait  then {Dictionary.put O orientation false} true
		     [] landscape then {Dictionary.put O orientation true} true
		     else false
		     end
		  else true end
	       elseif
		  What==search andthen
		  {List.sub {Arity OM} [1 failed information search]}
	       then O=self.Options.search in
		  if {HasFeature OM search} then S=OM.search in
		     case S
		     of none then {Dictionary.put O search 1} true
		     [] full then {Dictionary.put O search ~1} true
		     else
			if {IsInt S} then {Dictionary.put O search S} true
			else false
			end
		     end
		  else true end
		  andthen
		  if {HasFeature OM information} then I=OM.information in
		     case I
		     of none then {Dictionary.put O information 1} true
		     [] full then {Dictionary.put O information ~1} true
		     else
			if {IsInt I} then {Dictionary.put O information I} true
			else false
			end
		     end
		  else true end
		  andthen
		  if {HasFeature OM failed} then F=OM.failed in
		     if {IsBool F} then {Dictionary.put O failed F} true
		     else false
		     end
		  else true
		  end
	       elseif
		  What==drawing andthen
		  {List.sub {Arity OM} [1 hide scale update]}
	       then O=self.Options.drawing in
		  if {HasFeature OM hide} then H=OM.hide in
		     if {IsBool H} then {Dictionary.put O hide H} true
		     else false
		     end
		  else true end
		  andthen
		  if {HasFeature OM scale} then S=OM.scale in
		     if {IsBool S} then {Dictionary.put O scale S} true
		     else false
		     end
		  else true end
		  andthen
		  if {HasFeature OM update} then U=OM.update in
		     if {IsInt U} andthen {IsNat U} then
			{Dictionary.put O update U} true
		     else false
		     end
		  else true end
	       elseif What==visual andthen
		  {List.sub {Arity OM} [1 title]}
	       then O=self.Options.visual in
		  if {HasFeature OM title} then T=OM.title in
		     if {VirtualString.is T} then
			{Dictionary.put O title T} true
		     else false
		     end
		  else true
		  end
	       else false
	       end
	    then
	       case @MyManager of unit then skip elseof M then
		  {M updateAfterOption}
	       end
	    else
	       {Exception.raiseError explorer(option OM)}
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

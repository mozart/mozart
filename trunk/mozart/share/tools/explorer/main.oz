%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local
 
   proc {OpiError V M}
      E = {System.get errors}
   in
      {System.showError '*** Explorer Configuration Error'} 
      {System.showError ('*** '# V # ': ' #
			 {System.valueToVirtualString M E.depth E.width})}
      {System.showError ''}
   end

   \insert misc.oz
   
   \insert configure.oz

   \insert manager.oz

   NoLabel       = {NewName}
   MyManager     = {NewName}
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
      from UrObject

      attr
	 Stacked:    nil
         MyManager:  false

      meth Init()
	 case @MyManager\=false then skip else
	    MyManager <- {New Manager init(self)}
	    ExplorerClass
	       %% Include the standard actions
               \insert default-actions.oz
	    ExplorerClass,{Reverse @Stacked}
	    Stacked <- nil
	 end
      end

      meth !ManagerClosed()
	 MyManager <- false
      end
      
      meth solver(Solver Order <=false)
	 ExplorerClass,Init
	 {@MyManager query(proc {$ X} {Solver X} end Order)}
      end

      meth one(Solver Order <=false)
	 ExplorerClass,solver(Solver Order)
	 {@MyManager.menu.search.next tk(invoke)}
      end

      meth all(Solver Order <=false)
	 ExplorerClass,solver(Solver Order)
	 {@MyManager.menu.search.all  tk(invoke)}
      end


      meth add(Kind What
	       label: Label <=NoLabel
	       type:  Type  <=root) = Add
	 case @MyManager==false then Stacked <- Add|@Stacked
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
	 else {OpiError 'Action adding' Add}
	 end
      end

      meth delete(Kind What) = Del
	 case @MyManager==false then Stacked <- Del|@Stacked
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
	 else {OpiError 'Action deletion' Del}
	 end
      end

      meth option(What ...) = OM
	 case @MyManager==false then Stacked <- OM|@Stacked
	 elsecase
	    case
	       What==postscript andthen
	       {List.sub {Arity OM} [1 color orientation size]}
	    then O=@MyManager.options.postscript in
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
		  [] grayscale then {Dictionary.put O color grey} true
		  [] bw        then {Dictionary.put O color mono} true
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
	       {List.sub {Arity OM} [1 information search]}
	    then O=@MyManager.options.search in
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
	    then O=@MyManager.options.drawing in
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
	 then {@MyManager updateAfterOptions}
	 else {OpiError 'Option configuration' OM}
	 end
      end
      
      meth close
	 case @MyManager of false then skip elseof M then {M close} end
	 UrObject,close
      end

   end
   
end

%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local
 
   \insert misc.oz
   
   \insert configure.oz

   \insert shapes-and-images.oz
   
   \insert menues.oz
   
   \insert manager.oz

   NoLabel       = {NewName}
   MyManager     = {NewName}
   ManagerClosed = {NewName}
   
   Actions = [information compare statistics]

   ActionArities = a(information: [2 3]
		     compare:     [4 5]
		     statistics:  [2 3])

   Options = {Sort [search information drawing postscript] Value.'<'}

   fun {GetMenu Manager Action}
      Menu = Manager.menu
   in
      case Action
      of information then Menu.nodes.chooseInfo
      [] compare then Menu.nodes.chooseCmp
      [] statistics then Menu.nodes.chooseStat
      end
   end
		       
   proc {MethodError O M}
      {`ooMethodError` _ M O _}
   end

   fun {GetAction M W}
      case {Width M}==W then
	 case {List.subtract {Arity M} label} of [What] then
	    case {Member What Actions} then What else False end
	 else False end
      else False end
   end

   InitialActions = {MakeRecord actions Actions}

   {Record.forAll InitialActions fun {$} nil end}
   
   fun {DeleteAction AEs B}
      case AEs of nil then nil
      [] AE|AEr then
	 case AE.1==B then {AE.2 close} AEr else AE|{DeleteAction AEr B} end
      end
   end

   fun {IfAtThen T F P}
      case {HasSubtreeAt T F} then {P T.F} else True end
   end

in

   class ExplorerClass
      from UrObject

      attr
	 Actions:    InitialActions
	 Stacked:    nil
         MyManager:  False

      meth init(one:  ?OneHandler  <= _
		all:  ?AllHandler  <= _
		best: ?BestHandler <= _)
	 proc {OneHandler Q}
	    {self [query(Q) Next]}
	 end
	 proc {AllHandler Q}
	    {self [query(Q) All]}
	 end
	 proc {BestHandler Q O}
	    {self [query(Q O) All]}
	 end
      end
      
      meth Init()
	 case @MyManager\=False then true else
	    MyManager <- {New Manager init(self)}
	    <<ExplorerClass
	       %% Include the standard actions
               \insert default-actions.oz
	    >>
	    <<ExplorerClass {Reverse @Stacked}>>
	    Stacked <- nil
	    case {Det {@MyManager sync($)}} then
	       MyManager <- @MyManager
	    end
	 end
      end

      meth !ManagerClosed()
	 Actions   <- InitialActions
	 MyManager <- False
      end
      
      meth query(Query Order <=False)
	 <<ExplorerClass Init>>
	 {@MyManager query(Query Order)}
      end

      meth add(label:Label<=NoLabel ...) = Add
	 case @MyManager==False then
	    Stacked <- Add|@Stacked
	 else
	    case {GetAction Add case Label==NoLabel then 1 else 2 end}
	    of !False then
	       {MethodError self Add}
	    elseof ActionKind then
	       case Add.ActionKind
	       of separator then
		  {New Menues.separator
		   init(parent:{GetMenu @MyManager ActionKind}.menu) _}
	       elseof Action then
		  case
		     {Procedure.is Action} andthen
		     {Member {Procedure.arity Action}
		      ActionArities.ActionKind}
		  then
		     MenuLabel = case Label==NoLabel then
				    {Procedure.printName Action}
				 else Label
				 end
		     Menu      = {GetMenu @MyManager ActionKind}
		     Entry     = {New Menues.radiobutton
				  init(parent: Menu.menu
				       label:  MenuLabel
				       value:  Action
				       group:  Menu.group.'self')}
		  in
		     {Menu.group.'self' setValue(Action)}
		     Actions <- {AdjoinAt @Actions ActionKind
				 Action#Entry|@Actions.ActionKind}
		  else
		     {MethodError self Add}
		  end
	       end
	    end
	 end
      end

      meth delete(...) = Del
	 case @MyManager==False then
	    Stacked <- Del|@Stacked
	 else
	    case {GetAction Del 1} of !False then
	       {MethodError self Del}
	    elseof ActionKind then
	       Actions <- {AdjoinAt @Actions ActionKind
			   {DeleteAction @Actions.ActionKind
			    Del.ActionKind}}
	    end
	 end
      end

      meth option(...) = Opt
	 case @MyManager==False then
	    Stacked <- Opt|@Stacked
	 else Opts={Arity Opt} in
	    case
	       {List.sub Opts Options} andthen
	       {Record.allInd Opt
		fun {$ ON O}
		   case ON
		   of search  then
		      {IfAtThen O recomputation IsInt}
		   [] information  then
		      {IfAtThen O recomputation IsInt} andthen
		      {IfAtThen O solutions IsBool}
		   [] drawing then
		      {IfAtThen O hide   IsBool} andthen
		      {IfAtThen O update IsNat}
		   [] postscript then
		      {IfAtThen O mode
		       fun {$ C} {Member C [color grayscale bw]} end} andthen
		      {IfAtThen O orientation
		       fun {$ C} {Member C [portrait landscape]} end} andthen
		      {IfAtThen O size
		       fun {$ C}
			  {IsVirtualString C} andthen
			  {Misc.check {VirtualString.toString C}}\=False
		       end}
		   end
		end}
	    then {@MyManager setOptions(Opt)}
	    else {MethodError self Opt}
	    end
	 end
      end
      
      meth Next
	 {@MyManager.menu.search.next invoke}
      end

      meth All
	 {@MyManager.menu.search.all  invoke}
      end

      meth close
	 case @MyManager of !False then true elseof M then {M close} end
	 <<UrObject close>>
      end

   end
   
end

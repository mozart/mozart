%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   fun {MakeEvent IsCtrl Key}
      case IsCtrl then '<Control-' else '<' end #
      case Key
      of '<' then less
      [] '>' then greater
      [] '+' then plus
      [] '-' then minus
      else Key
      end # '>'
   end

   local
      fun {Get E|Er F I}
	 case E==F then I else {Get Er F I+1} end
      end

      fun {Delete Es F}
	 case Es of nil then nil
	 [] E|Er then case E==F then Er else {Delete Er F} end
	 end
      end

      fun {Add Es F}
	 case Es of nil then [F]
	 [] E|Er then E|{Add Er F}
	 end
      end
   in

      class Store
	 from UrObject
	 attr Os: nil

	 meth add(O)
	    Os <- {Add @Os O}
	 end
	 meth delete(O)
	    Os <- {Delete @Os O}
	 end
	 meth getIndex(O ?I)
	    I = {Get @Os O 1}
	 end
      end

   end
   

   Master    = {NewName}
   Slaves    = {NewName}
   State     = {NewName}
   InitTcl   = {NewName}

   class Menubar
      from Tk.frame

      meth init(parent:Parent)
	 <<Tk.frame tkInit(parent:             Parent
			   relief:             raised
			   bd:                 2
			   highlightthickness: 0)>>
      end

   end

   %%
   
   class Menu
      from Tk.menu

      feat
	 !Slaves

      meth init(parent:Parent)
	 self.Slaves    = {New Store ''}
	 <<Tk.menu tkInit(parent:Parent)>>
      end

   end

   
   class MenuEntry
      from UrObject
      feat
         !Master
      
      meth close
	 {self.Master.Slaves delete(self)}
	 <<UrObject close>>
      end
   end

   
   class StateMenuEntry
      from MenuEntry
      attr
	 !State: False

      meth disable
	 case @State then M = self.Master in
	    State <- False
	    case {M.Slaves getIndex(self $)} of !False then true
	    elseof Index then
	       {Tk.send o(M entryconfigure Index o(state:disabled))}
	    end
	 else true
	 end
      end

      meth normal
	 case @State then true
	 else M = self.Master in
	    State <- True
	    case {M.Slaves getIndex(self $)} of !False then true
	    elseof Index then
	       {Tk.send o(M entryconfigure Index o(state:normal))}
	    end
	 end
      end

      meth label(L)
	 M = self.Master
      in
	 case {M.Slaves getIndex(self $)} of !False then true
	 elseof Index then
	    {Tk.send o(M entryconfigure Index o(label:L))}
	 end
      end

   end

   
   class Command
      from StateMenuEntry

      feat
	 InvokeCall
      
      meth init(parent: Parent
		label:  Label
		ctrl:   IsCtrl    <=False
		key:    Key       <=False
		action: Action
		normal: InitState <=True)
	 Call    = case {IsProcedure Action} then Action
		   else !Action=O#M in proc {$} {O M} end
		   end
	 TkState = case InitState then normal else disabled end
	 Command = {New Tk.action tkInit(parent:Parent action:Call)}	 
      in
	 {Parent.Slaves add(self)}
	 self.Master = Parent
	 State <- InitState
	 self.InvokeCall = Call
	 {Tk.send o(Parent add case Key==False then
				  command(label:   Label
					  state:   TkState       
					  command: Command)
			       else
				  command(label:       Label
					  state:       TkState       
					  command:     Command
					  accelerator: case IsCtrl then 'C-'
						       else '    '
						       end # Key)
			       end)}
      end

      meth invoke
	 case @State then {self.InvokeCall} else true end
      end
   end

   
   class Checkbutton
      from Tk.variable StateMenuEntry 
      attr OnOff:False
      feat Action

      meth init(parent: Parent
		label:  Label
		on:     InitOnOff  <= True
		normal: InitState  <= True
		action: InitAction <= proc {$} true end)
	 {Parent.Slaves add(self)}
	 self.Master = Parent
	 <<Tk.variable tkInit(case InitOnOff then on else off end)>>
	 OnOff <- InitOnOff
	 State <- InitState
	 {Tk.send o(Parent add checkbutton(label:    Label
					   command:  {New Tk.action
						      tkInit(parent: Parent
							     action: proc {$}
									{self Toggle}
								     end)}
					   state:    case InitState then normal
						     else disabled end
					   onvalue:  on
					   offvalue: off
					   variable: self))}
	 self.Action = case {IsProcedure InitAction} then InitAction
		       else !InitAction=O#M in proc {$} {O M} end
		       end
      end

      meth Toggle
	 OnOff <- {Not @OnOff}
	 {self.Action}
      end

      meth on
	 OnOff <- True
	 <<Tk.variable tkSet(on)>>
      end

      meth off
	 OnOff <- False
	 <<Tk.variable tkSet(off)>>
      end

      meth getOnOff(?IsOnOff)
	 IsOnOff = @OnOff
      end

      meth close
	 <<StateMenuEntry close>>
      end
   end

   
   class Separator
      from MenuEntry

      meth init(parent:Parent)
	 {Parent.Slaves add(self)}
	 self.Master = Parent
	 {Tk.send o(Parent add separator)}
      end
   end

   
   local
      Register      = {NewName}
      UnRegister    = {NewName}

      fun {DeleteButton Bs C D ?E ?F}
	 case Bs
	 of nil then E=D F=D
	 [] B|Br then
	    case B.1==C then
	       E = B
	       F = case D==False then
		      case Br==nil then False else Br.1 end
		   else D
		   end
	       Br
	    else B|{DeleteButton Br C B ?E ?F}
	    end
	 end
      end

      fun {Lookup B|Br C}
	 case B.2==C then B.3 else {Lookup Br C} end
      end
	    

      NoValue = {NewName}

      proc {Void} true end
      
   in
      class Group
	 from Tk.variable
	 feat
	    NoneValue NoneHandler
	 attr
	    DefaultValue: False
	    CurValue:     False
	    GroupId:      1
	    Buttons:      nil

	 meth init(value:   InitValue     <= NoValue
		   none:    InitNoneValue <= False
		   handler: Handler       <= Void)
	    <<Tk.variable tkInit(0)>>
	    DefaultValue <- InitValue
	    CurValue     <- InitNoneValue
	    self.NoneValue   = InitNoneValue
	    self.NoneHandler = Handler
	 end

	 meth !Register(Button Value ?Id)
	    case Value==@DefaultValue then
	       <<Tk.variable tkSet(0)>>
	       Id=0
	       CurValue <- Value
	    else
	       GroupId <- Id = @GroupId + 1
	       case @Buttons\=nil then true else
		  <<Tk.variable tkSet(Id)>>
		  CurValue <- Value
	       end
	    end
	    Buttons <- Button#Value#Id|@Buttons
	 end

	 meth !UnRegister(Button)
	    ToSet Found
	 in
	    Buttons <- {DeleteButton @Buttons Button False ?Found ?ToSet}
	    case @CurValue==Found.2 then
	       %% The currently selected has been deleted
	       case ToSet of !False then
		  %% It was the last guy
		  CurValue <- self.NoneValue
		  {self.NoneHandler}
	       elseof NB#NV#NI then
		  CurValue <- NV
		  <<Tk.variable tkSet(NI)>>
	       end
	    else true
	    end
	 end
	 
	 meth setValue(Value)
	    CurValue <- Value
	    <<Tk.variable tkSet({Lookup @Buttons Value})>>
	 end

	 meth getValue(?GetValue)
	    GetValue = @CurValue
	 end
      end

      
      class Radiobutton
	 from StateMenuEntry
	 feat
	    MyGroup
	    MyValue
	 
	 meth init(parent: Parent
		   label:  Label
		   normal: InitState <= True
		   group:  Group
		   action: Action <= Void
		   value:  Value)
	    {Parent.Slaves add(self)}
	    self.Master = Parent
	    State <- InitState
	    self.MyGroup = Group
	    {Tk.send o(Parent add radiobutton(label:    Label
					      variable: Group
					      value:  {Group Register(self Value $)}
					      command:  {New Tk.action
							 tkInit(parent: Parent
								action: proc {$}
									   {Group
									    setValue(Value)}
									   {Action}
									end)}))}
	 end
	 meth close
	    {self.MyGroup UnRegister(self)}
	    <<StateMenuEntry close>>
	 end
      end
   end
   
   class Cascade
      from StateMenuEntry
      
      meth init(parent: Parent
		label:  Label
		normal: InitState <=True
		menu:   Menu)
	 {Parent.Slaves add(self)}
	 self.Master = Parent
	 State <- InitState
	 {Tk.send o(Parent add cascade(label: Label
				       state: case InitState then normal
					      else disabled
					      end
				       menu:  Menu))}
      end
   end

   local
      fun {MakeEntries Es KeyBinder Parent Group}
	 case Es of nil then nil
	 [] E|Er then
	    case E==separator then
	       {New Menues.separator init(parent:Parent) _}
	       {MakeEntries Er KeyBinder Parent Group}
	    else
	       Key  = {Label E}
	       Spec = E.1
	    in
	       case {Label Spec}
	       of cascade then
		  Menu    = {New Menues.menu init(parent:Parent)}
		  Cascade = {New Menues.cascade
			     {Adjoin Spec init(menu:   Menu
					       parent: Parent)}}
	       in
		  Key#{List.toRecord menu 'self'#Cascade|menu#Menu|
		       {MakeEntries Spec.menu KeyBinder Menu Group}}
	       [] group then
		  Group = {New Menues.group
			   {Adjoin {Record.subtract Spec menu} init}}
	       in
		  Key#{List.toRecord menu 'self'#Group|
		       {MakeEntries Spec.menu KeyBinder Parent Group}}
	       [] radiobutton then
		  Key#{New Menues.radiobutton
		       {Adjoin Spec init(parent: Parent
					 group:  Group)}}
	       elseof Kind then
		  Menu = {New Menues.Kind {Adjoin Spec init(parent:Parent)}}
	       in
		  case {HasSubtreeAt Spec key} then
		     Key    = Spec.key
		     IsCtrl = {Value.matchDefault Spec ctrl False}
		  in
		     {KeyBinder
		      tkBind(event:  {MakeEvent IsCtrl Key}
			     action: proc {$}
					{Menu invoke}
				     end)}
		  else true
		  end
		  Key#Menu
	       end
	       |{MakeEntries Er KeyBinder Parent Group}
	    end
	 end
      end

      proc {MakeMenues Ms KeyBinder Parent ?KOs ?Bs}
	 case Ms of nil then KOs=nil Bs=nil
	 [] M|Mr then
	    MenuButton = {New Tk.menubutton tkInit(parent: Parent
						   text:   M.label)}
	    Menu       = {New Menues.menu init(parent:MenuButton)}
	    Full       = {List.toRecord menu 'self'#Menu|
			  {MakeEntries M.menu KeyBinder Menu False}}
	    KOr Br
	 in
	    {MenuButton tk(configure(menu:Menu))}
	    KOs = {Label M}#Full|KOr
	    Bs  = MenuButton|Br
	    {MakeMenues Mr KeyBinder Parent KOr Br}
	 end
      end

   in
      fun {MakeMenu P M}
	 MenuBar = {New Menues.bar init(parent:P)}
	 KOs Bs
      in
	 {MakeMenues M P MenuBar ?KOs ?Bs} 
	 {Tk.batch [pack(b(Bs) o(side:left))
		    tk_menuBar(MenuBar b(Bs))]}
	 {List.toRecord menu 'self'#MenuBar|KOs}
      end
   end
   
in

   Menues = menues(bar:         Menubar
		   group:       Group
		   menu:        Menu
		   command:     Command
		   checkbutton: Checkbutton
		   radiobutton: Radiobutton
		   separator:   Separator
		   cascade:     Cascade
		   make:        MakeMenu)
		   
end

%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
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

functor

prepare
   
   ArgSpec = record(defaults(rightmost type:bool default:true)
		    random(rightmost type:bool default:true))

   %% Default companies and drivers
   DefaultScenario = d('Disney': ['Mickey'('Düsseldorf') 'Goofy'('Berlin')]
		       'Oz':     ['Tinman'('München') 'Toto'('Saarbrücken')])
   
import
   Application Tk TkTools

   Configure(fonts colors goods)
   AgentAbstractions(new)
   Agents(broker)
   Widgets(entryChooser
	   map)
   Dialogs(about
	   addCompany remCompany
	   addDriver remDriver)
   Randomizer('class')
   
define

   TextBg       = Configure.colors.textBg
   TextFont     = Configure.fonts.text
   TextWidth    = 4
   BigTextWidth = 17
   
   class Frontend
      prop locking
	 
      feat
         %% Widget references
	 menu src dst what weight send map randomvar
         %% Agent components
	 agents broker randomizer
	 %%
	 toplevel

      attr
	 src:unit dst:unit

      meth init(toplevel:T)
	 self.toplevel = T
	 
	 ThisRandomizer = {New Randomizer.'class' init(broker:ThisBroker)}

	 RandGo    = {New Tk.variable tkInit(false)}
	 RandSpeed = {New Tk.variable tkInit(1)}

	 Menu = {TkTools.menubar T T
		 [menubutton(text:    'Transportation'
			     feature: transportation
			     menu:
				[command(label:   'About...'
					 action:  self # about)
				 separator
				 command(label:   'Quit'
					 action:  Application.exit # 0)])
		  menubutton(text:    'Configure'
			     feature: configure
			     menu:
				[command(label:   'Add Company...'
					 action:  self # addCompany
					 feature: addCompany)
				 command(label:   'Remove Company...'
					 action:  self # remCompany
					 state:   disabled
					 feature: remCompany)
				 separator
				 command(label:   'Add Driver...'
					 action:  self # addDriver
					 state:   disabled
					 feature: addDriver)
				 command(label:   'Remove Driver...'
					 action:  self # remDriver
					 state:   disabled
					 feature: remDriver)
				 separator
				 command(label:   'Add Defaults'
					 action:  self # addDefaults
					 feature: addDefaults)])
		  menubutton(text:    'Random'
			     feature: random
			     menu: 
				[radiobutton(label:    'Slow'
					     action:   ThisRandomizer # slow
					     variable: RandSpeed
					     value:    0)
				 radiobutton(label:    'Medium'
					     action:   ThisRandomizer # medium
					     variable: RandSpeed
					     value:    1)
				 radiobutton(label:    'Fast'
					     action:   ThisRandomizer # fast
					     variable: RandSpeed
					     value:    2)
				 separator
				 checkbutton(label:    'Go'
					     action:   ThisRandomizer # toggle
					     variable: RandGo)])
		  menubutton(text:    'Windows'
			     feature: windows
			     menu:    nil)]
		 nil}
	 
	 CtyMap = {New Widgets.map init(parent:self)}
	 Query  = {New Tk.frame   tkInit(parent:T relief:sunken bd:3)}
	 
	 FromL  = {New Tk.label tkInit(parent:Query text:'From:')}
	 FromT  = {New Tk.label tkInit(parent:Query width:BigTextWidth
				       anchor:w
				       bg:TextBg font:TextFont)}
	 ToL    = {New Tk.label tkInit(parent:Query text:'To:')}
	 ToT    = {New Tk.label tkInit(parent:Query width:BigTextWidth
				       anchor:w
				       bg:TextBg font:TextFont)}
	 WhatL  = {New Tk.label tkInit(parent:Query text:'Good:')}
	 WhatT  = {New Widgets.entryChooser
		   tkInit(parent:Query toplevel:T
			  entries: {Record.toList Configure.goods})}
	 WhgtL  = {New Tk.label tkInit(parent:Query text:'Weight:')}
	 WhgtT  = {New Tk.entry tkInit(parent:Query width:TextWidth
				       bg:TextBg font:TextFont)}
	 
	 Send   = {New Tk.button tkInit(parent:Query text:'Send to Broker'
					action: self # send
					state:  disabled)}
	 {WhgtT tk(insert 0 25)}

	 ThisBroker
      in
	 {ForAll [transportation configure random windows]
	  proc {$ F}
	     {Menu.F.menu tk(configure tearoff:false)}
	  end}
	 {Tk.batch [pack(FromL FromT ToL ToT
			  WhatL WhatT WhgtL WhgtT side:left padx:2)
		    pack(Send side:left padx:6)
		    pack(Menu CtyMap Query fill:x)]}

	 self.menu   = Menu
	 self.src    = FromT
	 self.dst    = ToT
	 self.send   = Send
	 self.map    = CtyMap
	 self.weight = WhgtT
	 self.what   = WhatT.entry

	 ThisBroker = {AgentAbstractions.new Agents.broker init(toplevel:self)}

	 self.agents     = {Dictionary.new}
	 self.broker     = ThisBroker
	 self.randomizer = ThisRandomizer
	 self.randomvar  = RandGo
      end

      meth random
	 {self.randomvar  tkSet(true)}
	 {self.randomizer toggle}
      end
      
      meth DisableMenus
	 {ForAll [transportation configure random]
	  proc {$ F}
	     {self.menu.F tk(configure state:disabled)}
	  end}
      end
      
      meth EnableMenus
	 {ForAll [transportation configure random]
	  proc {$ F}
	     {self.menu.F tk(configure state:normal)}
	  end}
      end

      meth CheckSend
	 S=@src D=@dst
      in
	 {self.send tk(conf state:if S\=D andthen
				     {IsAtom S} andthen {IsAtom D}
				  then normal
				  else disabled
				  end)}
      end
      
      meth putSrc(Src)
	 lock
	    src <- Src   {self.src tk(conf text:Src)}
	    Frontend, CheckSend
	 end
      end
      
      meth putDst(Dst)
	 lock
	    dst <- Dst   {self.dst tk(conf text:Dst)}
	    Frontend, CheckSend
	 end
      end
      
      meth send
	 lock
	    W = {self.weight tkReturnInt(get $)}
	 in
	    if {IsInt W} then
	       {self.broker announce(src:@src dst:@dst weight:W
				     what:{self.what tkReturnAtom(get $)})}
	    end
	 end
      end
	 
      meth about
	 lock
	    Frontend, DisableMenus
	    {Wait {New Dialogs.about init(master:self.toplevel)}.tkClosed}
	    Frontend, EnableMenus
	 end
      end
      
      meth addCompany
	 lock Agents=self.agents C in
	    Frontend, DisableMenus
	    {Wait {New Dialogs.addCompany init(master: self.toplevel
					       agents: Agents
					       company:C)}.tkClosed}
	    if {IsDet C} then
	       Menu = self.menu.configure
	    in
	       {Dictionary.put Agents C nil}
	       {self.broker add(company:C)}
	       {Menu.remCompany tk(entryconfigure state:normal)}
	       {Menu.addDriver  tk(entryconfigure state:normal)}
	       if {Some {Arity DefaultScenario}
		     fun {$ C}
			{Dictionary.member Agents C}
		     end} then
		  {Menu.addDefaults tk(entryconfigure state:disabled)}
	       end
	    end
	    Frontend, EnableMenus
	 end
      end
      
      meth remCompany
	 lock Agents=self.agents C in
	    Frontend, DisableMenus
	    {Wait {New Dialogs.remCompany init(master: self.toplevel
					       agents: Agents
					       company:C)}.tkClosed}
	    if {IsDet C} then
	       Menu = self.menu.configure
	    in
	       {Dictionary.remove Agents C}
	       {self.broker remove(company:C)}
	       if {Dictionary.entries Agents}==nil then
		  {Menu.remCompany  tk(entryconf      state:disabled)}
		  {Menu.addDriver   tk(entryconf      state:disabled)}
		  {Menu.remDriver   tk(entryconf      state:disabled)}
		  {Menu.addDefaults tk(entryconfigure state:normal)}
	       elseif {Not {Some {Arity DefaultScenario}
			      fun {$ C}
				 {Dictionary.member Agents C}
			      end}} then
		  {Menu.addDefaults  tk(entryconfigure state:enabled)}
	       end
	    end
	    Frontend, EnableMenus
	 end
      end
      
      meth addDriver
	 lock Agents=self.agents C D Y in
	    Frontend, DisableMenus
	    {Wait {New Dialogs.addDriver init(master: self.toplevel
					      agents: Agents
					      company:C
					      driver: D
					      city:   Y)}.tkClosed}
	    if {IsDet C} then
	       {Dictionary.put Agents C D|{Dictionary.get Agents C}}
	       {self.menu.configure.remDriver tk(entryconf state:normal)}
	       {self.broker add(company:C driver:D city:Y)}
	    end
	    Frontend, EnableMenus
	 end
      end
      
      meth remDriver
	 lock Agents=self.agents C D in
	    Frontend, DisableMenus
	    {Wait {New Dialogs.remDriver init(master:  self.toplevel
					      agents:  Agents
					      company: C
					      driver:  D)}.tkClosed}
	    if {IsDet C} then
	       {Dictionary.put Agents C
		{List.subtract {Dictionary.get Agents C} D}}
	       {self.broker remove(company:C driver:D)}
	       if {All {Dictionary.keys Agents}
		   fun {$ C}
		      {Dictionary.get Agents C}==nil
		   end}
	       then
		  {self.menu.configure.remDriver tk(entryconf state:disabled)}
	       end
	    end
	    Frontend, EnableMenus
	 end
      end
      
      meth addDefaults
	 lock Menu=self.menu.configure in
	    {Record.forAllInd DefaultScenario
	     proc {$ C Ds}
		{Dictionary.put self.agents C {Map Ds Label}}
		{self.broker add(company:C)}
		{ForAll Ds
		 proc {$ D}
		    {self.broker add(driver:{Label D} company:C city:D.1)}
		 end}
	     end}
	    {Menu.addDefaults tk(entryconf state:disabled)}
	    {Menu.addDriver   tk(entryconf state:normal)}
	    {Menu.remCompany  tk(entryconf state:normal)}
	    {Menu.remDriver   tk(entryconf state:normal)}
	 end
      end

   end
   
   Args = {Application.getCmdArgs ArgSpec}

   T = {New Tk.toplevel tkInit(title:  'Transportation'
			       delete: Application.exit # 0)}

   F = {New Frontend init(toplevel:T)}
      
   if Args.defaults orelse Args.random then
      {F addDefaults}
   end

   if Args.random then
      {F random}
   end

end


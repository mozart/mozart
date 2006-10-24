%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local

   class AboutDialog 
      from TkTools.dialog

      meth init(master:Master)
	 TkTools.dialog,tkInit(master:  Master
			       title:   TitleName#': About'
			       buttons: ['Okay'#tkClose]
			       focus:   1
			       pack:    false
			       default: 1)
	 Title = {New Tk.label tkInit(parent:     self
				      font:       AboutFont
				      text:       TitleName
				      foreground: blue)}

	 Author = {New Tk.label tkInit(parent: self
				       text: ('Christian Schulte\n' #
					      '(schulte@dfki.uni-sb.de)\n'))}
      in
	 {Tk.send pack(Title Author side:top expand:1 padx:BigPad pady:BigPad)}
	 AboutDialog,tkPack
      end

   end


   class OkayDialog from TkTools.dialog
      meth init(master:M title:T okay:O)
	 TkTools.dialog,tkInit(master:  M
			       title:   TitleName#': '#T
			       buttons: ['Okay'#O 'Cancel'#tkClose]
			       pack:    false
			       default: 1)
      end
   end

   proc {Error M T}
      {Wait {New TkTools.error tkInit(master:M text:T)}.tkClosed}
   end

   class AddCompanyDialog from OkayDialog prop final
      meth init(master:M agents:AS company:C)
	 proc {Okay}
	    AddC={Entry tkReturnAtom(get $)}
	 in
	    if {Dictionary.member AS AddC} then
	       {Error self 'Company '#AddC#' already exists.'}
	    else C=AddC {self tkClose}
	    end
	 end

	 OkayDialog,init(master:M title:'Add Company' okay:Okay)
	 Frame = {New TkTools.textframe tkInit(parent:self text:'Add Company')}
	 Name  = {New Tk.label tkInit(parent:Frame.inner text:'Company:')}
	 Entry = {New Tk.entry tkInit(parent:Frame.inner bg:TextBg
				      width:BigTextWidth)}
      in
	 {Tk.batch [pack(Name Entry side:left padx:Pad pady:Pad)
		    pack(Frame) focus(Entry)]}
	 AddCompanyDialog,tkPack
      end

   end


   class RemCompanyDialog from OkayDialog prop final

      meth init(master:M agents:AS company:C)
	 proc {Okay}
	    RemC={Entry.entry tkReturnAtom(get $)}
	 in
	    if {Dictionary.member AS RemC} then C=RemC {self tkClose}
	    else {Error self 'There is no company with name: '#RemC#'.'}
	    end
	 end

	 OkayDialog, init(master:M title:'Remove Company' okay:Okay)
	 Frame = {New TkTools.textframe tkInit(parent:self
					       text:'Remove Company')}
	 Name  = {New Tk.label tkInit(parent:Frame.inner text:'Company:')}
	 Entry = {New EntryChooser tkInit(parent:Frame.inner
					  toplevel:self.toplevel
					  entries:{Dictionary.keys AS})}
      in
	 {Tk.batch [pack(Name Entry side:left padx:Pad pady:Pad)
		    pack(Frame) focus(Entry.entry)]}
	 RemCompanyDialog,tkPack
      end

   end


   class AddDriverDialog from OkayDialog prop final

      meth init(master:M agents:AS company:C driver:D city:Y)
	 proc {Okay}
	    AddC={EntryC.entry tkReturnAtom(get $)}
	    AddD={EntryD       tkReturnAtom(get $)}
	    AddY={EntryY.entry tkReturnAtom(get $)}
	 in
	    if {Dictionary.member AS AddC} then
	       if {Member AddD {Dictionary.get AS AddC}} then
		  {Error self 'Driver '#AddD#' already exists for company '#
		              AddC#'.'}
	       elseif {Country.isCity AddY} then
		  C=AddC D=AddD Y=AddY  {self tkClose}
	       else
		  {Error self 'There is no city '#AddY#'.'}
	       end
	    else {Error self 'There is no company '#AddC#'.'}
	    end
	 end

	 OkayDialog, init(master:M title:'Add Driver' okay:Okay)
	 Frame  = {New TkTools.textframe tkInit(parent:self text:'Add Driver')}
	 NameC  = {New Tk.label tkInit(parent:Frame.inner text:'Company:')}
	 EntryC = {New EntryChooser tkInit(parent:   Frame.inner
					   toplevel: self.toplevel
					   entries:  {Dictionary.keys AS})}
	 NameD  = {New Tk.label tkInit(parent:Frame.inner text:'Driver:')}
	 EntryD = {New Tk.entry tkInit(parent:Frame.inner bg:TextBg)}
	 NameY  = {New Tk.label tkInit(parent:Frame.inner text:'City:')}
	 EntryY = {New EntryChooser tkInit(parent:   Frame.inner
					   toplevel: self.toplevel
					   entries:  Country.cities)}
      in
	 {Tk.batch [grid(NameC  row:0 column:0 sticky:w)
		    grid(NameD  row:1 column:0 sticky:w)
		    grid(NameY  row:2 column:0 sticky:w)
		    grid(EntryC row:0 column:1 sticky:we)
		    grid(EntryD row:1 column:1 sticky:we)
		    grid(EntryY row:2 column:1 sticky:we)
		    pack(Frame)]}
	 AddDriverDialog,tkPack
      end

   end


   class RemDriverDialog from OkayDialog prop final

      meth init(master:M agents:AS company:C driver:D)
	 Companies = {Filter {Dictionary.keys AS}
		      fun {$ C}
			 {Dictionary.get AS C}\=nil
		      end}
	 
	 proc {Okay}
	    RemC={EntryC.entry tkReturnAtom(get $)}
	    RemD={EntryD.entry tkReturnAtom(get $)}
	 in
	    if {Dictionary.member AS RemC} then
	       if {Member RemD {Dictionary.get AS RemC}} then
		  C=RemC D=RemD {self tkClose}
	       else
		  {Error self 'No driver '#RemD#' for company '#RemC#'.'}
	       end
	    else {Error self 'There is no company '#RemC#'.'}
	    end
	 end

	 OkayDialog, init(master:M title:'Remove Driver' okay:Okay)
	 Frame  = {New TkTools.textframe tkInit(parent:self text:'Remove Driver')}
	 NameC  = {New Tk.label tkInit(parent:Frame.inner text:'Company:')}
	 EntryC = {New EntryChooser tkInit(parent:Frame.inner
					   toplevel:self.toplevel
					   entries: Companies
					   action:  proc {$ A}
						       {EntryD
							entries({Dictionary.get
								 AS A})}
						    end)}
	 NameD  = {New Tk.label tkInit(parent:Frame.inner text:'Driver:')}
	 EntryD = {New EntryChooser tkInit(parent:Frame.inner
					   toplevel:self.toplevel
					   entries: {Dictionary.get AS
						     Companies.1})}
      in
	 {Tk.batch [grid(NameC  row:0 column:0 sticky:w)
		    grid(NameD  row:1 column:0 sticky:w)
		    grid(EntryC row:0 column:1 sticky:we)
		    grid(EntryD row:1 column:1 sticky:we)
		    pack(Frame)]}
	 RemDriverDialog,tkPack
      end

   end


   local
      TownSize   = 3
      TextOffset = 11
   in

      class CountryMap
	 from Tk.canvas
      
	 meth init(parent:P)
	    Tk.canvas,tkInit(parent: P.toplevel
			     relief:sunken bd:3
			     width:  Country.width
			     height: Country.height
			     bg:     BackColor)
	    {ForAll {Country.graph}
	     proc {$ SPDs}
		Src#(SrcX#SrcY)#Dsts = SPDs
		Tag                  = {New Tk.canvasTag tkInit(parent:self)}
	     in
		{Tag tkBind(event:'<1>' action:P # putSrc(Src))}
		{Tag tkBind(event:'<2>' action:P # putDst(Src))}
		{ForAll Dsts
		 proc {$ Dst}
		    DstX#DstY = Dst
		 in 
		    {self tk(crea line SrcX SrcY DstX DstY fill:StreetColor)}
		 end}
		{self tk(crea rectangle
			 SrcX-TownSize SrcY-TownSize
			 SrcX+TownSize SrcY+TownSize
			 fill:CityColor tag:Tag)}
		{self tk(crea text SrcX SrcY+TextOffset
			 text:Src font:TextFont tag:Tag)}
	     end}
	 end
	 
      end
      
   end
   
in
   
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
	 
	 ThisRandomizer = {New Randomizer init(broker:ThisBroker)}

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
	 
	 CtyMap = {New CountryMap init(parent:self)}
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
	 WhatT  = {New EntryChooser tkInit(parent:Query toplevel:T
					   entries: {Record.toList Goods})}
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

	 ThisBroker = {NewAgent Broker init(toplevel:self)}

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
	    {Wait {New AboutDialog init(master:self.toplevel)}.tkClosed}
	    Frontend, EnableMenus
	 end
      end
      
      meth addCompany
	 lock Agents=self.agents C in
	    Frontend, DisableMenus
	    {Wait {New AddCompanyDialog init(master: self.toplevel
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
	    {Wait {New RemCompanyDialog init(master: self.toplevel
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
	    {Wait {New AddDriverDialog init(master: self.toplevel
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
	    {Wait {New RemDriverDialog init(master:  self.toplevel
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
   
end








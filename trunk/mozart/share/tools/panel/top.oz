%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   class UpdatePage
      from Note
      feat options
      meth init(parent:P options:O text:T)
	 <<Note tkInit(parent:P text:T)>>
	 self.options = O
      end
   end
	 
   class ThreadPage
      from UpdatePage
      attr InfoVisible: False
      meth update(What)
	 O  = self.options
	 OT = O.threads
	 OP = O.priorities
	 OR = O.runtime
	 T = {System.get threads}
	 P = {System.get priorities}
	 R = {System.get time}
      in
	 case What==nosample then true else
	    {OT.load        display([{IntToFloat T.runnable}] 2)}
	 end
	 case What==sample then true else
	    {OR.pie         display(R)}
	    {OT.created     set(T.created)}
	    {OT.runnable    set(T.runnable)}
	    case @InfoVisible then
	       {OP.high        set(P.high)}
	       {OP.middle      set(P.middle)}
	    else true
	    end
	    {OR.run         set(R.run)}
	    {OR.gc          set(R.gc)}
	    {OR.copy        set(R.copy)}
	    {OR.propagation set(R.propagate)}
	    {OR.load        set(R.load)}
	 end
      end
      meth clear
	 O  = self.options
	 OT = O.threads
	 OR = O.runtime
      in
	 {OR.pie         clear}
	 {OT.created     clear}
	 {OR.run         clear}
	 {OR.gc          clear}
	 {OR.copy        clear}
	 {OR.propagation clear}
	 {OR.load        clear}
	 <<ThreadPage update(nosample)>>
      end
      meth toggleInfo
	 O = self.options
      in
	 {Tk.send case @InfoVisible then pack(forget O.priorities.frame)
		  else pack(O.priorities.frame
			    after:O.threads.frame side:top padx:Pad pady:Pad)
		  end}
	 InfoVisible <- {Not @InfoVisible}
      end
      meth toTop
	 <<ThreadPage update(nosample)>>
      end
   end
   
   class MemoryPage
      from UpdatePage
      attr InfoVisible: False
      meth update(What)
	 O  = self.options
	 OG = O.gc
	 OP = O.parameter
	 OU = O.usage
	 G = {System.get gc}
      in
	 case What==nosample then true else
	    {OU.load       display([{IntToFloat G.threshold} / MegaByteF
				    {IntToFloat G.size} / MegaByteF
				    {IntToFloat G.active} / MegaByteF]
				   2)}
	 end
	 case What==sample then true else
	    {OU.active     set(G.active div KiloByteI)}
	    {OU.size       set(G.size div KiloByteI)}
	    {OU.threshold  set(G.threshold div KiloByteI)}
	    case @InfoVisible then
	       {OP.minSize    set(G.min div MegaByteI)}
	       {OP.maxSize    set(G.max div MegaByteI)}
	       {OP.free       set(G.free)}
	       {OP.tolerance  set(G.tolerance)}
	       {OG.active     set(G.on)}
	    else true
	    end
	 end
      end
      meth toggleInfo
	 O = self.options
      in
	 {Tk.send case @InfoVisible then
		     pack(forget O.parameter.frame O.gc.frame)
		  else pack(O.parameter.frame O.gc.frame
			    after:O.usage.frame side:top pady:Pad padx:Pad)
		  end}
	 InfoVisible <- {Not @InfoVisible}
      end
      meth toTop
	 <<MemoryPage update(nosample)>>
      end
   end

   class PsPage
      from UpdatePage
      meth update
	 O  = self.options
	 OS = O.spaces
	 OF = O.fd
	 S = {System.get spaces}
	 F = {System.get fd}
      in
	 {OS.created   set(S.created)}
	 {OS.cloned    set(S.cloned)}
	 {OS.chosen    set(S.chosen)}
	 {OS.failed    set(S.failed)}
	 {OS.succeeded set(S.succeeded)}
	 {OF.propc     set(F.propagators)}
	 {OF.propi     set(F.invoked)}
	 {OF.var       set(F.variables)}
      end
      meth clear
	 O  = self.options
	 OS = O.spaces
	 OF = O.fd
      in
	 {OS.created   clear}
	 {OS.cloned    clear}
	 {OS.chosen    clear}
	 {OS.failed    clear}
	 {OS.succeeded clear}
	 {OF.propc     clear}
	 {OF.propi     clear}
	 {OF.var       clear}
	 <<PsPage update>>
      end
      meth toTop
	 <<PsPage update>>
      end

   end
   
   class OpiPage
      from UpdatePage

      meth update
	 O  = self.options
	 OE = O.errors
	 OP = O.output
	 OM = O.messages
	 E = {System.get errors}
	 P = {System.get print}
	 M = {System.get messages}
      in
	 {OE.'thread' set(E.'thread')}
	 {OE.location set(E.location)}
	 {OE.hints    set(E.hints)}
	 {OP.width    set(P.width)}
	 {OP.depth    set(P.depth)}
	 {OM.gc       set(M.gc)}
	 {OM.time     set(M.idle)}
      end
      meth toTop
	 <<OpiPage update>>
      end

   end
   
in
   
   class PanelTop
      from Tk.toplevel

      feat
	 manager
	 notebook
	 menu
	 threads memory opi ps
      attr
	 UpdateTime:   1000
	 RequireMouse: True
	 MouseInside:  True
	 DelayStamp:   0
	 InfoVisible:  False
      
      meth init(manager:Manager)
	 <<Tk.toplevel tkInit(title:              TitleName
			      highlightthickness: 0)>>
	 {Tk.batch [wm(iconname   self TitleName)
		    wm(iconbitmap self BitMap)
		    wm(resizable self 0 0)]}
	 EventFrame = {New Tk.frame tkInit(parent:             self
					   highlightthickness: 0)}
	 Menu  = {TkTools.menubar EventFrame self
		  [menubutton(text: ' Panel '
			      menu: [command(label:   'About ...'
					     action:  self # about
					     feature: about)
				     separator
				     command(label:   'Clear'
					     action:  self # clear)
				     separator
				     command(label:   'Shutdown System ...'
					     action:  self # shutdown
					     feature: shutdown)
				     separator
				     command(label:   'Close'
					     action:  self # close)]
			      feature: panel)
		   menubutton(text:    ' Options '
			      feature: options
			      menu: [checkbutton(label: 'Configurable'
						 variable: {New Tk.variable
							    tkInit(False)}
						 action: self # toggleInfo)
				     separator
				     command(label:  'Update ...'
					     action:  self # optionUpdate
					     feature: update)])
		  ]
		  nil}
	 Frame = {New Tk.frame tkInit(parent: EventFrame
				      highlightthickness: 0
				      bd:                 4)}
	 Book  = {New Notebook tkInit(parent: Frame
				      width:  PanelWidth
				      height: PartPanelHeight)}
	 Threads =
	 {MakePage ThreadPage 'Threads' Book True
	  [frame(text:    'Threads'
		 height:  70
		 left:    [number(text: 'Created:')
			   number(text: 'Runnable:'
				  color: RunnableColor)]
		 right:   [load(feature: load
				colors:  [RunnableColor])])
	   frame(text:    'Priorities'
		 height:  64
		 pack:    False
		 left:    [scale(text:    'High / Middle:'
				 state:   {System.get priorities}.high
				 feature: high
				 action:  proc {$ N}
					     {System.set
					      priorities(high:N)}
					  end)
			   scale(text: 'Middle / Low:'
				 state:   {System.get priorities}.middle
				 feature: middle
				 action:  proc {$ N}
					     {System.set
					      priorities(middle:N)}
					  end)]
		 right:   [button(text: 'Default'
				  action: proc {$}
					     {System.set
					      priorities(high:   10
							 middle: 10)}
					     {Threads update(nosample)}
					  end)])
	   frame(text:    'Runtime'
		 height:  100
		 left:    [time(text:    'Run:'
				color:   TimeColors.run)
			   time(text:    'Garbage collection:'
				color:   TimeColors.gc
				feature: gc)
			   time(text:    'Copy:'
				color:   TimeColors.copy)
			   time(text:    'Propagation:'
				color:   TimeColors.prop)
			   time(text:    'Load:'
				color:   TimeColors.load)]
		 right:   [pie(feature: pie)])]}
	 Memory =
	 {MakePage MemoryPage 'Memory' Book True
	  [frame(text:    'Heap Usage'
		 feature: usage
		 height:  70
		 left:    [size(text:    'Threshold:'
				color:   ThresholdColor)
			   size(text:    'Size:'
				color:   SizeColor)
			   size(text:    'Active size:'
				color:   ActiveColor
				feature: active)]
		 right:   [load(feature: load
				colors:  [ThresholdColor SizeColor ActiveColor]
				dim:     'MB')])
	   frame(text:    'Heap Parameters'
		 feature: parameter
		 pack:    False
		 height:  125
		 left:    [scale(text:    'Maximal size:'
				 range:   1#512
				 dim:     'MB'
				 feature: maxSize
				 state:   local MS={System.get gc}.max in
					     case MS=<0 then 512
					     else MS div MegaByteI
					     end
					  end
				 action:  proc {$ N}
					     S = Memory.options.
					            parameter.minSize
					  in
					     case {S get($)}>N then
						{S set(N)}
					     else true end
					     {System.set 
					      gc(max: N * MegaByteI)}
					  end)
			   scale(text:    'Minimal size:'
				 range:   1#512
				 dim:     'MB'
				 feature: minSize
				 state:   {System.get gc}.min div MegaByteI
				 action:  proc {$ N}
					     S = Memory.options.
					            parameter.maxSize
					  in
					     case {S get($)}<N then
						{S set(N)}
					     else true end
					     {System.set
					      gc(min: N * MegaByteI)}
					  end)
			   scale(text:    'Free:'
				 state:   {System.get gc}.free
				 action:  proc {$ N}
					     {System.set gc(free: N)}
					  end
				 dim:     '%')
			   scale(text:    'Tolerance:'
				 dim:     '%'
				 state:   {System.get gc}.tolerance
				 action:  proc {$ N}
					     {System.set
					      gc(tolerance: N)}
					  end)]
		 right:   [button(text:   'Small'
				  action: proc {$}
					     {System.set
					      gc(max:       4 * MegaByteI
						 min:       1 * MegaByteI
						 free:      60
						 tolerance: 20)}
					     {Memory update(nosample)}
					  end)
			   button(text:'Middle'
				  action: proc {$}
					     {System.set
					      gc(max:       16 * MegaByteI
						 min:       2  * MegaByteI
						 free:      70
						 tolerance: 15)}
					     {Memory update(nosample)}
					  end)
			   button(text:'Large'
				  action: proc {$}
					     {System.set
					      gc(max:       64 * MegaByteI
						 min:       8 * MegaByteI
						 free:      80
						 tolerance: 10)}
					     {Memory update(nosample)}
					  end)])
	   frame(text:    'Garbage Collector'
		 feature: gc
		 pack:    False
		 height:  30
		 left:    [checkbutton(text:   'Active'
				       state:  {System.get gc}.on
				       action: proc {$ OnOff}
						  {System.set gc(on:OnOff)}
					       end)]
		 right:   [button(text: 'Invoke'
				  action: proc {$}
					     {System.gcDo}
					  end)])]}
	 PS =
	 {MakePage PsPage 'Problem Solving' Book True
	  [frame(text:    'Finite Domain Constraints'
		 feature: fd
		 height:  60
		 left:    [number(text: 'Variables created:'
				  feature: var)
			   number(text:    'Propagators created:'
				  feature: propc)
			   number(text:    'Propagators invoked:'
				  feature: propi)]
		 right:   nil)
	   frame(text:    'Spaces'
		 height:  100
		 left:    [number(text:    'Spaces created:'
				  feature: created)
			   number(text:    'Spaces cloned:'
				  feature: cloned)
			   number(text:    'Spaces failed:'
				  feature: failed)
			   number(text:    'Spaces succeeded:'
				  feature: succeeded)
			   number(text:    'Alternatives chosen:'
				  feature: chosen)]
		 right:   nil)]}
	 OPI =
	 {MakePage OpiPage 'Programming Interface' Book False
	  [frame(text:    'Errors'
		 height:  60
		 left:    [checkbutton(text:    'Show thread'
				       feature: 'thread'
				       state:  {System.get errors}.'thread'
				       action:  proc {$ B}
						   {System.set
						    errors('thread':B)}
						end)
			   checkbutton(text:    'Show location'
				       feature: location
				       state:  {System.get errors}.location
				       action:  proc {$ B}
						   {System.set
						    errors(location:B)}
						end)
			   checkbutton(text: 'Show hints'
				       state:  {System.get errors}.hints
				       action:  proc {$ B}
						   {System.set
						    errors(hints:B)}
						end
				       feature: hints)]
		 right:   [button(text:   'Default'
				  action: proc {$}
					     {System.set
					      errors('thread': True
					             location: True
						     hints:    True)}
					     {OPI update}
					  end)])
	   frame(text:    'Output'
		 height:  50
		 left:    [entry(text:    'Maximal print width:'
				 feature: width
				 action:  proc {$ N}
					     {System.set print(width: N)}
					  end
				 top:     self)
			   entry(text:    'Maximal print depth:'
				 feature: depth
				 action:  proc {$ N}
					     {System.set print(depth: N)}
					  end
				 top:     self)]
		 right:   [button(text:  'Default'
				  action: proc {$}
					     {System.set print(width: 100
							       depth: 10)}
					     {OPI update}
					  end)])
	   frame(text:    'Status Messages'
		 feature: messages
		 height:  45
		 left:    [checkbutton(text:    'Idle'
				       feature: time
				       state:  {System.get messages}.idle
				       action: proc {$ B}
						  {System.set
						   messages(idle: B)}
					       end)
			   checkbutton(text:    'Garbage collection'
				       feature: gc
				       state:  {System.get messages}.gc
				       action: proc {$ B}
						  {System.set
						   messages(gc: B)}
					       end)]
		 right:   [button(text:  'Default'
				  action: proc {$}
					     {System.set
					      messages(idle: False
						       gc:   True)}
					     {OPI update}
					  end)])]}
	  
      in
	 {Tk.batch [pack(Menu side:top fill:x)
		    pack(Book)
		    pack(Frame side:bottom)
		    pack(EventFrame)]}
	 self.manager  = Manager
	 self.threads  = Threads
	 self.memory   = Memory
	 self.opi      = OPI
	 self.ps       = PS
	 self.notebook = Book
	 self.menu     = Menu
	 {EventFrame tkBind(event:'<Enter>'
			    action:self # enter)}
	 {EventFrame tkBind(event:'<Leave>'
			    action:self # leave)}
	 <<PanelTop update(@DelayStamp)>>
      end

      meth update(Stamp)
	 case Stamp==@DelayStamp then
	    TopNote = {self.notebook getTop($)}
	    Threads = self.threads
	    Memory  = self.memory
	 in
	    case TopNote
	    of !Threads then
	       {Threads update(both)}
	       {Memory  update(sample)}
	    [] !Memory  then
	       {Threads update(sample)}
	       {Memory  update(both)}
	    else
	       {Threads update(sample)}
	       {Memory  update(sample)}
	       {TopNote update}
	    end
	    <<PanelTop delay>>
	 else true
	 end
      end

      meth shutdown
	 {self.menu.panel.shutdown tk(entryconf state:disabled)}
	 thread
	    case {DoShutdown self} then {System.shutDown exit}
	    else true
	    end
	    {self.menu.panel.shutdown tk(entryconf state:normal)}
	 end
      end

      meth about
	 {self.menu.panel.about tk(entryconf state:disabled)}
	 thread
	    {Wait {DoAbout self}}
	    {self.menu.panel.about tk(entryconf state:normal)}
	 end
      end

      meth toggleInfo
	 InfoVisible <- {Not @InfoVisible}
	 case @InfoVisible then
	    {self.notebook confHeight(FullPanelHeight)}
	    {self.notebook add(note:self.opi)}
	 else
	    {self.notebook confHeight(PartPanelHeight)}
	    {self.notebook remove(note:self.opi)}
	 end
	 {self.threads toggleInfo}
	 {self.memory  toggleInfo}
      end
      
      meth delay
	 DS = @DelayStamp
      in
	 {Delay @UpdateTime}
	 {self update(DS)}
      end

      meth stop
	 DelayStamp <- @DelayStamp + 1
      end
      
      meth enter
	 MouseInside <- True
	 case @RequireMouse then <<PanelTop delay>>
	 else true
	 end
      end

      meth leave
	 MouseInside <- False
	 case @RequireMouse then <<PanelTop stop>>
	 else true
	 end
      end
      
      meth setUpdate(time:T mouse:M) 
	 case
	    {Or case @RequireMouse==M then False
		else
		   RequireMouse <- M
		   case M then
		      case @MouseInside then True
		      else <<PanelTop stop>> False
		      end
		   else <<PanelTop stop>> True
		   end
		end
	        case @UpdateTime==T then False
		else
		   UpdateTime <- T
		   <<PanelTop stop>>
		   True
		end}
	 then <<PanelTop delay>>
	 else true
	 end
      end
      
      meth optionUpdate
	 T = @UpdateTime
	 M = @RequireMouse
      in
	 {self.menu.options.update tk(entryconf state:disabled)}
	 thread
	    Next = {DoOptionUpdate self setUpdate(time:T mouse:M)}
	 in
	    {Wait Next}
	    {self.menu.options.update tk(entryconf state:normal)}
	    {self Next}
	 end
      end

      meth clear
	 {self.ps      clear}
	 {self.threads clear}
      end
      
      meth close
	 <<UrObject    close>>
	 <<Tk.toplevel close>>
	 {self.manager PanelTopClosed}
      end
      
   end


end

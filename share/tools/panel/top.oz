%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   class UpdatePage from TkTools.note
      feat
	 options
      meth init(parent:P options:O text:T)
	 TkTools.note,tkInit(parent:P text:T)
	 self.options = O
      end
      meth toTop
	 self,update(nosample)
      end
   end
	 
   class ThreadPage from UpdatePage
      attr
	 InfoVisible:  False

      meth update(What)
	 O   = self.options
	 OT  = O.threads
	 OP  = O.priorities
	 OR  = O.runtime
	 T   = {System.get threads}
	 P   = {System.get priorities}
	 R   = {System.get time}
      in
	 case What==nosample then skip else
	    {OT.load        display([{IntToFloat T.runnable}])}
	 end
	 case What==sample then skip else
	    {OR.timebar     display(R)}
	    {OT.created     set(T.created)}
	    {OT.runnable    set(T.runnable)}
	    case @InfoVisible then
	       {OP.high        set(P.high)}
	       {OP.medium      set(P.medium)}
	    else skip
	    end
	    {OR.run         set(R.run)}
	    {OR.gc          set(R.gc)}
	    {OR.copy        set(R.copy)}
	    {OR.propagation set(R.propagate)}
	    {OR.load        set(R.load)}
	 end
	 touch
      end
      meth clear
	 O  = self.options
	 OT = O.threads
	 OR = O.runtime
      in
	 {OR.timebar     clear}
	 {OT.created     clear}
	 {OT.load        clear}
	 {OR.run         clear}
	 {OR.gc          clear}
	 {OR.copy        clear}
	 {OR.propagation clear}
	 {OR.load        clear}
	 touch
      end
      meth toggleInfo
	 O = self.options
      in
	 {Tk.send case @InfoVisible then pack(forget O.priorities.frame)
		  else pack(O.priorities.frame
			    after:O.threads.frame side:top fill:x padx:3)
		  end}
	 InfoVisible <- {Not @InfoVisible}
      end
   end
   
   class MemoryPage
      from UpdatePage
      attr
	 InfoVisible:  False
      
      meth update(What)
	 O   = self.options
	 OG  = O.gc
	 OU  = O.usage
	 G   = {System.get gc}
      in
	 case What==nosample then skip else
	    {OU.load       display([{IntToFloat G.threshold} / MegaByteF
				    {IntToFloat G.size} / MegaByteF
				    {IntToFloat G.active} / MegaByteF])}
	 end
	 case What==sample then skip else
	    {OU.active     set(G.active div KiloByteI)}
	    {OU.size       set(G.size div KiloByteI)}
	    {OU.threshold  set(G.threshold div KiloByteI)}
	    case @InfoVisible then
	       OP  = O.parameter
	       OU  = O.usage
	    in
	       {OP.minSize    set(G.min div MegaByteI)}
	       {OP.maxSize    set(G.max div MegaByteI)}
	       {OP.free       set(G.free)}
	       {OP.tolerance  set(G.tolerance)}
	       {OG.active     set(G.on)}
	    else
	       OP  = O.showParameter
	    in
	       {OP.minSize    set(G.min div MegaByteI)}
	       {OP.maxSize    set(G.max div MegaByteI)}
	    end
	 end
	 touch
      end
      meth clear
	 {self.options.usage.load clear}
	 touch
      end
      meth toggleInfo
	 O = self.options
      in
	 {Tk.batch case @InfoVisible then
		      [pack(forget O.parameter.frame O.gc.frame)
		       pack(O.showParameter.frame 
			    after:O.usage.frame side:top fill:x)]
		   else
		      [pack(forget O.showParameter)
		       pack(O.parameter.frame O.gc.frame
			    after:O.usage.frame side:top fill:x)]
		   end}
	 InfoVisible <- {Not @InfoVisible}
	 MemoryPage,update(nosample)
      end
   end

   class PsPage
      from UpdatePage

      meth update(...)
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
	 touch
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
	 touch
      end

   end
   
   class OpiPage
      from UpdatePage

      meth update(...)
	 O  = self.options
	 OE = O.errors
	 OP = O.output
	 OM = O.messages
	 E = {System.get errors}
	 P = {System.get print}
	 M = {System.get messages}
      in
	 {OE.'thread' set(E.'thread')}
	 {OE.width    set(E.width)}
	 {OE.depth    set(E.depth)}
	 {OE.location set(E.location)}
	 {OE.hints    set(E.hints)}
	 {OP.width    set(P.width)}
	 {OP.depth    set(P.depth)}
	 {OM.gc       set(M.gc)}
	 {OM.time     set(M.idle)}
	 touch
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
	 UpdateTime:    DefaultUpdateTime
	 HistoryRange:  DefaultHistoryRange
	 RequireMouse:  True
	 MouseInside:   True
	 DelayStamp:    0
	 InfoVisible:   False
      
      meth init(manager:Manager)
	 Tk.toplevel,tkInit(title:              TitleName
			    highlightthickness: 0
			    withdraw:           True)
	 {Tk.batch [wm(iconname   self TitleName)
		    wm(iconbitmap self BitMap)
		    wm(resizable self 0 0)]}
	 EventFrame = {New Tk.frame tkInit(parent:             self
					   highlightthickness: 0)}
	 Menu  = {TkTools.menubar EventFrame self
		  [menubutton(text: ' Panel '
			      menu: [command(label:   'About...'
					     action:  self # about
					     feature: about)
				     separator
				     command(label:   'Clear'
					     action:  self # clear)
				     separator
				     command(label:   'Shutdown System...'
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
				     command(label:  'Update...'
					     action:  self # optionUpdate
					     feature: update)
				     command(label:  'History...'
					     action:  self # optionHistory
					     feature: history)])
		  ]
		  nil}
	 {Menu.panel.menu   tk(conf tearoff:False)}
	 {Menu.options.menu tk(conf tearoff:False)}
	 Frame = {New Tk.frame tkInit(parent: EventFrame
				      highlightthickness: 0
				      bd:                 4)}
	 Book  = {New TkTools.notebook tkInit(parent: Frame)}
	 Threads =
	 {MakePage ThreadPage 'Threads' Book True
	  [frame(text:    'Threads'
		 left:    [number(text:    'Created:')
			   number(text:    'Runnable:'
				  color:   RunnableColor
				  stipple: RunnableStipple)]
		 right:   [load(feature: load
				colors:  [RunnableColor]
				stipple: [RunnableStipple])])
	   frame(text:    'Priorities'
		 pack:    False
		 left:    [scale(text:    'High / Medium:'
				 state:   {System.get priorities}.high
				 feature: high
				 action:  proc {$ N}
					     {System.set
					      priorities(high:N)}
					  end)
			   scale(text: 'Medium / Low:'
				 state:   {System.get priorities}.medium
				 feature: medium
				 action:  proc {$ N}
					     {System.set
					      priorities(medium:N)}
					  end)]
		 right:   [button(text: 'Default'
				  action: proc {$}
					     {System.set
					      priorities(high:   10
							 medium: 10)}
					     {Threads update(nosample)}
					  end)])
	   frame(text:    'Runtime'
		 left:    [time(text:    'Run:'
				color:   TimeColors.run
				stipple: TimeStipple.run)
			   time(text:    'Garbage collection:'
				color:   TimeColors.gc
				feature: gc
				stipple: TimeStipple.gc)
			   time(text:    'Copy:'
				color:   TimeColors.copy
				stipple: TimeStipple.copy)
			   time(text:    'Propagation:'
				color:   TimeColors.prop
				stipple: TimeStipple.prop)
			   time(text:    'Load:'
				color:   TimeColors.load
				stipple: TimeStipple.load)]
		 right:   [timebar(feature: timebar)])]}
	 Memory =
	 {MakePage MemoryPage 'Memory' Book True
	  [frame(text:    'Heap Usage'
		 feature: usage
		 left:    [size(text:    'Threshold:'
				color:   ThresholdColor
				stipple: ThresholdStipple)
			   size(text:    'Size:'
				color:   SizeColor
				stipple: SizeStipple)
			   size(text:    'Active size:'
				color:   ActiveColor
				feature: active
				stipple: ActiveStipple)]
		 right:   [load(feature: load
				colors:  [ThresholdColor SizeColor
					  ActiveColor]
				stipple: [ThresholdStipple SizeStipple
					  ActiveStipple]
				dim:     'MB')])
	   frame(text:    'Heap Parameters'
		 feature: parameter
		 pack:    False
		 left:    [scale(text:    'Maximal size limit:'
				 range:   1#1024
				 dim:     'MB'
				 feature: maxSize
				 state:   local MS={System.get gc}.max in
					     case MS=<0 then 1024
					     else MS div MegaByteI
					     end
					  end
				 action:  proc {$ N}
					     S = Memory.options.
					            parameter.minSize
					  in
					     case {S get($)}>N then
						{S set(N)}
					     else skip end
					     {System.set 
					      gc(max: N * MegaByteI)}
					  end)
			   scale(text:    'Minimal size limit:'
				 range:   1#1024
				 dim:     'MB'
				 feature: minSize
				 state:   {System.get gc}.min div MegaByteI
				 action:  proc {$ N}
					     S = Memory.options.
					            parameter.maxSize
					  in
					     case {S get($)}<N then
						{S set(N)}
					     else skip end
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
						 free:      75
						 tolerance: 20)}
					     {Memory update(nosample)}
					  end)
			   button(text:'Medium'
				  action: proc {$}
					     {System.set
					      gc(max:       16 * MegaByteI
						 min:       2  * MegaByteI
						 free:      80
						 tolerance: 15)}
					     {Memory update(nosample)}
					  end)
			   button(text:'Large'
				  action: proc {$}
					     {System.set
					      gc(max:       64 * MegaByteI
						 min:       8 * MegaByteI
						 free:      90
						 tolerance: 10)}
					     {Memory update(nosample)}
					  end)])
	   frame(text:    'Heap Parameters'
		 feature: showParameter
		 left:    [size(text:    'Maximal size limit:'
				feature: maxSize
				dim:     'MB')
			   size(text:    'Minimal size limit:'
				feature: minSize
				dim:     'MB')]
		 right:   nil)
	   frame(text:    'Garbage Collector'
		 feature: gc
		 pack:    False
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
		 left:    [number(text: 'Variables created:'
				  feature: var)
			   number(text:    'Propagators created:'
				  feature: propc)
			   number(text:    'Propagators invoked:'
				  feature: propi)]
		 right:   nil)
	   frame(text:    'Spaces'
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
		 left:    [checkbutton(text:    'Show location'
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
				       feature: hints)
			   entry(text:    'Maximal tasks:'
				 feature: 'thread'
				 action:  proc {$ N}
					     {System.set errors('thread': N)}
					  end
				 top:     self)
			   entry(text:    'Maximal print depth:'
				 feature: depth
				 action:  proc {$ N}
					     {System.set errors(depth: N)}
					  end
				 top:     self)
			   entry(text:    'Maximal print width:'
				 feature: width
				 action:  proc {$ N}
					     {System.set errors(width: N)}
					  end
				 top:     self)]
		 right:   [button(text:   'Default'
				  action: proc {$}
					     {System.set
					      errors('thread': 10
					             location: True
						     hints:    True
					             width:    10
						     depth:    2)}
					     {OPI update}
					  end)])
	   frame(text:    'Output'
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
					     {System.set print(width: 10
							       depth: 2)}
					     {OPI update}
					  end)])
	   frame(text:    'Status Messages'
		 feature: messages
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
	 PanelTop,tkWM(deiconify)
	         ,update(@DelayStamp)
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
	    PanelTop,delay
	 else skip
	 end
      end

      meth shutdown
	 {self.menu.panel.shutdown tk(entryconf state:disabled)}
	 thread
	    case {DoShutdown self} then {System.shutDown exit}
	    else skip
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
	 case @InfoVisible then {self.notebook add(self.opi)}
	 else {self.notebook remove(self.opi)}
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
	 case @RequireMouse then
	    PanelTop,stop
	            ,delay
	 else skip
	 end
      end

      meth leave
	 MouseInside <- False
	 case @RequireMouse then
	    PanelTop,stop
	 else skip
	 end
      end
      
      meth setUpdate(time:T mouse:M)
	 case
	    {Or case @RequireMouse==M then False
		else
		   RequireMouse <- M
		   case M then
		      case @MouseInside then True
		      else PanelTop,stop False
		      end
		   else PanelTop,stop True
		   end
		end
	        case @UpdateTime==T then False
		else
		   UpdateTime <- T
		   PanelTop,stop
		   PanelTop,setSlice
		   True
		end}
	 then PanelTop,delay
	 else skip
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

      meth setSlice
	 S = (LoadWidth * @UpdateTime) div @HistoryRange
      in
	 {self.memory.options.usage.load    slice(S)}
	 {self.threads.options.threads.load slice(S)}
      end

      meth setHistory(H)
	 case H==@HistoryRange then skip else
	    HistoryRange <- H
	    PanelTop,setSlice
	 end
      end

      meth optionHistory
	 H = @HistoryRange
      in
	 {self.menu.options.history tk(entryconf state:disabled)}
	 thread
	    Next = {DoOptionHistory self H}
	 in
	    {Wait Next}
	    {self.menu.options.history tk(entryconf state:normal)}
	    {self setHistory(Next)}
	 end
      end

      meth clear
	 {self.threads clear}
	 {self.memory  clear}
	 {self.ps      clear}
	 touch
      end
      
      meth close
	 UrObject,close
	 Tk.toplevel,close
	 {self.manager PanelTopClosed}
      end
      
   end


end

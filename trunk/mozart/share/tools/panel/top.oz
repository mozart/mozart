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
	 top
      meth init(parent:P top:Top options:O text:T)
	 TkTools.note,tkInit(parent:P text:T)
	 self.options = O
	 self.top     = Top
      end
      meth toTop
	 {self.top update(false)}
      end
   end
	 
   class ThreadPage from UpdatePage
      prop final
      attr InfoVisible: false

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
      prop final
      attr InfoVisible:  false
      
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
      end
      meth clear
	 {self.options.usage.load clear}
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
      prop final

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
      end

   end
   
   class OpiPage
      from UpdatePage
      prop final

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
      end

   end
   
in
   
   class PanelTop
      from Tk.toplevel
      prop
	 locking
	 final
      feat
	 manager
	 notebook
	 menu
	 threads memory opi ps
      attr
	 UpdateTime:    DefaultUpdateTime
	 HistoryRange:  DefaultHistoryRange
	 RequireMouse:  true
	 MouseInside:   true
	 DelayStamp:    0
	 InfoVisible:   false
	 IsClosed:      false
      
      meth init(manager:Manager)
	 lock
	    Tk.toplevel,tkInit(title:              TitleName
			       highlightthickness: 0
			       withdraw:           true)
	    {Tk.batch [wm(iconname   self TitleName)
		       wm(iconbitmap self BitMap)
		       wm(resizable self 0 0)]}
	    EventFrame = {New Tk.frame tkInit(parent:             self
					      highlightthickness: 0)}
	    Menu  = {TkTools.menubar EventFrame self
		     [menubutton(text:    ' Panel '
				 feature: panel
				 menu:
		        [command(label:   'About...'
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
				 action:  self # close)])
		      menubutton(text:    ' Options '
				 feature: options
				 menu:
		         [checkbutton(label: 'Configurable'
				      variable: {New Tk.variable tkInit(false)}
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
	    {Menu.panel.menu   tk(conf tearoff:false)}
	    {Menu.options.menu tk(conf tearoff:false)}
	    Frame = {New Tk.frame tkInit(parent: EventFrame
					 highlightthickness: 0
					 bd:                 4)}
	    Book  = {New TkTools.notebook tkInit(parent: Frame)}
	    Threads =
	    {MakePage ThreadPage 'Threads' Book self true
	     [frame(text:    'Threads'
		    left:
		       [number(text:    'Created:')
			number(text:    'Runnable:'
			       color:   RunnableColor
			       stipple: RunnableStipple)]
		    right:
		       [load(feature: load
			     colors:  [RunnableColor]
			     stipple: [RunnableStipple])])
	      frame(text:    'Priorities'
		    pack:    false
		    left:
		       [scale(text:    'High / Medium:'
			      state:   {System.get priorities}.high
			      feature: high
			      action:  proc {$ N}
					  {System.set priorities(high:N)}
				       end)
			scale(text: 'Medium / Low:'
			      state:   {System.get priorities}.medium
			      feature: medium
			      action:  proc {$ N}
					  {System.set priorities(medium:N)}
				       end)]
		    right:
		       [button(text: 'Default'
			       action: proc {$}
					  {System.set priorities(high:   10
								 medium: 10)}
					  {self update(false)}
				       end)])
	      frame(text:    'Runtime'
		    left:
		       [time(text:    'Run:'
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
			     color:   TimeColors.'prop'
			     stipple: TimeStipple.'prop')
			time(text:    'Load:'
			     color:   TimeColors.load
			     stipple: TimeStipple.load)]
		    right:
		       [timebar(feature: timebar)])]}
	    Memory =
	    {MakePage MemoryPage 'Memory' Book self true
	     [frame(text:    'Heap Usage'
		    feature: usage
		    left:
		       [size(text:    'Threshold:'
			     color:   ThresholdColor
			     stipple: ThresholdStipple)
			size(text:    'Size:'
			     color:   SizeColor
			     stipple: SizeStipple)
			size(text:    'Active size:'
			     color:   ActiveColor
			     feature: active
			     stipple: ActiveStipple)]
		    right:
		       [load(feature: load
			     colors:  [ThresholdColor   SizeColor
				       ActiveColor]
			     stipple: [ThresholdStipple SizeStipple
				       ActiveStipple]
			     dim:     'MB')])
	      frame(text:    'Heap Parameters'
		    feature: parameter
		    pack:    false
		    left:
		       [scale(text:    'Maximal size limit:'
			      range:   1#1024
			      dim:     'MB'
			      feature: maxSize
			      state:   local MS={System.get gc}.max in
					  case MS=<0 then 1024
					  else MS div MegaByteI
					  end
				       end
			      action:  proc {$ N}
					  S = Memory.options.parameter.minSize
				       in
					  case {S get($)}>N then {S set(N)}
					  else skip end
					  {System.set gc(max: N * MegaByteI)}
				       end)
			scale(text:    'Minimal size limit:'
			      range:   1#1024
			      dim:     'MB'
			      feature: minSize
			      state:   {System.get gc}.min div MegaByteI
			      action:  proc {$ N}
					  S = Memory.options.parameter.maxSize
				       in
					  case {S get($)}<N then {S set(N)}
					  else skip end
					  {System.set gc(min: N * MegaByteI)}
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
					  {System.set gc(tolerance: N)}
				       end)]
	            right:
		       [button(text:   'Small'
			       action: proc {$}
					  {System.set
					   gc(max:       4 * MegaByteI
					      min:       1 * MegaByteI
					      free:      75
					      tolerance: 20)}
					  {self update(false)}
				       end)
			button(text:'Medium'
			       action: proc {$}
					  {System.set
					   gc(max:       16 * MegaByteI
					      min:       2  * MegaByteI
					      free:      80
					      tolerance: 15)}
					  {self update(false)}
				       end)
			button(text:'Large'
			       action: proc {$}
					  {System.set
					   gc(max:       64 * MegaByteI
					      min:       8 * MegaByteI
					      free:      90
					      tolerance: 10)}
					  {self update(false)}
				       end)])
	      frame(text:    'Heap Parameters'
		    feature: showParameter
		    left:
		       [size(text:    'Maximal size limit:'
			     feature: maxSize
			     dim:     'MB')
			size(text:    'Minimal size limit:'
			     feature: minSize
			     dim:     'MB')]
		    right:
		       nil)
	      frame(text:    'Garbage Collector'
		    feature: gc
		    pack:    false
		    left:
		       [checkbutton(text:   'Active'
				    state:  {System.get gc}.on
				    action: proc {$ OnOff}
					       {System.set gc(on:OnOff)}
					    end)]
		    right:   [button(text: 'Invoke'
				     action: proc {$}
						{System.gcDo}
					     end)])]}
	     PS =
	     {MakePage PsPage 'Problem Solving' Book self true
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
	     {MakePage OpiPage 'Programming Interface' Book self false
	      [frame(text:    'Errors'
		     left:
			[checkbutton(text:    'Show location'
				     feature: location
				     state:  {System.get errors}.location
				     action:  proc {$ B}
						 {System.set
						  errors(location:B)}
					      end)
			 checkbutton(text: 'Show hints'
				     state:  {System.get errors}.hints
				     action:  proc {$ B}
						 {System.set errors(hints:B)}
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
		     right:
			[button(text:   'Default'
				action: proc {$}
					   {System.set
					    errors('thread': 10
						   location: true
						   hints:    true
						   width:    10
						   depth:    2)}
					   {self update(false)}
					end)])
	       frame(text:    'Output'
		     left:
			[entry(text:    'Maximal print width:'
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
		     right:
			[button(text:  'Default'
				action: proc {$}
					   {System.set print(width: 10
							     depth: 2)}
					   {self update(false)}
					end)])
	       frame(text:    'Status Messages'
		     feature: messages
		     left:
			[checkbutton(text:    'Idle'
				     feature: time
				     state:  {System.get messages}.idle
				     action: proc {$ B}
						{System.set messages(idle:B)}
					     end)
			 checkbutton(text:    'Garbage collection'
				     feature: gc
				     state:  {System.get messages}.gc
				     action: proc {$ B}
						{System.set messages(gc:B)}
					     end)]
		     right:
			[button(text:  'Default'
				action: proc {$}
					   {System.set messages(idle: false
								gc:   true)}
					   {self update(false)}
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
	    {EventFrame tkBind(event:'<Enter>' action:self # enter)}
	    {EventFrame tkBind(event:'<Leave>' action:self # leave)}
	    PanelTop, tkWM(deiconify)
	 end
	 PanelTop, delay(0)
      end

      meth update(Regular)
	 lock
	    case @IsClosed then skip else
	       TopNote = {self.notebook getTop($)}
	       Threads = self.threads
	       Memory  = self.memory
	    in
	       case Regular then
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
	       else {TopNote update(nosample)}
	       end
	    end
	 end
      end

      meth shutdown
	 {self.menu.panel.shutdown tk(entryconf state:disabled)}
	 case {DoShutdown self} then {System.shutDown exit}
	 else skip
	 end
	 {self.menu.panel.shutdown tk(entryconf state:normal)}
      end

      meth about
	 {self.menu.panel.about tk(entryconf state:disabled)}
	 {Wait {DoAbout self}}
	 {self.menu.panel.about tk(entryconf state:normal)}
      end

      meth toggleInfo
	 lock
	    InfoVisible <- {Not @InfoVisible}
	    case @InfoVisible then {self.notebook add(self.opi)}
	    else {self.notebook remove(self.opi)}
	    end
	    {self.threads toggleInfo}
	    {self.memory  toggleInfo}
	 end
      end
      
      meth delay(ODS)
	 case
	    lock
	       case @DelayStamp==ODS then {self update(true)} true
	       else false
	       end
	    end
	 then {Delay @UpdateTime} {self delay(ODS)}
	 else skip
	 end
      end

      meth stop
	 lock
	    DelayStamp <- @DelayStamp + 1
	 end
      end
      
      meth enter
	 case
	    lock
	       case @IsClosed then ~1 else
		  MouseInside <- true
		  case @RequireMouse then
		     PanelTop,stop
		     @DelayStamp
		  else ~1
		  end
	       end
	    end
	 of ~1 then skip
	 elseof DS then {self delay(DS)}
	 end
      end

      meth leave
	 lock
	    case @IsClosed then skip else
	       MouseInside <- false
	       case @RequireMouse then
		  PanelTop,stop
	       else skip
	       end
	    end
	 end
      end
      
      meth setUpdate(time:T mouse:M)
	 case
	    lock
	       {Max case @RequireMouse==M then ~1
		    else
		       RequireMouse <- M
		       case M then
			  case @MouseInside then @DelayStamp
			  else PanelTop,stop ~1
			  end
		       else PanelTop,stop @DelayStamp
		       end
		    end
	            case @UpdateTime==T then ~1
		    else
		       UpdateTime <- T
		       PanelTop,stop
		       PanelTop,setSlice
		       @DelayStamp
		    end}
	    end
	 of ~1 then skip
	 elseof DS then {self delay(DS)}
	 end
      end

      meth optionUpdate
	 T    = @UpdateTime
	 M    = @RequireMouse
	 Next
      in
	 {self.menu.options.update tk(entryconf state:disabled)}
	 Next = {DoOptionUpdate self setUpdate(time:T mouse:M)}
	 {Wait Next}
	 {self.menu.options.update tk(entryconf state:normal)}
	 {self Next}
      end

      meth setSlice
	 lock
	    S = (LoadWidth * @UpdateTime) div @HistoryRange
	 in
	    {self.memory.options.usage.load    slice(S)}
	    {self.threads.options.threads.load slice(S)}
	 end
      end

      meth setHistory(H)
	 lock
	    case H==@HistoryRange then skip else
	       HistoryRange <- H
	       PanelTop,setSlice
	    end
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
	 lock
	    {self.threads clear}
	    {self.memory  clear}
	    {self.ps      clear}
	 end
      end
      
      meth close
	 {self.manager PanelTopClosed}
	 IsClosed <- true
	 Tk.toplevel, tkClose
      end
      
   end


end

%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   class AboutDialog 
      from TkTools.dialog

      meth init(master:Master done:Done)
	 <<TkTools.dialog tkInit(master:  Master
				 title:   TitleName#': About'
				 buttons: ['Okay'#close(proc {$}
							   Done = Unit
							end)]
				 focus:   1
				 default: 1)>>
	 Title = {New Tk.label tkInit(parent:     self
				      font:       AboutFont
				      text:       TitleName
				      foreground: AboutColor)}

	 Author = {New Tk.label tkInit(parent: self
				       text: ('Christian Schulte\n' #
					      '(schulte@dfki.uni-sb.de)\n'))}
      in
	 {Tk.send pack(Title Author
		       side:top expand:1 padx:BigPad pady:BigPad)}
      end

   end

   class ShutdownDialog 
      from TkTools.dialog

      meth init(master:Master done:Done)
	 <<TkTools.dialog tkInit(master:  Master
				 title:   TitleName#': Shutdown'
				 buttons: ['Okay'#close(proc {$}
							   Done = True
							end)
					   'Cancel'#close(proc {$}
							   Done = False
							end)]
				 focus:   1
				 default: 1)>>
	 Bitmap  = {New Tk.label   tkInit(parent: self
					  bitmap: question)}
	 Message = {New Tk.message tkInit(parent: self
					  text:   'Do you really want to shutdown')}   
      in
	 {Tk.send pack(Bitmap Message
		       side:left expand:1 padx:BigPad pady:BigPad)}
      end

   end

   class UpdatePage
      from Note
      feat options
      meth init(parent:P options:O)
	 <<Note tkInit(parent:P)>>
	 self.options = O
      end
   end
	 
   class ThreadPage
      from UpdatePage

      meth update(What Slice)
	 O  = self.options
	 OT = O.threads
	 OP = O.priorities
	 OR = O.runtime
	 T = {System.get threads}
	 P = {System.get priorities}
	 R = {System.get time}
      in
	 case What==nosample then true else
	    {OT.load        display([{IntToFloat T.runnable}] Slice)}
	 end
	 case What==sample then true else
	    {OR.pie         display(R)}
	    {OT.created     set(T.created)}
	    {OT.runnable    set(T.runnable)}
	    {OP.high        set(P.high)}
	    {OP.middle      set(P.middle)}
	    {OR.total       set(R.user)}
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
	 {OR.total       clear}
	 {OR.gc          clear}
	 {OR.copy        clear}
	 {OR.propagation clear}
	 {OR.load        clear}
	 <<ThreadPage update(nosample 0)>>
      end
   end
   
   class MemoryPage
      from UpdatePage

      meth update(What Slice)
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
				   Slice)}
	 end
	 case What==sample then true else
	    {OU.active     set(G.active div KiloByteI)}
	    {OU.size       set(G.size div KiloByteI)}
	    {OU.threshold  set(G.threshold div KiloByteI)}
	    {OP.minSize    set(G.min div MegaByteI)}
	    {OP.maxSize    set(G.max div MegaByteI)}
	    {OP.free       set(G.free)}
	    {OP.tolerance  set(G.tolerance)}
	    {OG.active     set(G.on)}
	 end
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
	 <<PsPage update>>
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
   end
   
in
   
   class PanelTop
      from Tk.toplevel Time.repeat

      feat
	 manager
	 notebook
	 menu
	 threads memory opi ps
      attr
	 Slice: 2
      
      meth init(manager:Manager)
	 <<Tk.toplevel tkInit(title:              TitleName
			      highlightthickness: 0)>>
	 {Tk.batch [wm(iconname   self TitleName)
		    wm(iconbitmap self BitMap)
		    wm(resizable self 0 0)]}

	 VarSample = {New Tk.variable tkInit(1)}
	 Menu  = {TkTools.menubar self self
		  [menubutton(text: ' Oz Panel '
			      menu: [command(label:   'About...'
					     action:  self # about
					     feature: about)
				     separator
				     command(label:   'Clear'
					     action:  self # clear)
				     separator
				     command(label:   'Shutdown System'
					     action:  self # shutdown
					     feature: shutdown)
				     separator
				     command(label:   'Close'
					     action:  self # close)]
			      feature: panel)
		   menubutton(text: ' Sample '
			      menu: [radiobutton(label:    'Every half second'
						 variable: VarSample
						 value:    0
						 action:   self # sample(1))
				     radiobutton(label:    'Every second'
						 variable: VarSample
						 value:    1
						 action:   self # sample(2))
				     radiobutton(label:    'Every 2 seconds'
						 variable: VarSample
						 value:    2
						 action:   self # sample(4))
				     radiobutton(label:    'Every 10 seconds'
						 variable: VarSample
						 value:    3
						 action:   self # sample(20))
				     radiobutton(label:    'Every minute'
						 variable: VarSample
						 value:    4
						 action:   self # sample(120))])]
		  nil}
	 Frame = {New Tk.frame tkInit(parent: self
				      highlightthickness: 0
				      bd:                 4)}
	 Book  = {New Notebook tkInit(parent: Frame
				      width:  PanelWidth
				      height: PanelHeight)}
	 Threads =
	 {MakePage ThreadPage 'Threads' Book
	  [frame(text:    'Threads'
		 height:  90
		 left:    [number(text: 'Created:')
			   number(text: 'Runnable:'
				  color: RunnableColor)]
		 right:   [load(feature: load
				colors:  [RunnableColor])])
	   frame(text:    'Priorities'
		 height:  70
		 left:    [scale(text:    'High relative to Middle:'
				 state:   {System.get priorities}.high
				 feature: high
				 action:  proc {$ N}
					     {System.set
					      priorities(high:N)}
					  end)
			   scale(text: 'Middle relative to Low:'
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
					     {Threads update(nosample 0)}
					  end)])
	   frame(text:    'Runtime'
		 height:  100
		 left:    [time(text:    'Total:'
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
	 {MakePage MemoryPage 'Memory' Book
	  [frame(text:    'Heap Usage'
		 feature: usage
		 height:  90
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
		 height:  130
		 left:    [scale(text:    'Maximal Size:'
				 range:   1#512
				 dim:     'MB'
				 feature: maxSize
				 state:   {System.get gc}.max div MegaByteI
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
			   scale(text:    'Minimal Size:'
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
						 min:       2 * MegaByteI
						 free:      60
						 tolerance: 20)}
					     {Memory update(nosample 0)}
					  end)
			   button(text:'Middle'
				  action: proc {$}
					     {System.set
					      gc(max:       16 * MegaByteI
						 min:       4  * MegaByteI
						 free:      70
						 tolerance: 15)}
					     {Memory update(nosample 0)}
					  end)
			   button(text:'Large'
				  action: proc {$}
					     {System.set
					      gc(max:       64 * MegaByteI
						 min:       16 * MegaByteI
						 free:      80
						 tolerance: 10)}
					     {Memory update(nosample 0)}
					  end)])
	   frame(text:    'Garbage Collector'
		 feature: gc
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
	 {MakePage PsPage 'Problem Solving' Book
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
	 {MakePage OpiPage 'Programming Interface' Book
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
		 height:  50
		 left:    [checkbutton(text:    'Idle'
				       feature: time
				       state:  {System.get messages}.idle
				       action: proc {$ B}
						  {System.set
						   messages(idle: B)}
					       end)
			   checkbutton(text:    'Garbage Collection'
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
		     pack(Frame side:bottom)
		     pack(Book)]}
	 {Book toTop(Threads)}
	 self.manager  = Manager
	 self.threads  = Threads
	 self.memory   = Memory
	 self.opi      = OPI
	 self.ps       = PS
	 self.notebook = Book
	 self.menu     = Menu
	 <<Time.repeat setRepAll(action: update
				 delay:  1000)>>
	 <<Time.repeat go>>
      end

      meth update
	 TopNote = {self.notebook getTop($)}
	 Threads = self.threads
	 Memory  = self.memory
	 S       = @Slice
      in
	 case TopNote
	 of !Threads then
	    {Threads update(both   S)}
	    {Memory  update(sample S)}
	 [] !Memory  then
	    {Threads update(sample S)}
	    {Memory  update(both   S)}
	 else
	    {Threads update(sample S)}
	    {Memory  update(sample S)}
	    {TopNote update}
	 end
      end

      meth shutdown
	 Done
      in
	 {self.menu.panel.shutdown tk(entryconf state:disabled)}
	 _ = {New ShutdownDialog init(master:self done:Done)}
	 thread
	    case Done then {System.shutDown exit}
	    else true
	    end
	    {self.menu.panel.shutdown tk(entryconf state:normal)}
	 end
      end

      meth about
	 Done
      in
	 {self.menu.panel.about tk(entryconf state:disabled)}
	 _ = {New AboutDialog init(master:self done:Done)}
	 thread
	    {Wait Done}
	    {self.menu.panel.about tk(entryconf state:normal)}
	 end
      end

      meth clear
	 {self.ps      clear}
	 {self.threads clear}
      end
      
      meth sample(S)
	 Slice <- S
	 <<Time.repeat setRepDelay(S * 500)>>
      end
      
      meth close
	 <<UrObject    close>>
	 <<Tk.toplevel close>>
	 {self.manager PanelTopClosed}
      end
      
   end


end

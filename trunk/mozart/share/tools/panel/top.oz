%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local
   SmallPad     = 2
   Pad          = 4
   Border       = 2
   LabelWidth   = 6
   TextWidth    = 20
   ButtonWidth  = 6

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
	 P = a(high:10 middle:10) %{System.get priorities}
	 R = {System.get time}
      in
	 case What==nosample then true else
	    {OT.load        display([2.0] Slice)} % set(T.runnable)}
	    {OR.pie         draw(r:R.user g:R.gc c:R.copy
				 p:R.propagate l:R.load)}
	 end
	 case What==sample then true else
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
	 <<ThreadPage update(nosample 0)>>
	 {OT.created     clear}
	 {OR.total       clear}
	 {OR.gc          clear}
	 {OR.copy        clear}
	 {OR.propagation clear}
	 {OR.load        clear}
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
%	 {OP.minSize    set(G.min)}
%	 {OP.maxSize    set(G.max)}
%	 {OP.min        set(G.minFree)}
%	 {OP.max        set(G.maxFree)}
	    {OP.minSize    set(G.minimal)}
	    {OP.maxSize    set(G.maximal)}
	    {OP.min        set(G.increase)}
	    {OP.max        set(G.decrease)}
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
%	 {OS.succeeded set(S.succeeded)}
	 {OS.succeeded set(S.solved)}
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
	 true
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
	 <<Tk.toplevel tkInit(title:              'Oz Panel'
			      highlightthickness: 0)>>
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
				 feature: high
				 action:  proc {$ _} true end)
			   scale(text: 'Middle relative to Low:'
				 feature: middle
				 action:  proc {$ _} true end)]
		 right:   [button(text: 'Default'
				  action: proc {$}
					     {System.set _ priorities(high:   10
								    middle: 10)}
					     {Threads update(nosample)}
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
				 range:   4#512
				 dim:     'MB'
				 feature: maxSize
				 action:  proc {$ _} true end)
			   scale(text:    'Minimal Size:'
				 range:   4#512
				 dim:     'MB'
				 feature: minSize
				 action:  proc {$ _} true end)
			   scale(text:    'Maximal Free:'
				 feature: max
				 action:  proc {$ _} true end)
			   scale(text:    'Minimal Free:'
				 feature: min
				 action:  proc {$ _} true end)]
		 right:   [button(text:   'Small'
				  action: proc {$}
					     {Memory.options.parameter.maxSize
					      set(8)}
					     {Memory.options.parameter.minSize
					      set(1)}
					     {Memory.options.parameter.max
					      set(30)}
					     {Memory.options.parameter.min
					      set(50)}
					  end)
			   button(text:'Middle'
				  action: proc {$}
					     {Memory.options.parameter.maxSize
					      set(32)}
					     {Memory.options.parameter.minSize
					      set(4)}
					     {Memory.options.parameter.max
					      set(50)}
					     {Memory.options.parameter.min
					      set(70)}
					  end)
			   button(text:'Large'
				  action: proc {$}
					     {Memory.options.parameter.maxSize
					      set(128)}
					     {Memory.options.parameter.minSize
					      set(16)}
					     {Memory.options.parameter.max
					      set(70)}
					     {Memory.options.parameter.min
					      set(90)}
					  end)])
	   frame(text:    'Garbage Collector'
		 feature: gc
		 height:  30
		 left:    [checkbutton(text: 'Active')]
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
		 feature: error
		 height:  100
		 left:    [checkbutton(text:    'Show message'
				       feature: message)
			   checkbutton(text:    'Show thread'
				       feature: 'thread')
			   checkbutton(text:    'Show location'
				       feature: location)
			   checkbutton(text: 'Show hints'
				       feature: hints)]
		 right:   [button(text:   'Default'
				  action: proc {$}
					     {OPI.errors.message  set(True)}
					     {OPI.errors.'thread' set(True)}
					     {OPI.errors.location set(True)}
					     {OPI.errors.hints    set(True)}
					  end)])
	   frame(text:    'Output'
		 height:  50
		 left:    [entry(text:    'Maximal print width:'
				 feature: width)
			   entry(text:    'Maximal print depth:'
				 feature: height)]
		 right:   [button(text:  'Default'
				  action: proc {$}
					     {OPI.output.width  set(10)}
					     {OPI.output.height set(10)}
					  end)])
	   frame(text:    'Status Messages'
		 feature: status
		 height:  50
		 left:    [checkbutton(text:    'Runtime')
			   checkbutton(text:    'Garbage Collection'
				       feature: gc)]
		 right:   [button(text:  'Default'
				  action: proc {$}
					     {OPI.status.runtime set(10)}
					     {OPI.status.gc      set(10)}
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
	 {self.menu.panel.shutdown tk(entryconf state:disabled)}
      end

      meth about
	 true
      end

      meth clear
	 {self.ps      clear}
	 {self.threads clear}
      end
      
      meth sample(S)
	 {Show sample(S)}
	 Slice <- S
	 <<Time.repeat setRepDelay(S * 500)>>
      end
      
      meth close
	 <<UrObject    close>>
	 <<Tk.toplevel close>>
%	 <<Time.repeat kill>>
	 {self.manager PanelTopClosed}
      end
      
   end


end





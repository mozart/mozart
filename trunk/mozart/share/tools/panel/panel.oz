%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

declare

Panel

in

\insert labelframe.oz
\insert notebook.oz
\insert piechart.oz
\insert load.oz

local
   
   PanelWidth   = 640
   FrameWidth   = PanelWidth - 30
   PanelHeight  = 370
   DisplayWidth = 10
   DisplayColor = white
   EnterColor   = wheat
   SmallPad     = 2
   Pad          = 4
   Border       = 2
   LabelWidth   = 6
   TextWidth    = 20
   ButtonWidth  = 6

   fun {GetFeature X}
      case {HasFeature X feature} then X.feature
      else {String.toAtom
	    {Map {Filter {VirtualString.toString X.text} Char.isAlNum}
	     Char.toLower}}
      end
   end
   
   class Print
      from Tk.label
      attr Saved: Unit
      meth init(parent:P)
	 <<Print tkInit(parent:P text:0 anchor:e width:LabelWidth)>>
      end
      meth set(N)
	 case N==@Saved then true else
	    Saved <- N
	    <<Print tk(conf text:N)>>
	 end
      end
   end

   class Scale
      from Tk.scale
      attr Saved: Unit
      meth init(parent:P range:R action:A)
	 <<Tk.scale tkInit(parent:    P
			   length:    200
			   'from':    R.1
			   highlightthickness: 0
			   to:        R.2
			   orient:    horizontal
			       width:     8
			   action: A
			   showvalue: True)>>
      end
      meth set(N)
	 case N==@Saved then true else
	    Saved <- N
	    <<Scale tk(set N)>>
	 end
      end
   end
   
   class Checkbutton
      from Tk.checkbutton
      feat Var
      attr Saved: Unit
      meth init(parent:P text:T)
	 V = {New Tk.variable tkInit(False)}
      in
	 <<Tk.checkbutton tkInit(parent:             P
				 highlightthickness: 0
				 text:               T
				 anchor:             w
				 var:                V
				 width:              ButtonWidth)>>
	 self.Var = V
      end
      meth set(N)
	 case N==@Saved then true else
	    Saved <- N
	    {self.Var tkSet(N)}
	 end
      end
   end
   
   class Button
      from Tk.button
      meth init(parent:P text:T action:A)
	 <<Tk.button tkInit(parent:             P
			    highlightthickness: 0
			    text:               T
			    anchor:             w
			    action:             A
			    width:              ButtonWidth)>>
      end
   end
   
   fun {MakeSide Ls N P R TclT}
      case Ls of nil then TclT
      [] L|Lr then TclR TclS in
	 TclS =
	 case {Label L}
	 of scale then
	    L1 = {New Tk.label tkInit(parent: P
				      text:   L.text
				      anchor: e
				      width:  TextWidth)}
	    S1 = {New Scale init(parent: P
				 range:  {CondSelect L range 1#100}
				 action: L.action)}
	    L2 = {New Tk.label tkInit(parent: P
				      text:   {CondSelect L dim ''})}
	 in
	    R.{GetFeature L}=S1
	    grid(L1 sticky:e column:0 row:N) |
	    grid(S1 sticky:e column:1 row:N) | 
	    grid(L2 sticky:e column:2 row:N) | TclR	    
	 [] print    then
	    L1 = {New Tk.label tkInit(parent: P
				      text:   L.text
				      anchor: w
				      width:  TextWidth)}
	    L2 = {New Print init(parent:P)}
	 in
	    R.{GetFeature L}=L2
	    grid(L1 sticky:e column:0 row:N) |
	    grid(L2 sticky:e column:1 row:N) | TclR
	 [] button  then
	    B = {New Button init(parent:P
				 text:L.text
				 action:{CondSelect L action proc {$} true end})}
	 in
	    R.{GetFeature L}=B
	    grid(B sticky:w column:0 row:N) | TclR
	 [] checkbutton  then
	    B = {New Checkbutton tkInit(parent: P
					text:   L.text)}
	 in
	    R.{GetFeature L}=B
	    grid(B sticky:w column:0 row:N) | TclR
	 [] entry then
	    L1 = {New Tk.label tkInit(parent: P
				      text:   L.text
				      anchor: w
				      width:  TextWidth)}
	    E2 = {New Tk.entry tkInit(parent: P
				      bg:     EnterColor
				      width:  LabelWidth)}
	 in
	    R.{GetFeature L}=E2
	    grid(L1 sticky:e column:0 row:N) |
	    grid(E2 sticky:e column:1 row:N) | TclR
	 [] pie then
	    L1 = {New TimePie init(parent: P)}
	 in
	    R.{GetFeature L}=L1
	    grid(L1 sticky:e column:0 row:N) | TclR
	 [] load then
	    L1 = {New LoadImage tkInit(parent: P
				       maxy:   5
				       dim:    {CondSelect L dim ''})}
	 in
	    R.{GetFeature L}=L1
	    grid(L1 sticky:e column:0 row:N) | TclR
	 end
	 TclR={MakeSide Lr N+1 P R TclT}
	 TclS
      end
   end
   
   fun {MakeFrames Fs P R TclT}
      case Fs
      of nil then TclT
      [] F|Fr then TclR TclS
	 Border = {New Labelframe tkInit(parent: P
					 width:  FrameWidth
					 height: F.height
					 text:   F.text)}
	 Frame  = {New Tk.frame tkInit(parent:            Border
				       highlightthickness: 0)}
	 Left   = {New Tk.frame tkInit(parent:            Frame
				       highlightthickness: 0)}
	 Right  = {New Tk.frame tkInit(parent:            Frame
				       highlightthickness: 0)}
	 FR     = {MakeRecord a {Append {Map F.left GetFeature}
				 {Map F.right GetFeature}}}
      in
	 {Border add(Frame)}
	 R.{GetFeature F}=FR
	 {MakeSide F.left 0 Left FR
	  {MakeSide F.right 0 Right FR
	   pack(Border pady:Pad side:top) |
	   pack(Left   side:left  anchor:n) |
	   pack(Right  side:right anchor:s) | {MakeFrames Fr P R TclT}}}
     end
   end
	 
   fun {MakePage Class Mark Book PageSpec}
      R    = {MakeRecord a {Map PageSpec GetFeature}}
      Page = {New Class init(parent:Book options:R)}
   in
%      {Browse R}
      {Book add(note:Page text:' '#Mark#' ')}
      {Tk.batch {MakeFrames PageSpec Page R nil}}
      Page
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

      meth update
	 O  = self.options
	 OT = O.threads
	 OP = O.priorities
	 OR = O.runtime
	 T = {System.get threads}
	 P = a(high:10 middle:10) %{System.get priorities}
	 R = {System.get time}
      in
	 {OT.created     set(T.created)}
	 {OT.runnable    set(T.runnable)}
	 {OT.load        display([2])} % set(T.runnable)}
	 {OP.high        set(P.high)}
	 {OP.middle      set(P.middle)}
	 {OR.total       set(R.user)}
	 {OR.gc          set(R.gc)}
	 {OR.copy        set(R.copy)}
	 {OR.propagation set(R.propagate)}
	 {OR.load        set(R.load)}
	 {OR.pie         draw(r:R.user g:R.gc c:R.copy p:R.propagate l:R.load)}
      end
   end
   
   class MemoryPage
      from UpdatePage

      meth update
	 O  = self.options
	 OG = O.gc
	 OP = O.parameter
	 OU = O.usage
	 G = {System.get gc}
      in
	 {OU.active     set(G.active)}
	 {OU.size       set(G.size)}
	 {OU.threshold  set(G.threshold)}
	 {OU.load       display([G.threshold G.size G.active])}
	 {OP.minSize    set(G.min)}
	 {OP.maxSize    set(G.max)}
	 {OP.min        set(G.minFree)}
	 {OP.max        set(G.maxFree)}
	 {OG.active     set(G.on)}
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
   end
   
   class OpiPage
      from UpdatePage

      meth update
	 true
      end
   end
   
in
   
   class Panel
      from Tk.toplevel

      feat notebook

      
      meth init
	 <<Tk.toplevel tkInit(title:              'Oz Panel'
			      highlightthickness: 0)>>
	 VarSample = {New Tk.variable tkInit(1)}
	 Menu  = {TkTools.menubar self self
		  [menubutton(text: ' Oz Panel '
			      menu: [command(label:   'About...'
					     action:  self # about)
				     separator
				     command(label:   'Shutdown system'
					     action:  self # shutdown)
				     separator
				     command(label:   'Close'
					     action:  self # close)])
		   menubutton(text: ' Sample '
			      menu: [radiobutton(label:    'Every second'
						 variable: VarSample
						 value:    0
						 action:   self # sample(1))
				     radiobutton(label:    'Every 10 seconds'
						 variable: VarSample
						 value:    1
						 action:   self # sample(10))
				     radiobutton(label:    'Every minute'
						 variable: VarSample
						 value:    2
						 action:   self # sample(60))
				     separator
				     checkbutton(label:   'Requires mouse'
						 action: self # mouse)])]
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
		 left:    [print(text: 'Created:')
			   print(text: 'Runnable:')]
		 right:   [load(feature: load)])
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
					     {Threads.options.priorities.high
					      set(10)}
					     {Threads.options.priorities.middle
					      set(10)}
					  end)])
	   frame(text:    'Runtime'
		 height:  100
		 left:    [print(text:    'Total:')
			   print(text:    'Garbage collection:'
				feature: gc)
			   print(text:    'Copy:')
			   print(text:    'Propagation:')
			   print(text:    'Load:')]
		 right:   [pie(feature: pie)])]}
	 Memory =
	 {MakePage MemoryPage 'Memory' Book
	  [frame(text:    'Heap Usage'
		 feature: usage
		 height:  90
		 left:    [print(text:    'Threshold:')
			   print(text:    'Size:')
			   print(text:    'Active size:'
				 feature: active)]
		 right:   [load(feature: load
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
		 left:    [print(text: 'Variables created:'
				 feature: var)
			   print(text:    'Propagators created:'
				 feature: propc)
			   print(text:    'Propagators invoked:'
				 feature: propi)]
		 right:   nil)
	   frame(text:    'Spaces'
		 height:  100
		 left:    [print(text:    'Spaces created:'
				 feature: created)
			   print(text:    'Spaces cloned:'
				 feature: cloned)
			   print(text:    'Spaces failed:'
				 feature: failed)
			   print(text:    'Spaces succeeded:'
				 feature: succeeded)
			   print(text:    'Alternatives chosen:'
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
	 self.notebook = Book
      end

      meth update
	 {{self.notebook getTop($)} update}
      end

   end


end
   
   
/*

declare P={New Panel init}
declare
proc {DO}
   {Delay 500}
   {P update}
   {DO}
end
{DO}
{P update}
{Browse {System.get time}}
{Browse {System.get gc}}
{Browse Unit==False}
*/





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
   LabelWidth   = 8
   TextWidth    = 20
   ButtonWidth  = 6
   BackColor    = white

   fun {GetFeature X}
      case {HasFeature X feature} then X.feature
      else {String.toAtom
	    {Map {Filter {VirtualString.toString X.text} Char.isAlNum}
	     Char.toLower}}
      end
   end
   
   class PrintNumber
      from Tk.label
      attr Saved:~1 Clear:0
      meth init(parent:P color:C)
	 <<PrintNumber tkInit(parent:P text:0 anchor:e width:LabelWidth
			      fg:C bg:BackColor)>>
      end
      meth set(N)
	 case N==@Saved then true else
	    Saved <- N
	    <<PrintNumber tk(conf text:(N-@Clear))>>
	 end
      end
      meth clear
	 Clear <- @Saved
	 Saved <- ~1
      end
   end

   fun {CutTrail N}
      (N div 100)#'.'#(N mod 100)
   end
   
   class PrintTime
      from Tk.label
      feat Dim
      attr Saved:~1 Clear:0
      meth init(parent:P dim:D color:C)
	 <<PrintTime tkInit(parent:P text:0 anchor:e width:LabelWidth
			    fg:C bg:BackColor)>>
	 self.Dim = D
      end
      meth set(N)
	 case N==@Saved then true else
	    C = N - @Clear
	    DimText
	    PrintText
	 in
	    Saved <- N
	    case C>HourI then
	       DimText   = 'h'
	       PrintText = {CutTrail C * 100 div HourI}
	    elsecase N>MinuteI then
	       DimText   = 'm'
	       PrintText = {CutTrail C * 100 div HourI}
	    elsecase N>SecondI then
	       DimText   = 's'
	       PrintText = {CutTrail C * 100 div SecondI}
	    else
	       DimText   = 'ms'
	       PrintText = C
	    end
	    {self.Dim tk(conf text:DimText)}	       
	    <<PrintTime tk(conf text:PrintText)>>
	 end
      end
      meth clear
	 Clear <- @Saved
	 Saved <- ~1
      end
   end

   class Scale
      from Tk.scale
      attr Saved: Unit
      meth init(parent:P range:R action:A state:S)
	 <<Tk.scale tkInit(parent:    P
			   length:    200
			   'from':    R.1
			   highlightthickness: 0
			   to:        R.2
			       orient:    horizontal
			   action: self # noop
			       width:     8
			       showvalue: True)>>
	 <<Tk.scale tk(set S)>>
	 <<Tk.scale tkAction(proc {$ X}
				{A {Tk.string.toInt X}}
			     end)>>
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
      feat Var Action
      attr Saved:False
      meth init(parent:P text:T action:A state:S)
	 V = {New Tk.variable tkInit(S)}
      in
	 <<Tk.checkbutton tkInit(parent:             P
				 highlightthickness: 0
				 text:               T
				 anchor:             w
				 var:                V
				 action:             self # invoke
				 width:              TextWidth)>>
	 self.Var    = V
	 self.Action = A
	 Saved <- S
      end
      meth invoke
	 Saved <- {Not @Saved}
	 {self.Action @Saved}
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

   class Entry
      from Tk.entry
      feat Action Top
      attr Save: Unit
      meth init(parent:P action:A top:T)
	 <<Tk.entry tkInit(parent: P
			   bg:     EnterColor
			   width:  LabelWidth)>>
	 <<Tk.entry tkBind(event:  '<Return>'
			   action: self # take)>>
	 self.Action = A
	 self.Top    = T
      end
      meth take
	 O = @Save
	 N = <<Tk.entry tkReturnInt(get $)>>
      in
	 Save <- N
	 case {IsInt N} andthen N>=0 then
	    {self.Action N} {Tk.send focus(self.Top)}
	 else <<Entry set(O)>>
	 end
      end
      meth set(N)
	 case N==@Save then true else
	    Save <- N
	    <<Tk.entry tk(delete 0 'end')>>
	    <<Tk.entry tk(insert 0 N)>>
	 end
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
				      anchor: w
				      width:  TextWidth)}
	    S1 = {New Scale init(parent: P
				 range:  {CondSelect L range 1#100}
				 action: L.action
				 state:  L.state)}
	    L2 = {New Tk.label tkInit(parent: P
				      text:   {CondSelect L dim ''})}
	 in
	    R.{GetFeature L}=S1
	    grid(L1 sticky:e column:0 row:N) |
	    grid(S1 sticky:e column:1 row:N) | 
	    grid(L2 sticky:e column:2 row:N) | TclR	    
	 [] number    then
	    L1 = {New Tk.label tkInit(parent: P
				      text:   L.text
				      anchor: w
				      width:  TextWidth)}
	    L2 = {New PrintNumber init(parent:P
				       color: {CondSelect L color black})}
	 in
	    R.{GetFeature L}=L2
	    grid(L1 sticky:e column:0 row:N) |
	    grid(L2 sticky:e column:1 row:N) | TclR
	 [] size then
	    L1 = {New Tk.label tkInit(parent: P
				      text:   L.text
				      anchor: w
				      width:  TextWidth)}
	    L2 = {New PrintNumber init(parent:P
				       color: {CondSelect L color black})}
	    L3 = {New Tk.label tkInit(parent: P
				      anchor: e
				      text:   'KB')}
	 in
	    R.{GetFeature L}=L2
	    grid(L1 sticky:e column:0 row:N) |
	    grid(L2 sticky:e column:1 row:N) | 
	    grid(L3 sticky:e column:2 row:N) | TclR
	 [] time then
	    L1 = {New Tk.label tkInit(parent: P
				      text:   L.text
				      anchor: w
				      width:  TextWidth)}
	    L3 = {New Tk.label  tkInit(parent:P anchor:w width:4)}
	    L2 = {New PrintTime init(parent: P
				     color:  {CondSelect L color black}
				     dim:    L3)}
	 in
	    R.{GetFeature L}=L2
	    grid(L1 sticky:e column:0 row:N) |
	    grid(L2 sticky:e column:1 row:N) | 
	    grid(L3 sticky:e column:2 row:N) | TclR
	 [] button  then
	    B = {New Button init(parent: P
				 text:   L.text
				 action: {CondSelect L action
					  proc {$} true end})}
	 in
	    R.{GetFeature L}=B
	    grid(B sticky:w column:0 row:N) | TclR
	 [] checkbutton  then
	    B = {New Checkbutton init(parent: P
				      state:  L.state
				      text:   L.text
				      action: L.action)}
	 in
	    R.{GetFeature L}=B
	    grid(B sticky:w column:0 row:N) | TclR
	 [] entry then
	    L1 = {New Tk.label tkInit(parent: P
				      text:   L.text
				      anchor: w
				      width:  TextWidth)}
	    E2 = {New Entry init(parent: P
				 action: L.action
				 top:    L.top)}
	 in
	    R.{GetFeature L}=E2
	    grid(L1 sticky:e column:0 row:N) |
	    grid(E2 sticky:e column:1 row:N) | TclR
	 [] pie then
	    L1 = {New Pie init(parent: P)}
	 in
	    R.{GetFeature L}=L1
	    grid(L1 sticky:e column:0 row:N) | TclR
	 [] load then
	    L1 = {New Load init(parent: P
				colors: L.colors
				maxy:   5.0
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

in
   
   fun {MakePage Class Mark Book PageSpec}
      R    = {MakeRecord a {Map PageSpec GetFeature}}
      Page = {New Class init(parent:Book options:R)}
   in
      {Book add(note:Page text:' '#Mark#' ')}
      thread
	 {Tk.batch {MakeFrames PageSpec Page R nil}}
      end
      Page
   end

end

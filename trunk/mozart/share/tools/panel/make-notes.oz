%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local
   Border       = 2
   LargeBorder  = 5
   LabelWidth   = 8
   SquareSize   = 10
   ButtonWidth  = 6

   fun {GetFeature X}
      case {HasFeature X feature} then X.feature
      else {String.toAtom
	    {Map {Filter {VirtualString.toString X.text} Char.isAlNum}
	     Char.toLower}}
      end
   end

   class Square
      from Tk.canvas
      meth init(parent:P color:C stipple:S)
	 <<Square tkInit(parent:             P
			 width:              SquareSize
			 height:             SquareSize
			 bd:                 Border
			 relief:             groove
			 highlightthickness: 0)>>
	 <<Square tk(crea rectangle ~2 ~2 SquareSize+2 SquareSize+2
		     fill:C stipple:S)>>
      end
   end
   
   class PrintNumber
      from Tk.label
      attr Saved:~1 Clear:0
      meth init(parent:P)
	 <<PrintNumber tkInit(parent:P text:0 anchor:e width:LabelWidth)>>
      end
      meth set(N)
	 case N==@Saved then skip else
	    Saved <- N
	    <<PrintNumber tk(conf text:(N-@Clear))>>
	 end
      end
      meth clear
	 Clear <- @Saved
	 Saved <- ~1
	 <<PrintNumber tk(conf text:0)>>
      end
   end

   fun {CutTrail N}
      Head = N div 100
      Tail = N mod 100
   in
      Head#'.'#case Tail<10 then '0'#Tail else Tail end
   end
   
   class PrintTime
      from Tk.label
      feat Dim
      attr Saved:~1 Clear:0
      meth init(parent:P dim:D)
	 <<PrintTime tkInit(parent:P text:0 anchor:e width:LabelWidth)>>
	 self.Dim = D
      end
      meth set(N)
	 case N==@Saved then skip else
	    C = N - @Clear
	    DimText
	    PrintText
	 in
	    Saved <- N
	    case C>HourI then
	       DimText   = 'h'
	       PrintText = {CutTrail C * 100 div HourI}
	    elsecase C>MinuteI then
	       DimText   = 'm'
	       PrintText = {CutTrail C * 100 div MinuteI}
	    elsecase C>SecondI then
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
	 {self.Dim tk(conf text:ms)}	       
	 <<PrintTime tk(conf text:0)>>
	 Clear <- @Saved
	 Saved <- ~1
      end
   end

   class Scale
      from Tk.scale
      attr Saved:0
      meth init(parent:P range:R action:A state:S)
	 <<Tk.scale tkInit(parent:             P
			   length:             200
			   font:               ScaleFont
			   'from':             R.1
			   highlightthickness: 0
			   to:                 R.2
			   orient:             horizontal
			   action:             self # noop
			   width:              8
			   showvalue:          True)>>
	 Saved <- S
	 <<Tk.scale tk(set S)>>
	 <<Tk.scale tkAction(action:A args:[int])>>
      end
      meth set(N)
	 case N==@Saved then skip else
	    Saved <- N
	    <<Scale tk(set N)>>
	 end
      end
      meth get($)
	 @Saved
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
				 action:             self # invoke)>>
	 self.Var    = V
	 self.Action = A
	 Saved <- S
      end
      meth invoke
	 Saved <- {Not @Saved}
	 {self.Action @Saved}
      end
      meth set(N)
	 case N==@Saved then skip else
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
	 case N==@Save then skip else
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
				      anchor: w)}
	    S1 = {New Scale init(parent: P
				 range:  {CondSelect L range 1#100}
				 action: L.action
				 state:  L.state)}
	    L2 = {New Tk.label tkInit(parent: P
				      anchor: w
				      text:   {CondSelect L dim ''})}
	 in
	    R.{GetFeature L}=S1
	    grid(L1 sticky:w column:0 row:N) |
	    grid(S1 sticky:e column:1 row:N) | 
	    grid(L2 sticky:w column:2 row:N) | TclR	    
	 [] number    then
	    L1 = {New Tk.label tkInit(parent: P
				      text:   L.text
				      anchor: w)}
	    L2 = {New PrintNumber init(parent:P)}
	    L3 = case {HasFeature L color} orelse {HasFeature L stipple} then
		    C = {CondSelect L color black}
		    S = {CondSelect L stipple ''}
		 in {New Square init(parent:P color:C stipple:S)}
		 else {New Tk.frame tkInit(parent:P)}
		 end
	 in
	    R.{GetFeature L}=L2
	    grid(L1 sticky:w column:0 row:N) |
	    grid(L2 sticky:e column:1 row:N) | 
	    grid(L3 padx:Pad sticky:e column:2 row:N) | TclR
	 [] size then
	    L1 = {New Tk.label tkInit(parent: P
				      text:   L.text
				      anchor: w)}
	    L2 = {New PrintNumber init(parent:P)}
	    L3 = {New Tk.label tkInit(parent: P
				      anchor: e
				      text:   {CondSelect L dim 'KB'})}
	    L4 = case {HasFeature L color} orelse {HasFeature L stipple} then
		    C = {CondSelect L color black}
		    S = {CondSelect L stipple ''}
		 in {New Square init(parent:P color:C stipple:S)}
		 else {New Tk.frame tkInit(parent:P)}
		 end
	 in
	    R.{GetFeature L}=L2
	    grid(L1 sticky:w column:0 row:N) |
	    grid(L2 sticky:e column:1 row:N) | 
	    grid(L3 sticky:e column:2 row:N) | 
	    grid(L4 padx:Pad sticky:e column:3 row:N) | TclR
	 [] time then
	    L1 = {New Tk.label tkInit(parent: P
				      text:   L.text
				      anchor: w)}
	    L3 = {New Tk.label  tkInit(parent:P anchor:e)}
	    L2 = {New PrintTime init(parent: P
				     dim:    L3)}
	    L4 = case {HasFeature L color} orelse {HasFeature L stipple} then
		    C = {CondSelect L color black}
		    S = {CondSelect L stipple ''}
		 in {New Square init(parent:P color:C stipple:S)}
		 else {New Tk.frame tkInit(parent:P)}
		 end
	 in
	    R.{GetFeature L}=L2
	    grid(L1 sticky:w column:0 row:N) |
	    grid(L2 sticky:e column:1 row:N) | 
	    grid(L3 sticky:w column:2 row:N) | 
	    grid(L4 padx:Pad sticky:e column:3 row:N) | TclR
	 [] button  then
	    B = {New Button init(parent: P
				 text:   L.text
				 action: {CondSelect L action
					  proc {$} skip end})}
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
				      anchor: w)}
	    E2 = {New Entry init(parent: P
				 action: L.action
				 top:    L.top)}
	 in
	    R.{GetFeature L}=E2
	    grid(L1 sticky:w column:0 row:N) |
	    grid(E2 sticky:w column:1 row:N) | TclR
	 [] timebar then
	    F  = {New Tk.frame tkInit(parent:            P
				      highlightthickness:0)}
	    L1 = {New RuntimeBar init(parent: F)}
	 in
	    R.{GetFeature L}=L1
	    grid(F sticky:ens column:0 row:N) |
	    pack(L1 side:top pady:40) | TclR
	 [] load then
	    L1 = {New Load init(parent:  P
				colors:  L.colors
				stipple: L.stipple
				maxy:    5.0
				dim:     {CondSelect L dim ''})}
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
	 Border = {New TkTools.textframe tkInit(parent: P
						text:   F.text)}
	 Left   = {New Tk.frame tkInit(parent:            Border.inner
				       highlightthickness: 0)}
	 Right  = {New Tk.frame tkInit(parent:            Border.inner
				       border:            LargeBorder
				       highlightthickness: 0)}
	 FR     = {MakeRecord a frame|{Append {Map F.left GetFeature}
				 {Map F.right GetFeature}}}
      in
	 FR.frame = Border
	 R.{GetFeature F}=FR
	 {MakeSide F.left 0 Left FR
	  {MakeSide F.right 0 Right FR
	   pack(Left   side:left  anchor:nw) |
	   pack(Right  side:right anchor:se) |
	   case {CondSelect F pack True} then
	      pack(Border fill:x side:top padx:3) | {MakeFrames Fr P R TclT}
	   else {MakeFrames Fr P R TclT}
	   end}}
     end
   end

in
   
   fun {MakePage Class Mark Book Add PageSpec}
      R    = {MakeRecord a {Map PageSpec GetFeature}}
      Page = {New Class init(parent:Book options:R text:' '#Mark#' ')}
   in
      {Tk.batch {MakeFrames PageSpec Page R nil}}
      case Add then {Book add(Page)} else skip end
      Page
   end

end

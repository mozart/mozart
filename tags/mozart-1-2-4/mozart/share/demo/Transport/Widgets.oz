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

import
   Tk
   TkTools

   Configure(images:    Images
	     colors:    Colors
	     fonts:     Fonts
	     capacity:  Capacity
	     delta:     Delta
	     delay:     DelayMove)
   Country(getCoord
	   width
	   height
	   getGraph
	   getRoute)
   
export
   History
   EntryChooser
   Truck
   map:         CountryMap

define

   local
      An            = 'announce: '
      Sp            = '          '
      HistoryWidth  = 60
      HistoryHeight = 8
      HistoryFont   = '-adobe-helvetica-medium-r-normal-*-*-100*'
      HistoryBg     = '#fffff0'
   in
      
      class History from Tk.toplevel
	 attr
	    mapped:false
	 feat
	    text entry var
	    
	 meth init(master:M suffix:U)
	    History, tkInit(parent:M.toplevel title:'History: '#U withdraw:true
			    delete: self # unmap)
	    F = {New TkTools.textframe tkInit(parent:self text:'History: '#U)}
	    T = {New Tk.text tkInit(parent:F.inner
				    width:HistoryWidth height:HistoryHeight
				    bg:HistoryBg font:HistoryFont
				    highlightthickness: 0)}
	    S = {New Tk.scrollbar tkInit(parent:F.inner)}
	    V = {New Tk.variable tkInit(false)}
	    E = {New Tk.menuentry.checkbutton
		 tkInit(parent:   M.menu.windows.menu
			label:    U
			variable: V
			action:   self # toggle)}
	 in
	    {T tkBind(event:'<Map>'   action: self # map)}
	    {T tkBind(event:'<Unmap>' action: self # unmap)}
	    {Tk.addYScrollbar T S}
	    {Tk.batch [pack(T S side:left fill:y) pack(F)]}
	    self.text  = T
	    self.entry = E
	    self.var   = V
	 end
	 
	 meth setMapped(B)
	    mapped <- B
	    {self.var tkSet(B)}
	 end
	 
	 meth toggle
	    {Tk.send wm(if @mapped then
			   iconify
			else
			   deiconify
			end self)}
	    History, setMapped({Not @mapped})
	 end
	 
	 meth map
	    if {Not @mapped} then
	       {Tk.send wm(deiconify self)}
	    end
	    History, setMapped(true)
	 end
	 
	 meth unmap
	    if @mapped then
	       {Tk.send wm(iconify self)}
	    end
	 History, setMapped(false)
	 end
	 
	 meth print(V)
	    Text = self.text
	 in
	    {Text tk(insert 'end' V#'\n')}
	    {Text tk(yview pickplace:'end')}
	 end
	 
	 meth announce(what:W weight:N src:S dst:D reply:R<=unit ...)
	    History,print(An#W#', '#N#'\n'#
			  Sp#'from: '#S#' to: '#D#
			  case R
			  of grant  then '\n'#Sp#'granted'
			  [] reject then '\n'#Sp#'rejected'
			  else ''
			  end)
	 end
	 
	 meth tkClose
	    {self.entry tkClose}
	    Tk.toplevel, tkClose
	 end
      end
      
   end
   
   local
      proc {DefAction _}
	 skip
      end
      TextHeight    = 6
      BigTextWidth  = 17
   in
      class EntryChooser
	 from Tk.frame
	 feat entry button toplevel action
	 attr entries
	    
	 meth tkInit(parent:P toplevel:T entries:Es action:A<=DefAction)
	    Tk.frame, tkInit(parent:P highlightthickness:2)
	    Entry  = {New Tk.entry tkInit(parent:             self
					  width:              BigTextWidth
					  bg:                 Colors.textBg
					  font:               Fonts.text
					  highlightthickness: 0)}
	    Button = {New Tk.button tkInit(parent: self
					   image: Images.down
					   highlightthickness:0
					   state:  if Es==nil then disabled
						   else normal
						   end
					   action: self # OpenChooser)}
	 in
	    self.entry    = Entry
	    self.button   = Button
	    self.toplevel = T
	    self.action   = A
	    EntryChooser,entries(Es)
	    {Tk.send pack(self.entry self.button side:left fill:y)}
	 end
	 
	 meth entries(Es)
	    entries <- Es
	    if Es\=nil then
	       {self.entry tk(delete 0 'end')}
	       {self.entry tk(insert 0 Es.1)}
	    end
	 end
	 
	 meth OpenChooser
	    [X Y H] = {Map [rootx rooty height]
		       fun {$ WI} {Tk.returnInt winfo(WI self)} end}
	    T = {New Tk.toplevel tkInit(withdraw: true
					parent:   self
					cursor:   top_left_arrow)}
	    F = {New Tk.frame tkInit(parent:T bg:black bd:2)}
	    L = {New Tk.listbox   tkInit(parent:F height:TextHeight
					 width:BigTextWidth
					 bg:white
					 exportselection:false)}
	    S = {New Tk.scrollbar tkInit(parent:F width:10)}
	 in
	    {L tk(insert 0 b(@entries))}
	    {Tk.addYScrollbar L S}
	    {self.toplevel tkBind(event:'<1>' action:T#tkClose)}
	    {L tkBind(event:'<1>'
		      action: proc {$}
				 A={L tkReturnAtom(get
						   l(L curselection) $)}
			      in
				 {self.entry tk(delete 0 'end')}
				 {self.entry tk(insert 0 A)}
				 {self.action A}
				 {T tkClose}
			      end)}
	    {L tkBind(event:'<2>'
		      action: T # tkClose)}
	    {Tk.batch [wm(overrideredirect T true)
		       wm(geometry T '+'#X#'+'#Y+H)
		       pack(L S side:left fill:both)
		       pack(F)
		       wm(deiconify T)]}
	 end
	 
      end
   end

   local
      LoadLeftX   =  ~3.0
      LoadRightX  = ~24.0
      LoadY       =  ~8.0
      LoadHeight  =   6.0
      LoadWidth   =  27.0

      ColMan      = {New class $ from BaseObject
			    prop final
			    attr ColS: local Cs in
					  Cs={Append Colors.truck Cs} Cs
				       end
			    meth get(?Col) ColR in Col|ColR=(ColS<-ColR) end
			 end noop}
   in
      class Truck
	 from Tk.canvasTag
	 prop final
	 feat
	    parent load fill driver
	 attr
	    city: unit
	    turn: left
	    load: 0
	    x:    0.0
	    y:    0.0
	    
	 meth init(parent:P city:C driver:D)
	    X Y
	 in
	    {Country.getCoord C ?X ?Y}
	    city <- C
	    Truck, tkInit(parent:P)
	    x <- {IntToFloat X}
	    y <- {IntToFloat Y}
	    self.parent = P
	    self.load   = {New Tk.canvasTag tkInit(parent:P)}
	    self.fill   = {ColMan get($)}
	    self.driver = D
	    Truck, draw
	 end
	 
	 meth draw
	    P = self.parent
	    D = @turn
	    X = @x
	    Y = @y
	 in
	    Truck, tk(delete)
	    if Tk.isColor then
	       %% Create the truck's window
	       {P tk(create image X Y 
		     image: Images.truck.win.D
		     tags:  self)}
	       {P tk(create image X Y
		     image: Images.truck.fill.(self.fill).D
		     tags:  self)}
	    end
	    %% Create the frame for truck (better visibility) 
	    {P tk(create image X Y 
		  image: Images.truck.frame.D
		  tags:  self)}
	    {P tk(crea rectangle 0 0 0 0
		  fill:    Colors.good
		  outline: ''
		  tags:    q(self self.load))}
	    Truck, load(@load)
	 end
	 
	 meth load(L)
	    W  = {IntToFloat L} / {IntToFloat Capacity} * LoadWidth
	    X0 = @x + case @turn
		      of left  then LoadLeftX
		      [] right then LoadRightX + LoadWidth - W
		      end
	    X1 = X0 + W
	    Y0 = @y + LoadY
	    Y1 = Y0 + LoadHeight
	 in
	    load <- L
	    {self.load tk(coords X0 Y0 X1 Y1)}
	 end
	 
	 meth turn(X0 X1)
	    NewTurn = if X0<X1 then right else left end
	 in
	    if @turn\=NewTurn then
	       turn <- NewTurn Truck, draw
	    end
	 end
	 
	 meth drive(Dst Load NextLoad)
	    X Y
	 in
	    {Country.getCoord @city ?X ?Y}
	    Truck,load(Load)
	    Truck,Route({Country.getRoute @city Dst}
			{IntToFloat X} {IntToFloat Y})
	    Truck,load(NextLoad)
	    Truck,{self.driver getMessage($)}
	 end
	 
	 meth Move(N XS YS)
	    if N\=0 then
	       Truck,tk(move XS YS)
	       x <- @x + XS y <- @y + YS
	       {Delay DelayMove}
	       Truck,Move(N-1 XS YS)
	    end
	 end
	 
	 meth Route(Rs SrcX SrcY)
	    %% Moves the truck according to the route "Rs"
	    Src#Dist|Rr = Rs
	 in
	    case Rr of Dst#_|_ then
	       Steps = Dist div Delta
	       Ratio = {IntToFloat Steps}
	       DX DY
	       {Country.getCoord Dst ?DX ?DY}
	       DstX  = {IntToFloat DX}
	       DstY  = {IntToFloat DY}
	    in
	       %% Turn the truck
	       Truck,turn(SrcX DstX)
	       Truck,Move(Steps (DstX - SrcX) / Ratio (DstY - SrcY) / Ratio)
	       %% correct
	       Truck,tk(move DstX - @x DstY - @y)
	       x <- DstX
	       y <- DstY
	       Truck,Route(Rr DstX DstY)
	    [] nil then
	       city <- Src
	    end
	 end
	 
	 meth close
	    Truck, tkClose
	 end
	 
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
			     relief: sunken
			     bd:     3
			     width:  Country.width
			     height: Country.height
			     bg:     Colors.back)
	    {ForAll {Country.getGraph}
	     proc {$ SPDs}
		Src#(SrcX#SrcY)#Dsts = SPDs
		Tag                  = {New Tk.canvasTag tkInit(parent:self)}
	     in
		{Tag tkBind(event:'<1>' action:P # putSrc(Src))}
		{Tag tkBind(event:'<2>' action:P # putDst(Src))}
		{ForAll Dsts
		 proc {$ DstX#DstY}
		    {self tk(create line SrcX SrcY DstX DstY
			     fill: Colors.street)}
		 end}
		{self tk(create rectangle
			 SrcX-TownSize SrcY-TownSize
			 SrcX+TownSize SrcY+TownSize
			 fill: Colors.city
			 tags: Tag)}
		{self tk(create text SrcX SrcY+TextOffset
			 text: Src
			 font: Fonts.text
			 tags: Tag)}
	     end}
	 end
	 
      end
      
   end
   

end


functor

import
   Tk

export
   CopyCanvas
   GetPSOptions

define

   NoArgs={NewName}

   proc{CopyCanvas Source Dest DX DY}
      % trouve l'objet le plus au bas de la pile d'affichage
      fun{Loop Tag}
	 NT={New Tk.canvasTag tkInit(parent:Source)} in
	 {Source tk(addtag NT below Tag)}
	 if {Source tkReturnAtom(type(NT) $)}=='' then % plus d'objets au dessous
	    Tag
	 else
	    {Source tk(dtag Tag)}
	    {Loop NT}
	 end
      end
      FTag={New Tk.canvasTag tkInit(parent:Source)}
      {Source tk(addtag FTag closest 0 0)} % selection d'un objet de depart
      Start={Loop FTag} % passage a l'objet le plus bas
      % recree un nouveau canvas en fonction de l'ancien
      fun{Copy Tag}
	 fun{GetParams R L}
	    RetRec={NewCell R}
	 in
	    {ForAll L
	     proc{$ P}
		MyP={List.last {Source tkReturnList(itemconfigure(Tag "-"#P) $)}}
%		MyP={Source tkReturn(itemconfigure(Tag {StringToAtom {VirtualString.toString "-"#P}}) $)}
		{Wait MyP}
	     in
		if MyP=="{}" then skip else
		   {Assign RetRec {AdjoinAt {Access RetRec} P MyP}}
		end
	     end}
	    {Access RetRec}
	 end
	 C={Source tkReturnListFloat(coords(Tag) $)}
	 B={Source tkReturnListFloat(bbox(Tag) $)}
	 S={Source tkReturnAtom(type(Tag) $)}
	 Tr=tk(crea S b(C))
	 R
      in
	 case S
	 of '' then skip
	 [] 'arc' then
	    R={GetParams Tr [extent fill outline start stipple style]} % drop width=>unscalable
	 [] 'bitmap' then
	    F={List.last {Source tkReturnList(itemconfigure(Tag "-foreground") $)}} in
	    R=tk(crea rect b(B) stipple:gray25 fill:F outline:F)
	 [] 'image' then
	    R=tk(crea rect b(B) stipple:gray25 fill:black outline:black)
	 [] 'line' then
	    R1={GetParams Tr [fill stipple smooth capstyle joinstyle splinesteps]} %drop arrows=>unscalable
	 in
	    R={Record.adjoinAt R1 smooth R1.smooth=="bezier"}
	 [] 'oval' then
	    R={GetParams Tr [fill stipple outline]}
	 [] 'polygon' then
	    R1={GetParams Tr [fill smooth outline stipple splinesteps]}
	 in
	    R={Record.adjoinAt R1 smooth R1.smooth=="bezier"}
	 [] 'rectangle' then
	    R={GetParams Tr [fill stipple outline]}
	 [] 'text' then
	    R={GetParams tk(crea rect b(B) stipple:gray25 outline:black) [fill]}
	 else R=tk(type Tag)
	 end
	 if {IsFree R} then
	    false
	 else
	    {Dest R}
	    true
	 end
      end
      proc{CopyCanvas Tag}
	 if {Copy Tag} then
	    NT in
	    NT={New Tk.canvasTag tkInit(parent:Dest)}
	    {Source tk(addtag NT above Tag)}
	    {Source tk(dtag Tag)}
	    {CopyCanvas NT} % copie le suivant
	 else skip end
      end
   in
      {CopyCanvas Start}
      {Dest tk(move all ~DX ~DY)} % ramene l'origine a (0,0)
   end

   proc{GetPSOptions IP Return}
      Canvas=IP.canvas
      Title={CondSelect IP title "Postscript Generation Options"}
      SelReg={CondSelect IP region [0 0 200 200]}
      InitZoom={CondSelect IP zoom 1.0}
      InitRotate={CondSelect IP rotate false}
      InitFit={CondSelect IP fit false}
      InitPageW={CondSelect IP pagewidth "210m"}
      InitPageH={CondSelect IP pageheight "297m"}
      InitPageX={CondSelect IP pagex "10m"}
      InitPageY={CondSelect IP pagey "10m"}
      InitW={CondSelect IP width NoArgs}
      InitH={CondSelect IP height NoArgs}
      InitColor={CondSelect IP colormode mono}
      InitX={CondSelect IP x 0}
      InitY={CondSelect IP y 0}
      Zoom={NewCell 1.0}
      X1 Y1 X2 Y2 Width Height
      PageZoom={Tk.returnFloat tk(scaling)}
      T
       F1
        F2 L1 C 
        F3 L2 E1 L3 E2 L4
        F4 L5 L51 E3 L6 E4 L7
        B1
       F5
        F51 L8 C2 R1 R2 L18 L19 V1
        L9 Menu M1 F6
        F7 V2 R3 R4 R5
        F8 L11 E5 L12 B2 B3
        F9 L13 L14 L15 L16 E6 E7 E8 E9 L17 B4 B41
       F10 B5 B6
      Read
      Z={NewPort Read}
      class MyClass

	 feat pagetag ps ctag cbtag cctag ptag pbtag pctag

	 attr rotate pagew pageh pagex pagey w h x y zoom state orgx orgy
	    fit
	    ppw pph ppx ppy scale dx dy color
	    
	 meth init
	    {CopyCanvas Canvas C X1 Y1}
	    self.ps=['Custom'#""#""
		     'Fit To Canvas'#""#"Fit"
		     'Letter (8.5"x11")'#"8.5i"#"11i"
		     'Legal (8.5"x14")'#"8.5i"#"14i"
		     'Tabloïd (11"x17")'#"11i"#"17i"
		     'A (8,5"x11")'#"8.5i"#"11i"
		     'B (11"x17")'#"11i"#"17i"
		     'C (17"x22")'#"17i"#"22i"
		     'D (22"x34")'#"22i"#"34i"
		     'E (34"x44")'#"34i"#"44i"
		     'A4 (210mmx297mm)'#"210m"#"297m"
		     'A3 (297mmx420mm)'#"297m"#"420m"
		     'A2 (420mmx594mm)'#"420m"#"594m"
		     'A1 (594mmx841mm)'#"594m"#"841m"
		     'A0 (841mmx1189mm)'#"841m"#"1189m"
		     'B5(JIS) (182mmx257mm)'#"182m"#"257m"]
	    if InitFit then
	       case {Nth self.ps 2} of L#_#_ then
		  {M1 tk(configure menu:Menu text:L)}
	       end
	       fit<-true
	    else
	       case {Nth self.ps 1} of L#_#_ then
		  {M1 tk(configure menu:Menu text:L)}
	       end
	       fit<-false
	    end
	    {List.forAllInd self.ps
	     proc{$ I E}
		case E of L#W#H then
		   _={New Tk.menuentry.command tkInit(parent:Menu
						      label:L
						      action:Z#selectps(L W H))}
		   if @fit==false andthen W==InitPageW andthen H==InitPageH then
		      {M1 tk(configure menu:Menu text:L)}
		   else skip end
		end
	     end}
	    if InitW==NoArgs then
	       w<-Width
	    else
	       if {IsInt InitW} then
		  w<-{IntToFloat InitW}
	       else
		  w<-InitW
	       end
	    end
	    if InitH==NoArgs then
	       h<-Height
	    else
	       if {IsInt InitH} then
		  h<-{IntToFloat InitH}
	       else
		  h<-InitH
	       end
	    end
	    if {IsInt InitX} then
	       x<-{IntToFloat InitX}
	    else
	       x<-InitX
	    end
	    if {IsInt InitY} then
	       y<-{IntToFloat InitY}
	    else
	       y<-InitY
	    end
	    {self updcanvasentry}
	    self.ctag={New Tk.canvasTag tkInit(parent:C)}
	    self.cbtag={New Tk.canvasTag tkInit(parent:C)}
	    {self.cbtag tkBind(event:"<Enter>"
			       action:Z#setcursor(C fleur))}
	    {self.cbtag tkBind(event:"<Leave>"
			       action:Z#setcursor(C left_ptr))}
	    {self.cbtag tkBind(event:"<1>"
			       args:[float(x) float(y)]
			       action:Z#startcmove)}
	    self.cctag={New Tk.canvasTag tkInit(parent:C)}
	    {self.cctag tkBind(event:"<Enter>"
			       action:Z#setcursor(C sizing))}
	    {self.cctag tkBind(event:"<Leave>"
			       action:Z#setcursor(C left_ptr))}
	    {self.cctag tkBind(event:"<1>"
			       args:[float(x) float(y)]
			       action:Z#startcsize)}
	    {C tkBind(event:"<B1-Motion>"
		      args:[float(x) float(y)]
		      action:Z#cmove)}
	    {C tkBind(event:"<B1-ButtonRelease>"
		      args:[float(x) float(y)]
		      action:Z#crelease)}
	    {self drawcsel}
	    zoom<-InitZoom
	    {E5 tk(insert 0 {FloatToInt InitZoom*100.0})}
	    self.pagetag={New Tk.canvasTag tkInit(parent:C2)}
	    pageh<-InitPageH
	    pagew<-InitPageW
	    pagex<-InitPageX
	    pagey<-InitPageY
	    rotate<-InitRotate
	    {self updpageentry}
	    {self calcsize}
	    {self drawpage}
	    self.ptag={New Tk.canvasTag tkInit(parent:C2)}
	    self.pbtag={New Tk.canvasTag tkInit(parent:C2)}
	    self.pctag={New Tk.canvasTag tkInit(parent:C2)}
	    {self.pbtag tkBind(event:"<Enter>"
			       action:Z#setcursor(C2 fleur))}
	    {self.pbtag tkBind(event:"<Leave>"
			       action:Z#setcursor(C2 left_ptr))}
	    {self.pbtag tkBind(event:"<1>"
			       args:[float(x) float(y)]
			       action:Z#startpmove)}
	    {self.pctag tkBind(event:"<Enter>"
			       action:Z#setcursor(C2 sizing))}
	    {self.pctag tkBind(event:"<Leave>"
			       action:Z#setcursor(C2 left_ptr))}
	    {self.pctag tkBind(event:"<1>"
			       args:[float(x) float(y)]
			       action:Z#startpsize)}
	    {C2 tkBind(event:"<B1-Motion>"
		      args:[float(x) float(y)]
		      action:Z#pmove)}
	    {C2 tkBind(event:"<B1-ButtonRelease>"
		      args:[float(x) float(y)]
		      action:Z#prelease)}
	    {self drawpsel}
	    color<-InitColor
	    state<-0
	 end

	 meth startcmove(X Y)
	    state<-1 % mode deplacement de la selection
	    orgx<-X orgy<-Y
	 end

	 meth startcsize(X Y)
	    state<-2 % mode resize de la selection
	    orgx<-X orgy<-Y
	 end

	 meth cmove(X Y)
	    Scale={Access Zoom} in
	    case @state
	    of 1 then % deplacement de la selection
	       DX=(X-@orgx)/Scale
	       DY=(Y-@orgy)/Scale
	       NX=@x+DX
	       NY=@y+DY
	    in
	       if NX<0.0 then
		  x<-0.0
	       elseif NX+@w>=Width then
		  x<-Width-@w
	       else
		  x<-NX
		  orgx<-X
	       end
	       if NY<0.0 then
		  y<-0.0
	       elseif NY+@h>=Height then
		  y<-Height-@h
	       else
		  y<-NY
		  orgy<-Y
	       end
	       {self drawcsel}
	       {self setentry(E3 @x)}
	       {self setentry(E4 @y)}
	    [] 2 then % resize
	       DX=(X-@orgx)/Scale
	       DY=(Y-@orgy)/Scale
	       NW=@w+DX
	       NH=@h+DY
	    in
	       if NW<10.0 then
		  w<-10.0
		  orgx<-(@x+7.0)*Scale
	       elseif NW+@x>Width then
		  w<-Width-@x
		  orgx<-(@x+@w-1.0)*Scale
	       else
		  w<-NW
		  orgx<-X
	       end
	       if NH<10.0 then
		  h<-10.0
		  orgy<-(@y+7.0)*Scale
	       elseif NH+@y>Height then
		  h<-Height-@y
		  orgy<-(@y+@h-1.0)*Scale
	       else
		  h<-NH
		  orgy<-Y
	       end
	       {self drawcsel}
	       {self setentry(E1 @w)}
	       {self setentry(E2 @h)}
	    else skip end
	 end

	 meth crelease(X Y)
	    {self cmove(X Y)} % pour prendre en compte cet endroit-ci
	    {self updpsel}
	    state<-0 % retour a l'etat normal
	 end

	 meth cselall
	    x<-0.0
	    y<-0.0
	    w<-Width
	    h<-Height
	    {self drawcsel}
	    {self updpsel}
	    {self updcanvasentry}
	 end
	 
	 meth updc(Which)
	    fun {GetVal E}
	       {E tkReturn(get $)}
	    end
	    Val
	 in
	    case Which
	    of w then
	       if {self stringToFloat({GetVal E1} Val $)} then
		  if @x+Val>=Width then
		     w<-Width-@x
		  elseif Val<10.0 then
		     w<-10.0
		  else
		     w<-Val
		  end
	       else
		  {Tk.send bell}
	       end
	    [] h then
	       if {self stringToFloat({GetVal E2} Val $)} then
		  if @y+Val>=Height then
		     h<-Height-@y
		  elseif Val<10.0 then
		     h<-10.0
		  else
		     h<-Val
		  end
	       else
		  {Tk.send bell}
	       end	       
	    [] x then
	       if {self stringToFloat({GetVal E3} Val $)} then
		  if Val>Width-10.0 then
		     x<-Width-10.0
		  else
		     x<-Val
		  end
		  if @x+@w>=Width then
		     w<-Width-@x
		  else skip end
	       else
		  {Tk.send bell}
	       end	       
	    [] y then
	       if {self stringToFloat({GetVal E4} Val $)} then
		  if Val>Height-10.0 then
		     y<-Height-10.0
		  else
		     y<-Val
		  end
		  if @y+@h>=Height then
		     h<-Height-@y
		  else skip end
	       else
		  {Tk.send bell}
	       end	       
	    end
	    {self drawcsel}
	    {self updpsel}
	    {self updcanvasentry}
	 end   
	 
	 meth setcursor(C T)
	    {C tk(configure cursor:T)}
	 end
	 
	 meth updpageentry
	    {self setentry(E6 @pagew)}
	    {self setentry(E7 @pageh)}
	    {self setentry(E8 @pagex)}
	    {self setentry(E9 @pagey)}
	 end

	 meth updcanvasentry
	    {self setentry(E1 @w)}
	    {self setentry(E2 @h)}
	    {self setentry(E3 @x)}
	    {self setentry(E4 @y)}
	 end

	 meth drawsel(X1 Y1 X2 Y2 C Tag1 Tag2 Tag3)
	    {C tk(crea poly
		  X1 Y1     X1+5.0 Y1
		  X1+5.0 Y2 X1     Y2
		  width:1
		  stipple:gray75
		  fill:black
		  tags:Tag2)}
	    {C tk(crea poly
		  X2 Y1     X2-5.0 Y1
		  X2-5.0 Y2 X2     Y2
		  width:1
		  stipple:gray75
		  fill:black
		  tags:Tag2)}
	    {C tk(crea poly
		  X1 Y1     X1 Y1+5.0
		  X2 Y1+5.0 X2 Y1
		  width:1
		  stipple:gray75
		  fill:black
		  tags:Tag2)}
	    {C tk(crea poly
		  X1 Y2     X1 Y2-5.0
		  X2 Y2-5.0 X2 Y2
		  width:1
		  stipple:gray75
		  fill:black
		  tags:Tag2)}
	    {C tk(crea rect
		  X2-5.0 Y2-5.0 X2 Y2
		  width:1
		  outline:black
		  fill:black
		  tags:Tag3)}
	    {C tk(addtag Tag1 withtag Tag2)}
	    {C tk(addtag Tag1 withtag Tag3)}
	 end

	 meth drawcsel
	    X1 Y1 X2 Y2
	    Scale={Access Zoom}
	 in
	    {self.ctag tk(delete)}
	    X1=@x*Scale Y1=@y*Scale X2=(@x+@w-1.0)*Scale Y2=(@y+@h-1.0)*Scale
	    {self drawsel(X1 Y1 X2 Y2 C self.ctag self.cbtag self.cctag)}
	 end
	 
	 meth selectps(L W H)
	    fit<-false
	    {M1 tk(configure text:L)}
	    if W=="" then
	       if H=="Fit" then
		  fit<-true
		  {self calcsize}
		  {self drawpage}
		  {self drawpsel}		  
	       else skip end
	    else
	       pagew<-W
	       pageh<-H
	       {self updpageentry}
	       {self calcsize}
	       {self drawpage}
	       {self drawpsel}
	    end
	 end

	 meth selectcustom
	    case {Nth self.ps 1} of L#_#_ then
	       {M1 tk(configure menu:Menu text:L)}
	    end
	    fit<-false
	 end
	 
	 meth rotate(B)
	    rotate<-B
	    {self calcsize}
	    {self drawpage}
	    {self drawpsel}
	 end

	 meth drawpage
	    Prop
	    H W
	 in
	    if @rotate then
	       Prop=@pph/@ppw
	    else
	       Prop=@ppw/@pph
	    end
	    {self.pagetag tk(delete)}
	    if Prop<1.0 then
	       H=65.0
	       W=65.0*Prop
	       if @rotate then
		  scale<-130.0/@ppw
	       else
		  scale<-130.0/@pph
	       end
	       dx<-75.0-W
	       dy<-10.0
	    else
	       W=65.0
	       H=65.0/Prop
	       if @rotate then
		  scale<-130.0/@pph
	       else
		  scale<-130.0/@ppw
	       end
	       dx<-10.0
	       dy<-75.0-H
	    end
	    {C2 tk(crea rect
		   80.0-W 80.0-H
		   80.0+W 80.0+H
		   fill:black
		   outline:black
		   tags:self.pagetag)}
	    {C2 tk(crea rect
		   75.0-W 75.0-H
		   75.0+W 75.0+H
		   fill:white
		   outline:black
		   tags:self.pagetag)}
	 end

	 meth pointToUnit(Val Str Return)
	    case {StringToAtom [{List.last Str}]}
	    of m then Return={VirtualString.toString Val/72.0*25.4#"m"}
	    [] c then Return={VirtualString.toString Val/72.0*2.54#"c"}
	    [] i then Return={VirtualString.toString Val/72.0#"i"}
	    else Return={VirtualString.toString Val#"p"}
	    end
	 end 
	 
	 meth calcsize
	    if @fit then
	       %
	       % la taille depend de ce qui a ete selectionne dans le canvas
	       %
	       if @rotate then
		  ppw<-(@h/PageZoom)*@zoom
		  pph<-(@w/PageZoom)*@zoom
	       else
		  ppw<-(@w/PageZoom)*@zoom
		  pph<-(@h/PageZoom)*@zoom
	       end
	       ppx<-0.0
	       ppy<-0.0
	       pagew<-{self pointToUnit(@ppw @pagew $)}
	       pageh<-{self pointToUnit(@pph @pageh $)}
	       pagex<-{self pointToUnit(@ppx @pagex $)}
	       pagey<-{self pointToUnit(@ppy @pagey $)}
	       {self updpageentry}
	    else
	       fun{T E Old}
		  Val Unit CV CU in
		  if {self extract({E tkReturn(get $)} Val Unit $)} then
		     CV=Val CU=Unit
		  else
		     {self setentry(E Old)} % revient a l'ancienne valeur
		     {Tk.send bell}
		     if {self extract(Old CV CU $)} then
			skip
		     else
			raise badInitialValue end
		     end
		  end
		  case CU
		  of m then % conversion millimetres a points
		     (CV/25.4)*72.0
		  [] c then
		     (CV/2.54)*72.0
		  [] i then
		     CV*72.0
		  [] p then
		     CV
		  end
	       end
	    in
	       ppw<-{T E6 @pagew}
	       if @ppw<10.0 then
		  ppw<-10.0
		  pagew<-{self pointToUnit(@ppw {E6 tkReturn(get $)} $)}
		  {self setentry(E6 @pagew)}
	       else
		  pagew<-{E6 tkReturn(get $)}
	       end
	       pph<-{T E7 @pageh}
	       if @pph<10.0 then
		  pph<-10.0
		  pageh<-{self pointToUnit(@pph {E7 tkReturn(get $)} $)}
		  {self setentry(E7 @pageh)}
	       else
		  pagew<-{E6 tkReturn(get $)}
	       end
	       pageh<-{E7 tkReturn(get $)}
	       ppx<-{T E8 @pagex} pagex<-{E8 tkReturn(get $)}
	       ppy<-{T E9 @pagey} pagey<-{E9 tkReturn(get $)}
	       {self updpageentry}
	    end
	 end

	 meth drawpsel
	    X1 Y1 X2 Y2
	    W=(@w/PageZoom)*@zoom
	    H=(@h/PageZoom)*@zoom
	    IW=W/72.0
	    IH=H/72.0
	    fun{R V}
	       {Float.round V*10.0}/10.0
	    end
	 in
	    {self.ptag tk(delete)}
	    X1=@dx+@scale*@ppx
	    Y1=@dy+@scale*@ppy
	    X2=X1+@scale*W
	    Y2=Y1+@scale*H
	    {C2 tk(crea poly
		   X1 Y1 X1 Y2
		   X2 Y2 X2 Y1
		   fill:black
		   stipple:gray75
		   tags:self.pbtag)}
	    {C2 tk(crea rect
		   X2-3.0 Y2-3.0
		   X2 Y2
		   fill:black
		   tags:self.pctag)}
	    {C2 tk(addtag self.ptag withtag self.pbtag)}
	    {C2 tk(addtag self.ptag withtag self.pctag)}
	    {L19 tk(configure
		    text:{R IW*2.54}#"c x "#{R IH*2.54}#"c\n"#
		    {FloatToInt IW*25.4}#"m x "#{FloatToInt IH*25.4}#"m\n"#
		    {R IW}#"i x "#{R IH}#"i\n"#
		    {FloatToInt W}#"p x "#{FloatToInt W}#"p")}
	 end
	 
	 meth updpsel
	    %
	    % La taille de la zone selectionnee du canvas a change !
	    % On doit le prendre en compte ici
	    %
	    if @fit then
	       {self calcsize}
	       {self drawpage}
	    else skip end
	    {self drawpsel}
	 end
	 
	 meth startpmove(X Y)
	    if @fit then
	       {self selectcustom}
	    else skip end
	    orgx<-X
	    orgy<-Y
	    state<-3
	 end

	 meth startpsize(X Y)
	    {self selectcustom}
	    orgx<-X
	    orgy<-Y
	    state<-4
	 end

	 meth pmove(X Y)
	    case @state
	    of 3 then
	       DX DY NX NY
	    in
	       DX=X-@orgx
	       DY=Y-@orgy
	       NX=@ppx+DX/@scale
	       NY=@ppy+DY/@scale
	       if NX<0.0 then
		  ppx<-0.0
	       else
		  ppx<-NX
		  orgx<-X
	       end
	       if NY<0.0 then
		  ppy<-0.0
	       else
		  ppy<-NY
		  orgy<-Y
	       end
	       {self drawpsel}
	    [] 4 then
	       X1=@dx+@scale*@ppx
	       Y1=@dy+@scale*@ppy
	       OldH=@scale*(@w/PageZoom)*@zoom
	       OldW=@scale*(@h/PageZoom)*@zoom
	       NewW=X-X1
	       NewH=Y-Y1
	       Zoom1=@zoom*NewW/OldW
	       Zoom2=@zoom*NewH/OldH
	       Zoom
	       if Zoom1<Zoom2 then Zoom=Zoom1 else Zoom=Zoom2 end
	    in
	       if Zoom<0.1 then
		  zoom<-0.1
	       else
		  zoom<-Zoom
	       end
	       {self drawpsel}
	    else skip end
	 end

	 meth prelease(X Y)
	    case @state
	    of 3 then
	       {self pmove(X Y)}
	       pagex<-{self pointToUnit(@ppx @pagex $)}
	       pagey<-{self pointToUnit(@ppy @pagey $)}
	       {self setentry(E8 @pagex)}
	       {self setentry(E9 @pagey)}
	    [] 4 then skip
	       {self setentry(E5 @zoom*100.0)}
	    else skip end
	    state<-0
	 end
	 
	 meth updp(Which)
	    if @fit then
	       {self selectcustom}
	    elseif (Which==w orelse Which==h) andthen (@pagew\={E6 tkReturn(get $)} orelse @pageh\={E7 tkReturn(get $)}) then
	       {self selectcustom}
	    else skip end
	    {self calcsize}
	    {self drawpage}
	    {self drawpsel}
	 end   

	 meth chgzoom
	    Val in
	    if {self stringToFloat({E5 tkReturn(get $)} Val $)} then
	       if Val<10.0 then
		  zoom<-0.1
	       else
		  zoom<-Val/100.0
	       end
	       if @fit then
		  {self calcsize}
		  {self drawpage}
	       else skip end
	       {self drawpsel}
	    else
	       {Tk.send bell}
	    end
	    {self setentry(E5 @zoom*100.0)}
	 end
	 
	 meth hcenter
	    if @fit then
	       skip
	    else
	       if @rotate then
		  ppx<-(@pph-(@w/PageZoom)*@zoom)/2.0
	       else 
		  ppx<-(@ppw-(@w/PageZoom)*@zoom)/2.0
	       end
	       if @ppx<0.0 then ppx<-0.0 else skip end
	       pagex<-{self pointToUnit(@ppx @pagex $)}
	       {self setentry(E8 @pagex)}
	       {self drawpsel}
	    end
	 end

	 meth vcenter
	    if @fit then
	       skip
	    else
	       if @rotate then
		  ppy<-(@ppw-(@h/PageZoom)*@zoom)/2.0
	       else
		  ppy<-(@pph-(@h/PageZoom)*@zoom)/2.0
	       end
	       if @ppy<0.0 then ppy<-0.0 else skip end
	       pagey<-{self pointToUnit(@ppy @pagey $)}
	       {self setentry(E9 @pagey)}
	       {self drawpsel}
	    end
	 end

	 meth zoom100
	    zoom<-1.0
	    {self setentry(E5 100)}
	    if @fit then
	       {self calcsize}
	       {self drawpage}
	    else skip end
	    {self drawpsel}
	 end

	 meth zoomfit
	    if @fit then
	       skip
	    else
	       CW=@w/PageZoom
	       CH=@h/PageZoom
	       H W
	       if @rotate then
		  H=@ppw-2.0*@ppy
		  W=@pph-2.0*@ppx
	       else
		  H=@pph-2.0*@ppy
		  W=@ppw-2.0*@ppx
	       end
	       Zoom1=W/CW
	       Zoom2=H/CH
	       Zoom
	       if Zoom1<Zoom2 then Zoom=Zoom1 else Zoom=Zoom2 end
	    in
	       if Zoom<0.1 then
		  zoom<-0.1
	       else
		  zoom<-Zoom
	       end
	       {self drawpsel}
	       {self setentry(E5 @zoom*100.0)}
	    end
	 end

	 meth chgcolor(Val)
	    color<-Val
	 end
	 
	 meth setentry(E Val)
	    {E tk(delete 0 {E tkReturnInt(index('end') $)}+1)}
	    {E tk(insert 0 Val)}
	 end

	 meth stringToFloat(Str ?Val ?Ok)
	    fun{LoopRight Xs Fact Val}
	       case Xs of X|Xr then
		  if X<48 orelse X>57 then
		     false
		  else
		     {LoopRight Xr Fact*10.0 Val+{IntToFloat (X-48)}/Fact}
		  end
	       else
		  Val
	       end
	    end
	    fun{LoopLeft Xs Val}
	       case Xs of X|Xr then
		  if X==46 then % le point
		     {LoopRight Xr 10.0 Val}
		  elseif X<48 orelse X>57 then
		     false
		  else
		     {LoopLeft Xr Val*10.0+{IntToFloat X-48}}
		  end
	       else
		  Val
	       end
	    end
	 in
	    Val={LoopLeft Str 0.0}
	    Ok=(Val\=false)
	 end

	 meth extract(Str ?Val ?Unit ?Ok)
	    if {Length Str}<2 then
	       Ok=false
	    else
	       Unit={StringToAtom [{List.last Str}]}
	       if Unit\=m andthen Unit\=c andthen Unit\=p andthen Unit\=i then
		  Ok=false
	       else
		  {self stringToFloat({List.take Str {Length Str}-1} Val Ok)}
	       end   
	    end
	 end
	 
	 meth resize(H W)
	    Fact
	    Size S1 Scale
	 in
	    if H<W then S1=H-10.0 else S1=W-10.0 end
	    if S1<50.0 then Size=50.0 else Size=S1 end
	    if Width>Height then
	       Scale=Size/Width
	    else
	       Scale=Size/Height
	    end
	    Fact=Scale/{Access Zoom}
	    {C tk(scale all 0 0 Fact Fact)}
	    {Assign Zoom Scale}
	    {C tk(configure width:Width*Scale-6.0 height:Height*Scale-6.0)}
	    {self drawcsel}
	 end

	 meth ok
	    Return=postscript(
		      height:@h
		      width:@w
		      x:@x
		      y:@y
		      pagewidth:@pagew
		      pageheight:@pageh
		      fit:@fit
		      pagex:@pagex
		      pagey:@pagey
		      rotate:@rotate
		      colormode:@color
		      zoom:@zoom
		      tkx:(~1.0*@ppx*PageZoom+@x*@zoom)
		      tky:(~1.0*@ppy*PageZoom+@y*@zoom)
		      tkwidth:@ppw*PageZoom
		      tkheight:@pph*PageZoom
		      )
	 end

	 meth cancel
	    Return=''
	 end
      end
      proc{Bind E Event}
	 {E tkBind(event:"<FocusOut>"
		   action:Z#Event)}
	 {E tkBind(event:"<Key-Return>"
		   action:Z#Event)}
      end
   in
      [X1 Y1 X2 Y2]={Map SelReg fun{$ V}
				   if {IsInt V} then
				      {IntToFloat V}
				   else
				      V
				   end
				end}
      Width=X2-X1+1.0
      Height=Y2-Y1+1.0
      T={New Tk.toplevel tkInit(title:Title
				delete:Z#cancel
				withdraw:true)}
      F1={New Tk.frame tkInit(parent:T
			      relief:ridge
			      borderwidth:2)}
      F2={New Tk.frame tkInit(parent:F1 width:10 height:10)}
      {F2 tkBind(event:"<Configure>"
		 args:[float(h) float(w)]
		 append:true
		 action:Z#resize)}
      L1={New Tk.label tkInit(parent:F1
			      text:"Select the area to print below"
			      anchor:w)}
      C={New Tk.canvas tkInit(parent:F2
			      bg:{Canvas tkReturn(cget("-background") $)}
			      relief:ridge
			      width:100
			      height:100
			      borderwidth:2
			      highlightthickness:0)}
      F3={New Tk.frame tkInit(parent:F1)}
      L2={New Tk.label tkInit(parent:F3
			      text:"Size :")}
      E1={New Tk.entry tkInit(parent:F3
			      width:6
			      background:white)}
      {Bind E1 updc(w)}
      L3={New Tk.label tkInit(parent:F3
			      text:"X")}
      E2={New Tk.entry tkInit(parent:F3
			      width:6
			      background:white)}
      {Bind E2 updc(h)}
      L4={New Tk.label tkInit(parent:F3
			      anchor:w
			      text:" screen units")}
      F4={New Tk.frame tkInit(parent:F1)}
      L5={New Tk.label tkInit(parent:F4
			      text:"Origin :")}
      L51={New Tk.label tkInit(parent:F4
			       text:"(")}
      E3={New Tk.entry tkInit(parent:F4
			      width:6
			      background:white)}
      {Bind E3 updc(x)}
      L6={New Tk.label tkInit(parent:F4
			      text:",")}
      E4={New Tk.entry tkInit(parent:F4
			      width:6
			      background:white)}
      {Bind E4 updc(y)}
      L7={New Tk.label tkInit(parent:F4
			      anchor:w
			      text:")")}
      B1={New Tk.button tkInit(parent:F1
			       text:"Select whole canvas"
			       action:Z#cselall)}
      F5={New Tk.frame tkInit(parent:T
			      relief:ridge
			      borderwidth:2)}
      F51={New Tk.frame tkInit(parent:F5)}
      L8={New Tk.label tkInit(parent:F51
			      anchor:w
			      text:"Place the area to print on the page below")}
      C2={New Tk.canvas tkInit(parent:F51
			       background:white
			       width:150
			       height:150
			       relief:sunken
			       borderwidth:2)}
%      L10={New Tk.label tkInit(parent:F51
%			       text:"Orientation :")}
      V1={New Tk.variable tkInit(InitRotate)}
      R1={New Tk.radiobutton tkInit(parent:F51
				    text:"Portait"
				    value:false
				    variable:V1
				    action:Z#rotate(false)
				    anchor:w)}
      R2={New Tk.radiobutton tkInit(parent:F51
				    text:"Landscape"
				    value:true
				    variable:V1
				    action:Z#rotate(true)
				    anchor:w)}
      L18={New Tk.label tkInit(parent:F51
			       anchor:w
			       justify:left
			       text:"Size :")}
      L19={New Tk.label tkInit(parent:F51
			       anchor:n
			       justify:center
			       text:"1c x 1c")}
      F6={New Tk.frame tkInit(parent:F5)}
      L9={New Tk.label tkInit(parent:F6
			      text:"Page size :")}
      M1={New Tk.menubutton tkInit(parent:F6
				   text:""
				   width:25
				   relief:raised)}
      Menu={New Tk.menu tkInit(parent:M1
			       tearoff:false
			       type:normal)}
      F7={New Tk.frame tkInit(parent:F5)}
      V2={New Tk.variable tkInit(InitColor)}
      R3={New Tk.radiobutton tkInit(parent:F7
				    text:"Color"
				    value:color
				    variable:V2
				    action:Z#chgcolor(color)
				    anchor:w)}
      R4={New Tk.radiobutton tkInit(parent:F7
				    text:"Greyscale"
				    value:gray
				    variable:V2
				    action:Z#chgcolor(gray)
				    anchor:w)}
      R5={New Tk.radiobutton tkInit(parent:F7
				    text:"B&W"
				    value:mono
				    variable:V2
				    action:Z#chgcolor(mono)
				    anchor:w)}
      F8={New Tk.frame tkInit(parent:F5)}
      L11={New Tk.label tkInit(parent:F8
			       text:"Zoom :")}
      E5={New Tk.entry tkInit(parent:F8
			      background:white
			      width:6)}
      {Bind E5 chgzoom}
      L12={New Tk.label tkInit(parent:F8
			       text:"%")}
      B2={New Tk.button tkInit(parent:F8
			       pady:0
			       text:"Best Fit"
			       action:Z#zoomfit)}
      B3={New Tk.button tkInit(parent:F8
			       pady:0
			       text:"100%"
			       action:Z#zoom100)}
      F9={New Tk.frame tkInit(parent:F5
			      relief:sunken
			      borderwidth:2)}
      L13={New Tk.label tkInit(parent:F9
			       text:"Page width :"
			       anchor:e)}
      L14={New Tk.label tkInit(parent:F9
			       text:"height :"
			       anchor:e)}
      L15={New Tk.label tkInit(parent:F9
			       text:"Left margin :"
			       anchor:w)}
      L16={New Tk.label tkInit(parent:F9
			       text:"Top margin :"
			       anchor:w)}
      E6={New Tk.entry tkInit(parent:F9
			      background:white
			      width:10)}
      {Bind E6 updp(w)}
      E7={New Tk.entry tkInit(parent:F9
			      background:white
			      width:10)}
      {Bind E7 updp(h)}
      E8={New Tk.entry tkInit(parent:F9
			      background:white
			      width:10)}
      {Bind E8 updp(x)}
      E9={New Tk.entry tkInit(parent:F9
			      background:white
			      width:10)}
      {Bind E9 updp(y)}
      L17={New Tk.label tkInit(parent:F9
			       justify:left
			       anchor:nw
			       text:"m=millimeters,\nc=centimeters,\np=printer points,\ni=inches.")}
      B4={New Tk.button tkInit(parent:F9
			       text:"Center"
			       pady:0
			       action:Z#hcenter)}
      B41={New Tk.button tkInit(parent:F9
				text:"Center"
				pady:0
				action:Z#vcenter)}
      F10={New Tk.frame tkInit(parent:T)}
      B5={New Tk.button tkInit(parent:F10
			       text:"Ok"
			       action:Z#ok)}
      B6={New Tk.button tkInit(parent:F10
			       text:"Cancel"
			       action:Z#cancel)}
      {T tkBind(event:"<Escape>"
		action:Z#cancel)}
      {Tk.batch [% main window
		 grid(F1  row:0 column:0 sticky:nswe padx:5 pady:5)
		 grid(F5  row:0 column:1 sticky:nswe padx:5 pady:5)
		 grid(F10 row:1 column:0 columnspan:2 sticky:nswe)
		 grid(columnconfigure T 0 weight:1)
		 grid(rowconfigure    T 0 weight:1)
		 % Source size selection frame
		 grid(L1 row:0 column:0 sticky:we   padx:5 pady:5)
		 grid(F2 row:1 column:0 sticky:nswe padx:5)
		 grid(F3 row:2 column:0 sticky:we   padx:5 pady:2)
		 grid(F4 row:3 column:0 sticky:we   padx:5 pady:2)
		 grid(B1 row:4 column:0 sticky:we   padx:5 pady:5)
		 grid(columnconfigure F1 0 weight:1)
		 grid(rowconfigure    F1 1 weight:1)
		 % Source canvas selection frame
		 pack(C fill:none expand:true)
		 % Source text selection
		 pack(L2 side:left fill:x padx:5)
		 pack(E1 L3 E2 side:left fill:x)
		 pack(L4 side:left fill:x padx:5)
		 pack(L5 side:left fill:y padx:5)
		 pack(L51 E3 L6 E4 L7 side:left fill:y)
		 % Output size selection frame
		 pack(F51 F6 F9 side:top expand:false fill:x padx:5 pady:5)
		 pack(F7 F8 side:top expand:false fill:x pady:2)
		 % Output text selection
		 grid(L8 row:0 column:0 columnspan:2 sticky:nswe pady:5)
		 grid(C2 row:1 column:0 rowspan:5 padx:5)
		 grid(R1 row:1 column:1 sticky:nswe padx:5 pady:5)
		 grid(R2 row:2 column:1 sticky:nswe padx:5 pady:5)
		 grid(L18 row:3 column:1 sticky:nswe padx:5)
		 grid(L19 row:4 column:1 sticky:nswe padx:5)
		 grid(columnconfigure F51 0 weight:1)
		 grid(rowconfigure    F51 5 weight:3)
		 pack(L9 M1 side:left fill:x padx:5)
		 pack(R3 R4 R5 side:left padx:5)
		 grid(columnconfigure F7 1 weight:1)
		 pack(L11 side:left padx:5)
		 pack(E5 L12 side:left)
		 pack(B2 B3 side:left padx:5)
		 grid(L13 row:0 column:0 sticky:we padx:2 pady:2)
		 grid(L14 row:1 column:0 sticky:we padx:2 pady:2)
		 grid(L15 row:2 column:0 sticky:we padx:2 pady:2)
		 grid(L16 row:3 column:0 sticky:we padx:2 pady:2)
		 grid(E6  row:0 column:1 sticky:we padx:2 pady:2)
		 grid(E7  row:1 column:1 sticky:we padx:2 pady:2)
		 grid(E8  row:2 column:1 sticky:we padx:2 pady:2)
		 grid(E9  row:3 column:1 sticky:we padx:2 pady:2)
		 grid(L17 row:0 column:2 rowspan:2 sticky:we padx:2 pady:2)
		 grid(B4  row:2 column:2 sticky:we padx:2 pady:2)
		 grid(B41 row:3 column:2 sticky:we padx:2 pady:2)
		 grid(columnconfigure F9 2 weight:1)
		 % Ok/Cancel buttons
		 pack(B5 B6 side:left padx:5 pady:5)
		 wm(deiconify T)
		 grab(T)]}
      local ThId in
	 thread
	    Obj
	    proc{Loop Xs}
	       Skip in
	       case Xs of X|Xr then
		  if {Label X}==cmove andthen {IsDet Xr} then
		     case Xr of Y|_ then
			if {Label Y}==cmove then
			   Skip=unit
			else skip end
		     else skip end
		  elseif {Label X}==pmove andthen {IsDet Xr} then
		     case Xr of Y|_ then
			if {Label Y}==pmove then
			   Skip=unit
			else skip end
		     else skip end
		  else skip end
		  if {IsFree Skip} then {Obj X} else skip end
		  {Loop Xr}
	       else skip end
	    end
	 in
	    ThId={Thread.this}
	    Obj={New MyClass init}
	    {ForAll Read
	     proc{$ Msg}
		{Obj Msg}
	     end}
	 end
	 {Wait Return}
	 {Thread.terminate ThId}
	 {Tk.send grab(release T)}
	 {T tkClose}
      end
   end

end
%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   class Toplevel
      from Tk.toplevel

      feat
	 manager
	 scale
      
      meth init(Manager Menu Canvas Status ?PackMe)
	 <<Tk.toplevel tkInit(title:              TitleName
			      relief:             sunken
			      withdraw:           True
			      highlightthickness: 0)>>
	 {Tk.batch [focus(self)
		    wm(iconname   self TitleName)
		    wm(iconbitmap self BitMap)
		    wm(minsize self MinSizeX MinSizeY)]}
	 
	 Frame = {New Tk.frame tkInit(parent:self highlightthickness:0)}
	 
	 CanFrame = {New Tk.frame tkInit(parent:Frame highlightthickness:0)}
	 
	 ScaleFrame = {New Tk.frame tkInit(parent:Frame highlightthickness:0)}
	 
	 Scale = {New Tk.scale [tkInit(parent:    ScaleFrame
				       'from':    MinScalePercent
				       to:        MaxScalePercent
				       showvalue: 0
				       width:     ScrollerWidth
				       action: proc {$ S}
						  {Manager
						   scale({Tk.string.toFloat S}
							 / 100.0)}
					       end)
				tk(set DefScalePercent)
				tkBind(event:  '<2>'
				       action: proc {$}
						  {Manager scaleToFit}
					       end)]}

      in
	 Canvas = {New ScrollCanvas init(self CanFrame Manager)}
	 self.manager = Manager
	 self.scale   = Scale
	 proc {PackMe}
	    {Tk.batch [pack(Scale       o(expand:yes fill:y))
		       pack(CanFrame    o(side:left fill:both expand:yes))
		       pack(ScaleFrame  o(side:left fill:y))
		       pack(Menu.'self' o(side:top fill:x))
		       pack(Status      o(side:bottom anchor:w fill:x))
		       pack(Frame       o(side:bottom fill:both expand:yes))
		       wm(deiconify self)]}
	 end
      end


      meth close
	 job
	    {self.manager close}
	 end
	 <<Tk.toplevel close>>
      end

   end


   class ScrollCanvas
      from Tk.canvas

      attr
	 scale:  1.0
         %% These values are scaled
	 left:   0.0
	 right:  0.0
	 bottom: 0.0
         %% These values are unscaled
	 width:  CanvasWidth
	 height: CanvasHeight
   
      feat
	 manager
	 genTreeId
	 genNodeId
	 genLinkId
	 tagsVar
	 initTags
	 addTag
	 skipTag
	 actionTag

      meth init(Toplevel Parent Manager)
	 CanScrY = {New Tk.frame tkInit(parent:Parent bd:0
					highlightthickness:0)}
	 ScrXBox = {New Tk.frame tkInit(parent:Parent bd:0
					highlightthickness:0)}
	 <<Tk.canvas tkInit(parent:          Toplevel
			    width:           CanvasWidth
			    height:          CanvasHeight
			    relief:          sunken
			    background:      BackColor
			    bd:              1
			    scrollregion:    q(~CanvasWidth/2.0 0
					       CanvasWidth/2.0  CanvasHeight)
			    highlightthickness:0)>>
	 ScrX    = {New Tk.scrollbar tkInit(parent: ScrXBox
					    relief: sunken
					    bd:     Border
					    width:  ScrollerWidth
					    orient: horizontal)}
	 ScrY    = {New Tk.scrollbar tkInit(parent: Parent
					    relief: sunken
					    bd:     Border
					    width:  ScrollerWidth)}
	 Box     = {New Tk.frame tkInit(parent: ScrXBox
					width:  ScrollerWidth + 2*Border
					height: ScrollerWidth + 2*Border
					highlightthickness:0)}
	 TagsVar      = {Tk.server tkGet($)}
	 AddTagPrefix = v({String.toAtom
			   {VirtualString.toString
			    TagsVar#' [linsert $'#TagsVar#' 0'}})
	 AddTagSuffix = v(']')
	 SkipTag      = v({String.toAtom
			   {VirtualString.toString
			    'set '#TagsVar#' [lreplace $'#TagsVar#' 0 0]'}})
      in
	 {Tk.addYScrollbar self ScrY}
	 {Tk.addXScrollbar self ScrX}
	 {Tk.batch
	  [pack(ScrXBox o(side:bottom fill:x     padx:Pad pady:Pad))
	   pack(ScrY    o(side:right  fill:y     padx:Pad pady:Pad))
	   pack(CanScrY o(fill:both              padx:Pad pady:Pad expand:yes))
	   pack(ScrX    o(side:left   fill:x     expand:yes))
	   pack(Box     o(side:right  fill:none  padx:Pad))
	   pack(self    o('in':CanScrY
			    fill:both   expand:yes padx:BigPad pady:BigPad))]}
	 <<Tk.canvas tkBind(event: '<Configure>'
			    action: proc {$ H W}
				       {self  Resized({Tk.string.toFloat H}
						      {Tk.string.toFloat W})}
				    end
			    args:   [h w]
			    append: True)>>
	 self.manager   = Manager
	 self.genNodeId = {New Tk.server tkInit}
	 self.genTreeId = {New Tk.server tkInit}
	 self.genLinkId = {New Tk.server tkInit}
	 self.tagsVar   = v({String.toAtom
			     {VirtualString.toString
			      '[linsert $'#TagsVar#' 0'}})
	 self.initTags  = proc {$ Tags}
			     {Tk.send set(v(TagsVar) q(b(Tags)))}
			  end
	 self.addTag    = proc {$ Tag}
			     {Tk.send set(AddTagPrefix Tag AddTagSuffix)}
			  end
	 self.skipTag   = proc {$}
			     {Tk.send SkipTag}
			  end
	 self.actionTag = {New Tk.canvasTag
			   [tkInit(parent:self)
			    tkBind(event:  '<1>'
				   args:   [x y]
				   action: proc {$ X Y}
					      {Manager 
					       setByXY({Tk.string.toFloat X}
						       {Tk.string.toFloat Y})}
					   end)
			    tkBind(event:  '<Double-1>'
				   args:   [x y]
				   action: proc {$ X Y}
					      {Manager
					       selInfo({Tk.string.toFloat X}
						       {Tk.string.toFloat Y})}
					   end)
			    tkBind(event: '<2>'
				   args:  [x y]
				   action: proc {$ X Y}
					      {Manager
					       nodesByXY(hide
							 {Tk.string.toFloat X}
							 {Tk.string.toFloat Y})}
					   end)
			    tkBind(event:  '<3>'
				   args:   [x y]
				   action: proc {$ X Y}
					      {Manager
					       nodesByXY(hideFailed
							 {Tk.string.toFloat X}
							 {Tk.string.toFloat Y})}
					   end)]}
      end

      meth clear
	 left   <- 0.0
	 right  <- 0.0
	 bottom <- 0.0
	 <<Tk.canvas tk(delete all)>>
	 <<ScrollCanvas AdjustRegion>>
	 {self.genNodeId tkReset}
	 {self.genTreeId tkReset}
	 {self.genLinkId tkReset}
      end

      meth scrollTo(X Y)
	 Scale  = @scale
	 Left   = Scale * @left
	 Right  = Scale * @right
	 Bottom = Scale * @bottom
      in 
	 <<Tk.canvas tk(xview moveto
			(X*Scale - Left - @width/2.0)/(Right-Left))>>
	 <<Tk.canvas tk(yview moveto
			(Y*Scale - @height/2.0)/Bottom)>>
      end

      meth bounding(NewLeft NewRight NewBottom)
	 left   <- NewLeft
	 right  <- NewRight
	 bottom <- NewBottom
	 <<ScrollCanvas AdjustRegion>>
      end
   
      meth AdjustRegion
	 Scale  = @scale
	 Left   = Scale * @left
	 Right  = Scale * @right
	 Bottom = Scale * @bottom
	 Width  = Right - Left
	 Delta  = (Width - @width) / 2.0
	 ReqLeft # ReqRight = case Delta<0.0 then
				 (Left+Delta) # (Right-Delta)
			      else
				 Left#Right
			      end
	 ReqBottom = case Bottom>@height then Bottom
		     else @height
		     end
      in
	 <<Tk.canvas tk(configure(scrollregion:
				     q(ReqLeft 0 ReqRight ReqBottom)))>>
      end
      
      meth scale(Scale)
	 ScaleBy = Scale / @scale
      in
	 scale <- Scale
	 <<ScrollCanvas AdjustRegion>>
	 <<Tk.canvas tk(scale all 0 0 ScaleBy ScaleBy)>>
      end

      meth scaleToFit(?NewScale)
	 Left   = @left
	 Right  = @right
	 Bottom = @bottom
      in
	 case Bottom==0.0 orelse Left==0.0 orelse Right==0.0 then true else
	    Factor = {FloatToInt 100.0 * {Min
					  @width / (Right - Left)
					  @height / Bottom}}
	    ScaleFactor = case Factor<MinScalePercent then MinScalePercent
			  elsecase Factor>MaxScalePercent then MaxScalePercent
			  else Factor
			  end
	 in
	    NewScale = {IntToFloat ScaleFactor} / 100.0
	    {self.manager.toplevel.scale tk(set ScaleFactor)}
	 end
      end
      
      meth Resized(H W)
	 height <- H
	 width  <- W
	 <<ScrollCanvas AdjustRegion>>
      end

      meth postscript(colormode:C rotate:R file:F height:H width:W)
	 Scale  = @scale
	 Height = @bottom * Scale
	 Width  = (@right - @left) * Scale
      in
	 <<Tk.canvas tk(postscript(file:       F
				   colormode:  C
				   rotate:     R
				   height:     Height
				   width:      Width
				   x:          @left * Scale
				   y:          0)
			case H/Height > W/Width then
			   o(pagewidth: W#c)
			else
			   o(pageheight: H#c)
			end)>>
	 
      end
      
   end


   fun {PickFont Fs Scale}
      case Fs of nil then False
      [] F|Fr then
	 case F.scale=<Scale then F else {PickFont Fr Scale} end
      end
   end

   proc {MakeDirty Ns}
      case Ns of nil then true
      [] N|Nr then
	 case N.mom of !False then true elseof Mom then {Mom dirtyUp} end
	 {MakeDirty Nr}
      end
   end
      
in

   class ToplevelManager
      feat
	 toplevel
	 canvas
	 cursor
	 connection
	 numbers
      
      attr
	 CurNumber:     1
	 curFont:       {PickFont NumberFonts InitialScale}
	 NumberNodes:   nil
	 curNode:       False
	 cmpNode:       False
	 scale:         1.0

      meth init(?PackMe)
	 self.toplevel  = {New Toplevel init(self
					     self.menu
					     self.canvas
					     self.status ?PackMe)}
	 self.cursor     = {Tk.server tkGet($)}
	 self.connection = {Tk.server tkGet($)}
	 self.numbers    = {Tk.server tkGet($)}
      end

      meth clear
	 {self.canvas tk(delete all)}
	 curNode       <- False
	 cmpNode       <- False
	 <<ToplevelManager clearNumbers>>
      end
      
      meth configurePointer(Status)
	 case {Det {self.canvas
		    tkReturn(conf(cursor:case Status
					 of drawing   then   pencil
					 [] searching then watch
					 [] idle      then top_left_arrow
					 end) $)}} then
	    scale <- @scale
	 end
      end

      meth scale(Scale)
	 Font     = {PickFont NumberFonts Scale}
	 FontName = case Font==False then False else Font.name end
	 Canvas   = self.canvas
      in
	 scale   <- Scale
	 {Canvas scale(Scale)}
	 case @curFont of !Font then true elseof CF then
	    case @NumberNodes==nil then true else
	       case Font==False then
		  {Canvas tk(delete self.numbers)}
	       else
		  case CF==False then
		     {ForAll @NumberNodes
		      proc {$ Node}
			 {Node redrawNumber(Scale FontName)}
		      end}
		  else
		    {Canvas tk(itemconfigure self.numbers o(font:FontName))}
		  end
	       end
	    end
	    curFont <- Font
	 end
      end

      meth scaleToFit
	 case {self.canvas scaleToFit($)} of !False then true
	 elseof NewScale then
	    <<ToplevelManager scale(NewScale)>>
	 end
      end

      meth findByXY(ScaledX ScaledY $)
	 CanvasX = job {self.canvas tkReturnFloat(canvasx(ScaledX) $)} end
	 CanvasY = job {self.canvas tkReturnFloat(canvasy(ScaledY) $)} end
	 Scale   = @scale
	 FindX   =  CanvasX / Scale - MaxExtent 
	 Depth   = {Float.toInt
		    {Float.round (CanvasY / Scale - HalfVerSpace) / VerSpace}}
      in
	 {@root findByX(Depth RootX FindX $)}
      end
      
      meth setCursor(CurNode IsVisible)
	 X Y
	 CursorX CursorY
	 Scale = @scale
	 Canvas = self.canvas
      in
	 {Canvas tk(delete self.cursor self.connection)}
	 {CurNode getCenter(?X ?Y)}
	 CursorX = (X + ShadeWidth) * Scale
	 CursorY = (Y + ShadeWidth) * Scale
	 {Canvas tk(crea {Shapes.(case
				     CurNode.kind==choice andthen
				     {CurNode isHidden($)} then hidden
				  else CurNode.kind end)
			  (X + ShadeWidth) * Scale
			  (Y + ShadeWidth) * Scale
			  Scale}
		    o(fill:CursorColor outline: '' tags:self.cursor))}
	 {Canvas tk(lower self.cursor CurNode.node)}
	 case CurNode==@curNode orelse IsVisible then true else
	    {Canvas scrollTo(X Y)}
	 end
	 curNode <- CurNode
	 case @cmpNode of !False then
	    {Canvas tk(delete self.connection)}
	 elseof CmpNode then
	    case CmpNode==CurNode then true else
	       CmpX CmpY
	    in
	       {CmpNode getCenter(?CmpX ?CmpY)}
	       {self.canvas [tk(delete self.connection)
			     tk(crea line
				Scale*X Scale*Y Scale*CmpX Scale*CmpY
				o(arrow: last
				  fill:  CursorColor
				  width: LinkWidth
				  tags:  self.connection))
			  tk(raise self.connection CmpNode.node)
			  tk(raise self.connection CurNode.node)]}
	    end
	 end
      end

      meth hideCursor
	 {self.canvas tk(delete self.cursor self.connection)}
      end

      meth clearNumbers
	 {self.canvas tk(delete self.numbers)}
	 {ForAll @NumberNodes proc {$ N} {N clearNumber} end}
	 CurNumber   <- 1
	 NumberNodes <- nil
      end
      
      meth getNumber(Node ?N)
	 NewNumber = @CurNumber
      in
	 {Node getNumber(@scale case @curFont of !False then False
				elseof CF then CF.name
				end
			 NewNumber ?N)}
	 case NewNumber==N then
	    CurNumber   <- NewNumber + 1
	    NumberNodes <- Node|@NumberNodes
	 else true
	 end
      end
      
      meth refreshNumbers
	 {self.canvas tk(itemconfigure self.numbers o(fill:LineColor))}
      end

      meth makeDirty(Ns)
	 {MakeDirty Ns}
      end
      
      meth close
	 {self.toplevel close}
      end
      
   end
   
end


